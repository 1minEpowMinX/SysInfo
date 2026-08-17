// Editor watcher + insertion of the sysinfo block.

import { t, slog, swarn } from "./compat.js";
import {
	FORM_PATH_RE,
	INSERTION_TICK_MS,
	EDITOR_SELECTOR,
	EDITOR_ROOT_SELECTOR,
	EDITOR_WAIT_MS,
	EDITOR_ROOT_GRACE_MS
} from "./constants.js";
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

/** Returns the current pathname when it is a whitelisted ticket form, and null otherwise. */
function whitelistedFormPath() {
	const path = location.pathname;
	return FORM_PATH_RE.test(path) && isTicketAllowed(path) ? path : null;
}

/**
 * Inserts `data` into the editor every time a new editor element appears, and reports the two
 * failures that leave a whitelisted form without a block.
 *
 * A poll every INSERTION_TICK_MS and a MutationObserver on the body both drive the check. Each
 * form path is reported at most once, whichever of the two failures it hits, and a report of a
 * missing editor is withdrawn if the editor turns up after all.
 * @param data - A normalized `/systeminfo` payload, or null when the agent could not be reached.
 */
function watchEditor(data) {
	let lastElement = null;
	let armedForm = null;
	let deadline = 0;
	let rootSeenAt = 0;
	let dismissWaitReport = null;
	const reported = new Set();

	const check = () => {
		// Both failures are reported against a whitelisted form and nothing else: the content
		// script runs on every page of the host, and a report tied to the failure rather than to
		// the form would reach pages where no block was ever due.
		const formPath = whitelistedFormPath();
		// The deadline belongs to one form: an SPA navigation to another restarts the wait.
		if (formPath !== armedForm) {
			armedForm = formPath;
			deadline = Date.now() + EDITOR_WAIT_MS;
			rootSeenAt = 0;
		}

		const el = document.querySelector(EDITOR_SELECTOR);
		if (el) {
			if (el === lastElement) return;
			lastElement = el;
			// The editor arrived after the wait was reported, so the report describes a page
			// state that no longer holds and would otherwise stand next to the success.
			if (dismissWaitReport) {
				dismissWaitReport();
				dismissWaitReport = null;
			}
			if (data) {
				// Read from the closure rather than fetched here, which keeps a tick synchronous.
				insertSysInfoInto(el, data);
			} else if (formPath && !reported.has(formPath)) {
				reported.add(formPath);
				swarn("insertion: agent unreachable on a whitelisted form", { path: formPath });
				showToast(t("toastAgentUnreachable"), 0, "error");
			}
			return;
		}

		if (!formPath || reported.has(formPath)) return;

		const rootPresent = !!document.querySelector(EDITOR_ROOT_SELECTOR);
		if (!rootPresent) rootSeenAt = 0;
		else if (!rootSeenAt) rootSeenAt = Date.now();

		// A root standing without its paragraph is markup that no further waiting resolves, so it
		// is answered on the grace period rather than on the full one.
		const due = rootSeenAt ? Math.min(deadline, rootSeenAt + EDITOR_ROOT_GRACE_MS) : deadline;
		if (Date.now() < due) return;

		reported.add(formPath);
		swarn("insertion: no editor on a whitelisted form",
			{ path: formPath, selector: EDITOR_SELECTOR, rootPresent, waitedMs: Date.now() - (deadline - EDITOR_WAIT_MS) });
		dismissWaitReport = showToast(t("toastEditorMissing"), 0, "error");
	};

	slog("editor watcher armed", { selector: EDITOR_SELECTOR, intervalMs: INSERTION_TICK_MS });
	setInterval(check, INSERTION_TICK_MS);
	new MutationObserver(check).observe(document.body, { childList: true, subtree: true });
	check();
}

/**
 * Fetches an agent payload and arms the editor watcher with it.
 *
 * A failed fetch arms the watcher all the same, with nothing to insert.
 */
export function startInsertion() {
	requestSysInfo((data) => {
		// Armed even without a payload: the failure is worth reporting only once the user reaches
		// a form, which happens long after this call.
		if (!data) swarn("insertion: no data — watcher reports instead of inserting");
		watchEditor(data);
	});
}
