// Background-script communication + small side-effects shared across tabs.
// Each public function ends with a render() so the UI reflects new state.

import { state } from "./state.js";
import { render } from "./render.js";
import { normalizeSysInfo } from "../../shared/sysinfo_payload.js";

/**
 * Sends an action to the background service worker and waits for its reply.
 * @param action - Names the message, matching a handler in the service worker.
 * @returns A promise for the reply object.
 */
function sendBg(action) {
	return new Promise(resolve =>
		browser.runtime.sendMessage({ action }, resolve)
	);
}

/**
 * Pings the agent and settles `state.status` on the outcome.
 *
 * A reachable agent then populates the rest of the state through `fetchVersion` and
 * `fetchSysinfo`. The popup is rendered on both sides of the request so the in-flight state is
 * shown as well as the settled one.
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
 * Reads the agent's version and build stamp into the state.
 *
 * A failure leaves the previous values in place rather than blanking the chips.
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
 * Reads the current system information into the state, normalized onto the internal field names.
 *
 * A failure leaves the previous values in place rather than blanking the rows.
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
 * Writes `value` to the clipboard and flashes the row's confirmation checkmark.
 *
 * A clipboard the browser refuses is silent: nothing is copied and no checkmark appears.
 * @param key - Names the field, marking which row shows the checkmark.
 * @param value - The text written to the clipboard.
 */
export function copyValue(key, value) {
	navigator.clipboard.writeText(value).then(() => {
		state.copied = key;
		render();
		setTimeout(() => { state.copied = null; render(); }, 1100);
	}).catch(() => { /* clipboard denied — silent */ });
}
