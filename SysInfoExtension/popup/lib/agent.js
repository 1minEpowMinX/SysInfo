// Background-script communication + small side-effects shared across tabs.
// Each public function ends with a render() so the UI reflects new state.

import { state } from "./state.js";
import { render } from "./render.js";
import { normalizeSysInfo } from "../../shared/sysinfo_payload.js";

/**
 * The function `sendBg` sends a message to the background service worker and returns a Promise
 * that resolves with the response once the background script replies.
 * @param action - The `action` parameter is a string identifying the message type, matching a
 * handler registered in the background service worker.
 * @returns A Promise that resolves to the response object returned by the background script.
 */
function sendBg(action) {
	return new Promise(resolve =>
		browser.runtime.sendMessage({ action }, resolve)
	);
}

/**
 * The function `checkStatus` pings the background agent via `getStatus`, updates `state.status`
 * to reflect the result, and on a successful response kicks off `fetchVersion` and `fetchSysinfo`
 * to populate the remaining state. A `render()` is called both at the start and at the end so
 * the popup reflects the in-flight and settled states.
 */
export async function checkStatus() {
	state.status = "loading";
	render();
	try {
		const res = await sendBg("getStatus");
		if (res && res.success && (res.text || "").trim().startsWith("OK")) {
			state.status = "active";
			fetchVersion();
			fetchSysinfo();
		} else {
			state.status = "error";
		}
	} catch (e) {
		state.status = "error";
	}
	render();
}

/**
 * The function `fetchVersion` requests the agent version and build metadata from the background
 * script and stores the result in `state.agentVersion` and `state.agentBuild`. On failure the
 * previous values are left in place so the UI does not regress.
 */
async function fetchVersion() {
	try {
		const res = await sendBg("getVersion");
		if (res && res.success && res.data) {
			state.agentVersion = res.data.version ? "v" + res.data.version : null;
			state.agentBuild = res.data.build || null;
			render();
		}
	} catch (e) { /* leave previous values in state */ }
}

/**
 * The function `fetchSysinfo` requests current system information from the background script,
 * normalizes it onto the internal field names and stores the result in `state.sysinfo`, then
 * re-renders. On failure the previous sysinfo value is retained silently.
 */
export async function fetchSysinfo() {
	try {
		const res = await sendBg("getSystemInfo");
		if (res && res.success && res.data) {
			state.sysinfo = normalizeSysInfo(res.data);
			render();
		}
	} catch (e) { /* keep previous sysinfo in state */ }
}

/**
 * The function `copyValue` writes `value` to the system clipboard, then briefly sets
 * `state.copied` to `key` so the corresponding row can render a confirmation checkmark. The
 * copied indicator is automatically cleared after 1.1 seconds.
 * @param key - The field key (e.g. `"hostname"`) used to identify which row shows the copied
 * state.
 * @param value - The string value that will be written to the clipboard.
 */
export function copyValue(key, value) {
	navigator.clipboard.writeText(value).then(() => {
		state.copied = key;
		render();
		setTimeout(() => { state.copied = null; render(); }, 1100);
	}).catch(() => { /* clipboard denied — silent */ });
}
