// The environment the extension's modules run in when they are not in a browser.
//
// Only the surfaces the code actually touches are provided, and none of the code under test is
// replaced: the modules imported after installEnv are the shipped ones.

/** A node carrying the parts of Element that the extension uses. */
export class FakeEl {
	/**
	 * @param tag - Tag name.
	 * @param sels - Selectors this node is to be found by, which stands in for CSS matching.
	 */
	constructor(tag = "div", sels = []) {
		this.tagName = String(tag).toUpperCase();
		this.sels = new Set(sels);
		this.children = [];
		this.parent = null;
		this.className = "";
		this.textContent = "";
		this.attrs = {};
		this.listeners = {};
		this.classList = {
			add: (c) => { if (!this.className.split(" ").includes(c)) this.className = (this.className + " " + c).trim(); },
			remove: (c) => { this.className = this.className.split(" ").filter(x => x && x !== c).join(" "); },
			contains: (c) => this.className.split(" ").includes(c)
		};
	}
	get isConnected() { return this.parent ? this.parent.isConnected : this._root === true; }
	setAttribute(k, v) { this.attrs[k] = v; }
	getAttribute(k) { return this.attrs[k]; }
	appendChild(child) { child.parent = this; this.children.push(child); return child; }
	remove() { if (this.parent) this.parent.children = this.parent.children.filter(c => c !== this); this.parent = null; }
	addEventListener(type, fn) { (this.listeners[type] ||= []).push(fn); }
	matches(sel) { return this.sels.has(sel); }
	/** Returns this node or its nearest ancestor registered under `sel`. */
	closest(sel) {
		for (let n = this; n; n = n.parent) if (n.matches(sel)) return n;
		return null;
	}
	/** Returns the text of this node and everything under it. */
	text() { return [this.textContent, ...this.children.map(c => c.text())].filter(Boolean).join(" "); }
}

/** A clock that only moves when a case moves it. */
export class Clock {
	constructor() { this.now = 1_700_000_000_000; this.timers = []; this.seq = 0; }
	setTimeout(fn, ms) { const id = ++this.seq; this.timers.push({ id, at: this.now + (ms || 0), fn, every: 0 }); return id; }
	setInterval(fn, ms) { const id = ++this.seq; this.timers.push({ id, at: this.now + ms, fn, every: ms }); return id; }
	clearTimeout(id) { this.timers = this.timers.filter(t => t.id !== id); }
	clearInterval(id) { this.clearTimeout(id); }
	/** Runs every timer falling due within `ms`, in due order, then leaves the clock at the end. */
	advance(ms) {
		const target = this.now + ms;
		for (let guard = 0; guard < 200000; guard++) {
			const due = this.timers.filter(t => t.at <= target).sort((a, b) => a.at - b.at)[0];
			if (!due) break;
			this.now = due.at;
			if (due.every) due.at = this.now + due.every;
			else this.timers = this.timers.filter(t => t !== due);
			due.fn();
		}
		this.now = target;
	}
	/** Lets pending microtasks settle without moving time. */
	async flush() { for (let i = 0; i < 16; i++) await Promise.resolve(); }

	/**
	 * Advances like `advance`, letting promises settle between one timer and the next.
	 *
	 * Code that hands a storage read to a callback from inside a `then` needs both to make
	 * progress, which the synchronous form cannot give it.
	 * @param ms - How far to move.
	 */
	async runFor(ms) {
		const target = this.now + ms;
		for (let guard = 0; guard < 200000; guard++) {
			await this.flush();
			const due = this.timers.filter(t => t.at <= target).sort((a, b) => a.at - b.at)[0];
			if (!due) break;
			this.now = due.at;
			if (due.every) due.at = this.now + due.every;
			else this.timers = this.timers.filter(t => t !== due);
			due.fn();
		}
		await this.flush();
		this.now = target;
	}
}

/**
 * Installs the globals the extension reads and returns the handles a case drives them with.
 * @param opts - `pathname`, `storage` (initial contents), `agent` (a /systeminfo payload or null
 * for a refusing agent), `nodes` (selector to node), `title`.
 * @returns The environment handles.
 */
