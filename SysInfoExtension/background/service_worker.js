// Background service worker — proxies HTTP requests to the local SysInfo
// agent on localhost:8734 and reports its own diagnostic state.

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
 * The function fetchOptions returns an object with headers containing a key "X-Sysinfo-Client" with
 * the value of browser.runtime.id.
 * @returns An object with a `headers` property containing an object with a key-value pair of
 * `"X-Sysinfo-Client": browser.runtime.id`.
 */
function fetchOptions() {
	return { headers: { "X-Sysinfo-Client": browser.runtime.id } };
}

/**
 * The `proxyFetch` function sends a fetch request to a specified path, parses the response based on
 * the specified format, and sends the response or error message along with a label to the designated
 * function.
 * @param path - The `path` parameter in the `proxyFetch` function represents the URL path that you
 * want to fetch data from.
 * @param parseAs - The `parseAs` parameter in the `proxyFetch` function specifies how the response
 * data should be parsed. It can have two possible values:
 * @param sendResponse - The `sendResponse` parameter in the `proxyFetch` function is a function that
 * is used to send the response data back to the caller of the function. It is called with the response
 * data as an argument once the fetch operation is completed. The response data can either be the
 * parsed JSON data or
 * @param label - The `label` parameter is a string that is used to identify the specific fetch request
 * being made. It is used in logging messages to provide context for the request.
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
			bwarn(`${label} ✗`, err && err.message, "(", Date.now() - tStart, "ms)");
			sendResponse({ success: false, error: err.message });
		});
}

/* Setting up an event listener for messages sent to the background service
worker. When a message is received, it checks the `msg.action` property to determine the type of
action requested. */
browser.runtime.onMessage.addListener((msg, sender, sendResponse) => {
	if (!msg || !msg.action) return;
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

/* Setting up an event listener for the `onInstalled` event in the browser runtime API. */
browser.runtime.onInstalled && browser.runtime.onInstalled.addListener((details) => {
	blog("onInstalled", details);
	checkHostPermissions();
});

/* Setting up an event listener for the `onStartup` event in the browser runtime API. */
browser.runtime.onStartup && browser.runtime.onStartup.addListener(() => {
	blog("onStartup");
	checkHostPermissions();
});

/**
 * The function `checkHostPermissions` checks if the required host permissions are granted and logs the
 * status accordingly.
 * @returns The `checkHostPermissions` function returns either a log message indicating that no host
 * permissions are declared in the manifest, or a message indicating whether all required origins have
 * been granted permissions. If all required origins have been granted permissions, it logs a message
 * saying "permissions ✓ all required origins granted" along with the list of required origins. If some
 * origins are missing permissions, it logs a message saying "permissions.
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

// Also runs on every cold SW spin-up — gives a fresh report in the
// console regardless of which lifecycle event woke us.
checkHostPermissions();
