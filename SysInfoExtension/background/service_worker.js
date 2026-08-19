// Background service worker — proxies HTTP requests to the local SysInfo
// agent at AGENT_BASE and reports its own diagnostic state.

// Cross-browser alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

const BG_TAG = "[SysInfo BG]";
const blog = (...args) => console.log(BG_TAG, ...args);
const bwarn = (...args) => console.warn(BG_TAG, ...args);

blog("service worker started", { runtimeId: browser.runtime.id });

const AGENT_BASE = "http://localhost:8734";

/**
 * Returns the fetch options every agent request carries, identifying this
 * extension instance to the agent.
 * @returns An options object holding the client header.
 */
function fetchOptions() {
	return { headers: { "X-Sysinfo-Client": browser.runtime.id } };
}

/**
 * Forwards a request to the agent and answers the caller with the parsed body.
 *
 * A failure answers with a `success: false` object carrying the message rather than rejecting.
 * Both outcomes are logged with their elapsed time.
 * @param path - Path appended to AGENT_BASE.
 * @param parseAs - Selects the parser and the reply shape: "text" parses text and answers
 * through `text`, "json" parses JSON and answers through `data`.
 * @param sendResponse - The onMessage reply callback.
 * @param label - Names the request in the log lines.
 */
function proxyFetch(path, parseAs, sendResponse, label) {
	const tStart = Date.now();
	fetch(AGENT_BASE + path, fetchOptions())
		.then(res => parseAs === "json" ? res.json() : res.text())
		.then(data => {
			blog(`${label} ✓`, "(", Date.now() - tStart, "ms)");
			sendResponse(parseAs === "text" ? { success: true, text: data } : { success: true, data });
		})
		.catch(err => {
			// Answered rather than left to reject, so that a caller waiting on the reply settles.
			// The message is read defensively on both lines: a rejection carrying no Error at all
			// would otherwise throw here and strand that caller.
			const reason = (err && err.message) || String(err);
			bwarn(`${label} ✗`, reason, "(", Date.now() - tStart, "ms)");
			sendResponse({ success: false, error: reason });
		});
}

browser.runtime.onMessage.addListener((msg, sender, sendResponse) => {
	if (!msg || !msg.action) return;
	// Only this extension's own content scripts and popup are served; a message
	// carrying another extension's ID is dropped without a reply.
	if (sender.id && sender.id !== browser.runtime.id) return;

	if (msg.action === "diagPing") {
		// Content-script-loaded heartbeat — visible in this SW console
		// even if the page console filters out content-script logs.
		const from = sender && sender.tab ? `tab#${sender.tab.id}` : "extension";
		blog("DIAG PING ←", { from, ...msg });
		sendResponse({ ok: true });
		return false;
	}

	if (msg.action === "getSystemInfo") {
		proxyFetch("/systeminfo", "json", sendResponse, "getSystemInfo");
		return true;
	}
	if (msg.action === "getStatus") {
		proxyFetch("/status", "text", sendResponse, "getStatus");
		return true;
	}
	if (msg.action === "getVersion") {
		proxyFetch("/version", "json", sendResponse, "getVersion");
		return true;
	}

	bwarn("unknown action", msg.action);
});

browser.runtime.onInstalled && browser.runtime.onInstalled.addListener((details) => {
	blog("onInstalled", details);
	checkHostPermissions();
});

browser.runtime.onStartup && browser.runtime.onStartup.addListener(() => {
	blog("onStartup");
	checkHostPermissions();
});

/**
 * Reports through the log whether every origin the manifest requires has been granted.
 */
async function checkHostPermissions() {
	const required = browser.runtime.getManifest().host_permissions || [];
	if (required.length === 0) {
		blog("permissions: manifest declares no host_permissions");
		return;
	}
	try {
		const missing = [];
		for (const origin of required) {
			const ok = await browser.permissions.contains({ origins: [origin] });
			if (!ok) missing.push(origin);
		}
		if (missing.length === 0) {
			blog("permissions ✓ all required origins granted", required);
		} else {
			bwarn("permissions ✗ NOT GRANTED:", missing,
				"— content_scripts won't auto-inject; grant via about:addons or enterprise policy");
		}
	} catch (e) {
		bwarn("checkHostPermissions failed:", e && e.message);
	}
}

// Also runs on every cold SW spin-up — gives a fresh report in the console
// regardless of which lifecycle event woke the worker.
checkHostPermissions();
