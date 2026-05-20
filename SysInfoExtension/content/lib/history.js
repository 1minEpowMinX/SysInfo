// Deferred ticket history.

import { slog } from "./compat.js";
import { PENDING_TTL_MS, TICKET_PATH_RE, TITLE_SELECTOR, TITLE_WAIT_MS, HISTORY_KEY } from "./constants.js";

let pendingInsertion = null;

/**
 * The function `markPendingInsertion` records that sysinfo was just inserted into the form at the
 * current pathname. The record is later matched against a ticket-page URL by
 * `finalizeHistoryIfCreated` to confirm the ticket was actually created from this form.
 * @param portalId - The portal ID extracted from the form URL.
 * @param typeId - The ticket-type ID extracted from the form URL.
 */
export function markPendingInsertion(portalId, typeId) {
	pendingInsertion = { portalId, typeId, formPath: location.pathname, at: Date.now() };
}

/**
 * The function `detectCreatedTicket` extracts the portal ID and ticket key from a given pathname using
 * a regular expression.
 * @param pathname - The `pathname` parameter is a string that represents the URL path of a web page.
 * @returns An object with the properties `portalId` and `ticketKey` if the `pathname` matches the
 * `TICKET_PATH_RE` regular expression pattern, otherwise `null` is returned.
 */
function detectCreatedTicket(pathname) {
	const m = pathname.match(TICKET_PATH_RE);
	return m ? { portalId: m[1], ticketKey: m[2] } : null;
}

/**
 * The function `waitForHeading` uses a Promise to wait for a specific heading element to appear in the
 * document and resolves with the text content of the heading once it is available.
 * @returns The `waitForHeading` function returns a Promise that resolves to the text content of the
 * heading element selected by the `TITLE_SELECTOR` constant after it becomes available in the
 * document. If the heading element is already present, the Promise resolves immediately with the
 * trimmed text content of the existing heading. If the heading element is not found within the
 * specified time limit (`TITLE_WAIT_MS`), the Promise resolves with `
 */
function waitForHeading() {
	return new Promise((resolve) => {
		const existing = document.querySelector(TITLE_SELECTOR);
		if (existing) {
			resolve(existing.textContent.trim());
			return;
		}

		const timer = setTimeout(() => {
			observer.disconnect();
			resolve(null);
		}, TITLE_WAIT_MS);

		const observer = new MutationObserver(() => {
			const el = document.querySelector(TITLE_SELECTOR);
			if (!el) return;
			clearTimeout(timer);
			observer.disconnect();
			resolve(el.textContent.trim());
		});

		observer.observe(document.body, { childList: true, subtree: true });
	});
}

/**
 * The function `finalizeHistoryIfCreated` checks whether the current page is the ticket that was
 * created from the pending form insertion and, if so, saves an entry to history.
 *
 * `fromPath` must be the pathname from which the SPA navigation originated. If provided, the
 * function requires that it matches the form path recorded by `markPendingInsertion` — any other
 * navigation (user clicking a link, back/forward) clears the pending state and returns without
 * saving, preventing false history entries for tickets the user merely visited.
 *
 * All mismatching or ambiguous conditions clear `pendingInsertion` rather than leaving it alive for
 * a later URL change, which was the source of the false-positive bug.
 *
 * @param fromPath - The `location.pathname` before the navigation that triggered this call, or
 * `undefined` when called at bootstrap (in which case the form-path guard is skipped).
 */
export function finalizeHistoryIfCreated(fromPath) {
	if (!pendingInsertion) return;

	if (Date.now() - pendingInsertion.at > PENDING_TTL_MS) {
		pendingInsertion = null;
		return;
	}

	// Navigation must originate directly from the form that recorded the insertion.
	// Any intermediate stop (another page, a different form) invalidates the pending state.
	if (fromPath !== undefined && fromPath !== pendingInsertion.formPath) {
		pendingInsertion = null;
		return;
	}

	const created = detectCreatedTicket(location.pathname);
	if (!created || created.portalId !== pendingInsertion.portalId) {
		pendingInsertion = null;
		return;
	}

	slog("history: ticket created", created);

	const pending = pendingInsertion;
	pendingInsertion = null;

	waitForHeading().then((headingText) => {
		const cleanTitle = (
			headingText ||
			document.title.replace(/\s*-\s*Jira.*$/i, "").trim()
		).slice(0, 120);

		browser.storage.local.get([HISTORY_KEY], (r) => {
			const list = (r && r[HISTORY_KEY]) || [];
			if (list.some(it => it.id === created.ticketKey)) return;

			const entry = {
				id: created.ticketKey,
				portalId: pending.portalId,
				typeId: pending.typeId,
				title: cleanTitle || created.ticketKey,
				url: location.href,
				when: Date.now()
			};

			const updated = [entry, ...list].slice(0, 20);
			browser.storage.local.set({ [HISTORY_KEY]: updated });
			slog("history: saved", entry);
		});
	});
}
