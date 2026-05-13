// System information block: text formatting + agent fetch.

import { t, slog, swarn } from "./compat.js";
import { SYSINFO_REQUEST_RETRIES, SYSINFO_REQUEST_RETRY_MS } from "./constants.js";

/**
 * The function `buildSysInfoLines` takes system information data and formats it into pairs of lines
 * for display.
 * @param data - The `buildSysInfoLines` function takes an object `data` as a parameter, which should
 * have the following properties:
 * @returns The function `buildSysInfoLines(data)` returns an array of strings where each element
 * contains two lines of system information data joined by a comma and a space.
 */
export function buildSysInfoLines(data) {
	const raw = [
		`${t("sysinfoHostname")}: ${data.hostname}`,
		`${t("sysinfoUsername")}: ${data.username}`,
		`${t("sysinfoIP")}: ${data.ip}`,
		`${t("sysinfoUptime")}: ${data.uptime}`
	];
	const result = [];
	for (let i = 0; i < raw.length; i += 2) {
		result.push(raw.slice(i, i + 2).join(", "));
	}
	return result;
}

/**
 * The `makeDivider` function generates a divider line made of a specified character repeated a certain
 * percentage of the length of the longest line in a given array of lines.
 * @param lines - Lines is an array of strings that you want to create a divider for. Each string in
 * the array represents a line of text.
 * @param [char=─] - The `char` parameter in the `makeDivider` function is used to specify the
 * character that will be repeated to create the divider line. By default, it is set to "─" which is
 * the em dash character. You can change this parameter to any character you prefer when calling the
 * function.
 * @param [percent=0.45] - The `percent` parameter in the `makeDivider` function determines what
 * percentage of the maximum line length should be used to create the divider. It is set to a default
 * value of 0.45, meaning that by default, the divider will be 45% of the maximum line length. You
 * @returns The `makeDivider` function returns a string consisting of the character specified repeated
 * a number of times based on the maximum length of the lines provided and the percentage specified.
 */
export function makeDivider(lines, char = "─", percent = 0.45) {
	const maxLen = Math.max(...lines.map(l => l.length));
	return char.repeat(Math.floor(maxLen * percent));
}

/**
 * The function `alreadyInserted` checks if a target element already contains a specific divider
 * string.
 * @param target - The `target` parameter is the element or object where you want to check if a
 * specific `divider` is already inserted. It could be an HTML element, a string, or any object that
 * has a `value` or `innerText` property.
 * @param divider - The `divider` parameter is the string that we are checking for in the `target`
 * element. It is the substring that we want to see if it is already present in the `target` element.
 * @returns The function `alreadyInserted` returns a boolean value indicating whether the `divider` is
 * present in the `target` element's value or inner text.
 */
export function alreadyInserted(target, divider) {
	const haystack = ("value" in target ? target.value : target.innerText) || "";
	return haystack.includes(divider);
}

/**
 * The function `requestSysInfo` attempts to fetch system information with retries and provides
 * feedback in case of failure.
 * @param callback - The `callback` parameter is a function that will be called with the system
 * information data once it is successfully retrieved.
 * @param [retriesLeft] - The `retriesLeft` parameter in the `requestSysInfo` function represents the
 * number of retries left for fetching system information. It is used to keep track of how many more
 * times the function can attempt to retrieve the system information in case of failures. The default
 * value for `retriesLeft`
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
			callback(res.data);
		});
	} catch (e) {
		retry(e && e.message);
	}
}
