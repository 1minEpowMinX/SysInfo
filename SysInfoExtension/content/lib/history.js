// Deferred ticket history.

import { slog } from "./compat.js";
import { PENDING_TTL_MS, TICKET_PATH_RE, TITLE_SELECTOR, TITLE_WAIT_MS, HISTORY_KEY } from "./constants.js";

let pendingInsertion = null;

/**
 * The function markPendingInsertion sets a pending insertion with the specified portalId, typeId, and
 * timestamp.
 * @param portalId - Portal ID is a unique identifier for a specific portal in the system. It helps to
 * distinguish one portal from another.
 * @param typeId - Type ID is a unique identifier that represents the type of data or object being
 * inserted into the system. It helps differentiate between different types of data or objects within
 * the system.
 */
export function markPendingInsertion(portalId, typeId) {
	pendingInsertion = { portalId, typeId, at: Date.now() };
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
 * The function `finalizeHistoryIfCreated` checks if a pending ticket insertion has expired, detects a
 * created ticket, and saves its details to browser storage.
 * @returns If the `pendingInsertion` is not set, the function will return early. If the difference
 * between the current time and the time of the pending insertion is greater than the `PENDING_TTL_MS`,
 * then `null` will be returned. If the `created` ticket is not detected or if the `portalId` of the
 * created ticket does not match the `portalId` of the
 */
export function finalizeHistoryIfCreated() {
	if (!pendingInsertion) return;

	if (Date.now() - pendingInsertion.at > PENDING_TTL_MS) {
		pendingInsertion = null;
		return;
	}

	const created = detectCreatedTicket(location.pathname);
	if (!created) return;
	if (created.portalId !== pendingInsertion.portalId) return;

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
