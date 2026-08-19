// System information block: text formatting + agent fetch.

import { t, slog, swarn } from "./compat.js";
import { SYSINFO_REQUEST_RETRIES, SYSINFO_REQUEST_RETRY_MS } from "./constants.js";
import { normalizeSysInfo } from "../../shared/sysinfo_payload.js";
import { FIELD_KEYS, resolveFields } from "../../shared/fields.js";

// The agent reports an unobtainable value as an empty string: it carries data,
// and how a missing value is spelled belongs to the client that displays it.
// Without this the line would read "IP address: " and stop there. An absent key
// lands here too, so a field a later agent stops sending cannot reach the ticket
// as the text "undefined".
const orFallback = (value, fallbackKey) => value || t(fallbackKey);

// The label and the "no value" spelling of each field, keyed as FIELD_KEYS names them. Both are
// catalogue keys rather than text: the block is written in the browser's UI language.
const FIELD_TEXT = {
	hostname: { label: "sysinfoHostname", fallback: "sysinfoUnavailable" },
	username: { label: "sysinfoUsername", fallback: "sysinfoUnavailable" },
	ip: { label: "sysinfoIP", fallback: "sysinfoNoIp" },
	lastBootTime: { label: "sysinfoLastBootTime", fallback: "sysinfoUnavailable" }
};

/**
 * Renders the visible fields of an agent payload as labelled lines, joined in pairs.
 *
 * Pairing follows what is left after the hidden fields are dropped, so switching one off closes
 * the gap rather than leaving a line half empty.
 * @param data - A normalized payload, carrying `hostname`, `username`, `ip` and `lastBootTime`.
 * @param fields - A visibility flag per field, as `resolveFields` returns it; every field is
 * rendered when the caller names none.
 * @returns An array of strings, each holding up to two labelled fields joined by ", ". Empty
 * when every field is switched off.
 */
export function buildSysInfoLines(data, fields = resolveFields()) {
	const raw = FIELD_KEYS
		.filter(key => fields[key])
		.map(key => `${t(FIELD_TEXT[key].label)}: ${orFallback(data[key], FIELD_TEXT[key].fallback)}`);

	const result = [];
	for (let i = 0; i < raw.length; i += 2) {
		result.push(raw.slice(i, i + 2).join(", "));
	}
	return result;
}

/**
 * Returns a run of `char` as wide as `percent` of the longest string in `lines`.
 * @param lines - The lines the divider is drawn above.
 * @param char - The character the run is built from.
 * @param percent - The fraction of the longest line the run spans.
 * @returns The repeated character, rounded down to a whole number of characters; empty when
 * there is no line to measure against.
 */
export function makeDivider(lines, char = "─", percent = 0.45) {
	// Math.max of nothing is -Infinity, which repeat() answers with a RangeError rather than a
	// short divider.
	if (lines.length === 0) return "";
	const maxLen = Math.max(...lines.map(l => l.length));
	return char.repeat(Math.floor(maxLen * percent));
}

/**
 * Reports whether `target` already carries `divider`.
 * @param target - The editor element, read through `innerText`.
 * @param divider - The divider string to search for; an empty one is carried by every editor
 * there is, so it is answered as "not there" rather than as "already inserted".
 */
export function alreadyInserted(target, divider) {
	if (!divider) return false;
	return (target.innerText || "").includes(divider);
}

/**
 * Asks the background script for an agent payload, retrying on a messaging error, an
 * unsuccessful response or a thrown exception.
 *
 * Attempts are spaced SYSINFO_REQUEST_RETRY_MS apart and every outcome is logged.
 * @param callback - Receives the payload normalized onto the internal field names, or null once
 * every attempt has failed.
 * @param retriesLeft - The attempts still available.
 */
export function requestSysInfo(callback, retriesLeft = SYSINFO_REQUEST_RETRIES) {
	// The first call is an attempt of its own, so the run is one longer than the retry budget.
	const attempts = SYSINFO_REQUEST_RETRIES + 1;
	const attempt = attempts - retriesLeft;
	const tStart = Date.now();

	const retry = (reason) => {
		swarn("sysinfo fetch failed",
			`(attempt ${attempt}/${attempts})`,
			"reason=", reason,
			"elapsed=", Date.now() - tStart, "ms");
		if (retriesLeft > 0) {
			setTimeout(() => requestSysInfo(callback, retriesLeft - 1), SYSINFO_REQUEST_RETRY_MS);
		} else {
			swarn("sysinfo fetch: giving up after all retries");
			callback(null);
		}
	};

	try {
		browser.runtime.sendMessage({ action: "getSystemInfo" }, (res) => {
			if (browser.runtime.lastError) {
				retry(browser.runtime.lastError.message);
				return;
			}
			if (!res || !res.success) {
				retry((res && res.error) || "no response");
				return;
			}
			slog("sysinfo received", "(elapsed=", Date.now() - tStart, "ms)", res.data);
			// Normalizing here keeps the wire names off every path downstream.
			// A success carrying no payload stays null so the caller's own guard
			// against it still fires.
			callback(res.data ? normalizeSysInfo(res.data) : null);
		});
	} catch (e) {
		retry(e && e.message);
	}
}
