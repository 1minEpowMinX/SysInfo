// System information block: text formatting + agent fetch.

import { t, slog, swarn } from "./compat.js";
import { SYSINFO_REQUEST_RETRIES, SYSINFO_REQUEST_RETRY_MS } from "./constants.js";
import { normalizeSysInfo } from "../../shared/sysinfo_payload.js";

// The agent reports an unobtainable value as an empty string: it carries data,
// and how a missing value is spelled belongs to the client that displays it.
// Without this the line would read "IP address: " and stop there. An absent key
// lands here too, so a field a later agent stops sending cannot reach the ticket
// as the text "undefined".
const orFallback = (value, fallbackKey) => value || t(fallbackKey);

/**
 * Renders the fields of an agent payload as labelled lines, joined in pairs.
 * @param data - A normalized payload, carrying `hostname`, `username`, `ip` and `lastBootTime`.
 * @returns An array of strings, each holding two labelled fields joined by ", ".
 */
export function buildSysInfoLines(data) {
	const raw = [
		`${t("sysinfoHostname")}: ${orFallback(data.hostname, "sysinfoUnavailable")}`,
		`${t("sysinfoUsername")}: ${orFallback(data.username, "sysinfoUnavailable")}`,
		`${t("sysinfoIP")}: ${orFallback(data.ip, "sysinfoNoIp")}`,
		`${t("sysinfoLastBootTime")}: ${orFallback(data.lastBootTime, "sysinfoUnavailable")}`
	];
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
 * @returns The repeated character, rounded down to a whole number of characters.
 */
export function makeDivider(lines, char = "─", percent = 0.45) {
	const maxLen = Math.max(...lines.map(l => l.length));
	return char.repeat(Math.floor(maxLen * percent));
}

/**
 * Reports whether `target` already carries `divider`.
 * @param target - The editor element, read through `value` when it has one and `innerText`
 * otherwise.
 * @param divider - The divider string to search for.
 */
export function alreadyInserted(target, divider) {
	const haystack = ("value" in target ? target.value : target.innerText) || "";
	return haystack.includes(divider);
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
	const attempt = SYSINFO_REQUEST_RETRIES - retriesLeft + 1;
	const tStart = Date.now();

	const retry = (reason) => {
		swarn("sysinfo fetch failed",
			`(attempt ${attempt}/${SYSINFO_REQUEST_RETRIES})`,
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
