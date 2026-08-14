// Editor watcher + insertion of the sysinfo block.

import { t, slog, swarn } from "./compat.js";
import { FORM_PATH_RE, INSERTION_TICK_MS, EDITOR_SELECTOR } from "./constants.js";
import { buildSysInfoLines, makeDivider, alreadyInserted, requestSysInfo } from "./sysinfo.js";
import { isTicketAllowed } from "./portals.js";
import { showToast } from "./toast.js";
import { markPendingInsertion } from "./history.js";

/**
 * Writes the sysinfo block into `target` and records the insertion in the history.
 *
 * Returns without touching `target` when the path is not a ticket form, when the portal or the
 * ticket type is outside the whitelist, or when the block is already there.
 * @param target - The editor element, written through `value` when it has one and `innerText`
 * otherwise.
 * @param data - A normalized `/systeminfo` payload.
 */
function insertSysInfoInto(target, data) {
	const path = location.pathname;
	const formMatch = path.match(FORM_PATH_RE);
	if (!formMatch || !isTicketAllowed(path)) return;

	const lines = buildSysInfoLines(data);
	const divider = makeDivider(lines);
	// The Jira editor collapses a run of empty lines, so each one carries a
	// zero-width space to survive as a blank line above the block.
	const indents = "\n\u200B\n\u200B\n\u200B\n";
	const text = `${indents}${divider}\n${lines.join("\n")}`;

	if (alreadyInserted(target, divider)) return;

	slog("inserting sysinfo block", { path, target: target.tagName });

	if ("value" in target) {
		target.value = text;
		target.dispatchEvent(new Event("input", { bubbles: true }));
	} else {
		target.innerText = text;
	}

	showToast(t("toastReceived"), 10000);
	markPendingInsertion(formMatch[1], formMatch[2]);
}

/**
 * Inserts `data` into the editor every time a new editor element appears.
 *
 * A poll every INSERTION_TICK_MS and a MutationObserver on the body both drive the check.
 * @param data - A normalized `/systeminfo` payload, held in the closure so that a tick stays
 * synchronous.
 */
function watchEditor(data) {
	let lastElement = null;

	const check = () => {
		const el = document.querySelector(EDITOR_SELECTOR);
		if (el && el !== lastElement) {
			lastElement = el;
			insertSysInfoInto(el, data);
		}
	};

	slog("editor watcher armed", { selector: EDITOR_SELECTOR, intervalMs: INSERTION_TICK_MS });
	setInterval(check, INSERTION_TICK_MS);
	new MutationObserver(check).observe(document.body, { childList: true, subtree: true });
	check();
}

/**
 * Fetches an agent payload and arms the editor watcher with it.
 *
 * The watcher stays unarmed when the fetch fails.
 */
export function startInsertion() {
	requestSysInfo((data) => {
		if (!data) {
			swarn("insertion: no data — watcher not started");
			return;
		}
		watchEditor(data);
	});
}