export function installEnv(opts = {}) {
	const clock = new Clock();
	const nodes = new Map(Object.entries(opts.nodes || {}));
	const store = { ...(opts.storage || {}) };
	const storageListeners = [];
	const mutationCallbacks = new Set();
	const docListeners = {};
	const winListeners = {};
	const fetches = [];
	const messages = [];

	const docEl = new FakeEl("html");
	docEl._root = true;
	docEl.dataset = {};
	const body = new FakeEl("body");
	docEl.appendChild(body);

	const document = {
		documentElement: docEl,
		body,
		title: opts.title || "Untitled",
		getElementById: (id) => findById(docEl, id),
		querySelector: (sel) => nodes.get(sel) || null,
		createElement: (tag) => new FakeEl(tag),
		createElementNS: (_ns, tag) => new FakeEl(tag),
		addEventListener: (type, fn) => { (docListeners[type] ||= []).push(fn); }
	};

	/** Answers a storage read through a callback or a promise, whichever the caller asked for. */
	const storageGet = (keys, cb) => {
		const wanted = Array.isArray(keys) ? keys : [keys];
		const out = {};
		for (const k of wanted) if (k in store) out[k] = store[k];
		if (!cb) return Promise.resolve(out);
		clock.setTimeout(() => cb(out), 1);
		return undefined;
	};

	/** Writes keys and reports them to the onChanged listeners, as the real storage does. */
	const storageSet = (obj, cb) => {
		const changes = {};
		for (const [k, v] of Object.entries(obj)) {
			changes[k] = { oldValue: store[k], newValue: v };
			store[k] = v;
		}
		for (const fn of storageListeners) fn(changes, "local");
		if (cb) cb();
		return Promise.resolve();
	};

	const browser = {
		i18n: { getMessage: () => "" },   // falls through to the key, which assertions read
		runtime: {
			id: "sysinfo@company.local",
			lastError: null,
			getManifest: () => opts.manifest || { version: "2.1.0", host_permissions: [] },
			sendMessage: (msg, cb) => {
				messages.push(msg);
				if (!cb) return Promise.resolve({ ok: true });
				if (msg.action !== "getSystemInfo") { cb({ ok: true }); return; }
				// A responder may set browser.runtime.lastError before answering, which is how a
				// messaging failure reaches the caller.
				clock.setTimeout(() => {
					browser.runtime.lastError = null;
					const respond = opts.respond;
					cb(respond
						? respond(msg, browser)
						: (opts.agent ? { success: true, data: opts.agent } : { success: false, error: "refused" }));
				}, 5);
			},
			onMessage: { addListener: (fn) => { browser.runtime._onMessage = fn; } },
			onInstalled: { addListener: () => {} },
			onStartup: { addListener: () => {} }
		},
		permissions: { contains: async () => true },
		storage: {
			local: { get: storageGet, set: storageSet },
			onChanged: { addListener: (fn) => storageListeners.push(fn) }
		}
	};

	Object.assign(globalThis, {
		document,
		browser,
		Element: FakeEl,
		location: { pathname: opts.pathname || "/", href: "https://jira.company.local" + (opts.pathname || "/") },
		window: { addEventListener: (type, fn) => { (winListeners[type] ||= []).push(fn); } },
		MutationObserver: class {
			constructor(cb) { this.cb = cb; }
			observe() { mutationCallbacks.add(this.cb); }
			disconnect() { mutationCallbacks.delete(this.cb); }
		},
		requestAnimationFrame: (fn) => clock.setTimeout(fn, 16),
		fetch: (url, init) => {
			fetches.push({ url, init });
			return (opts.fetch || (() => Promise.reject(new Error("no fetch configured"))))(url, init);
		}
	});
	globalThis.setTimeout = (fn, ms) => clock.setTimeout(fn, ms);
	globalThis.setInterval = (fn, ms) => clock.setInterval(fn, ms);
	globalThis.clearTimeout = (id) => clock.clearTimeout(id);
	globalThis.clearInterval = (id) => clock.clearInterval(id);
	globalThis.Date.now = () => clock.now;

	const logs = [];
	console.log = (...a) => logs.push(a.map(String).join(" "));
	console.warn = (...a) => logs.push("WARN " + a.map(String).join(" "));

	return {
		clock, logs, store, storageListeners, fetches, messages, document,
		/** Puts `el` behind `sel`, or takes it away when `el` is nullish. */
		setNode(sel, el) { if (el) nodes.set(sel, el); else nodes.delete(sel); },
		/** Moves the page to `pathname`, as an SPA router would. */
		navigate(pathname) {
			globalThis.location.pathname = pathname;
			globalThis.location.href = "https://jira.company.local" + pathname;
		},
		/** Runs the MutationObserver callbacks currently observing. */
		fireMutation() { for (const cb of [...mutationCallbacks]) cb([], null); },
		/** Delivers `event` to the document listeners of `type`. */
		dispatch(type, event) { for (const fn of docListeners[type] || []) fn(event); },
		/** Delivers an event to the window listeners of `type`. */
		dispatchWindow(type, event) { for (const fn of winListeners[type] || []) fn(event); },
		/** Returns the toasts currently attached, oldest first. */
		toasts() {
			const c = findById(docEl, "sysinfo-toast-container");
			return (c ? c.children : []).map(t => ({
				kind: /sysinfo-toast--(\w+)/.exec(t.className)?.[1] || "?",
				text: t.text().trim().replace(/\s+/g, " "),
				fading: t.classList.contains("sysinfo-toast--fading")
			}));
		},
		/** Returns the message listener the service worker registered. */
		messageListener() { return browser.runtime._onMessage; }
	};
}

/** Returns the first node under `el` carrying `id`, or null. */
function findById(el, id) {
	if (el.id === id || el.attrs?.id === id) return el;
	for (const c of el.children) { const hit = findById(c, id); if (hit) return hit; }
	return null;
}
