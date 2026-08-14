// Deferred ticket history.

import { slog } from "./compat.js";
import { SUBMIT_TTL_MS, TICKET_PATH_RE, TITLE_SELECTOR, TITLE_WAIT_MS, HISTORY_KEY } from "./constants.js";
import { isTicketAllowed } from "./portals.js";

let pendingInsertion = null;

/**
 * Records that the sysinfo block was inserted into the form at the current pathname.
 *
 * The record reaches the history only once `markFormSubmitted` reports the form sent; until
 * then the first navigation drops it.
 * @param portalId - The portal ID from the form URL.
 * @param typeId - The ticket-type ID from the form URL.
 */
export function markPendingInsertion(portalId, typeId) {
	pendingInsertion = { portalId, typeId, formPath: location.pathname, submittedAt: null };
}

/**
 * Records that the form carrying the pending insertion was sent.
 *
 * Sending again restarts the window, which is what an attempt the portal rejected leads to.
 */
export function markFormSubmitted() {
	if (!pendingInsertion) return;
	// Read from location rather than from the URL watcher's last poll: a submission can land
	// between two polls, and a stale pathname would reject a legitimate one.
	if (location.pathname !== pendingInsertion.formPath) return;
	pendingInsertion.submittedAt = Date.now();
	slog("history: form submitted", { formPath: pendingInsertion.formPath });
}

/**
 * Extracts the portal ID and the ticket key from a ticket-page pathname.
 * @param pathname - The pathname to match against TICKET_PATH_RE.
 * @returns An object carrying `portalId` and `ticketKey`, or null when the pathname is not a
 * ticket page.
 */
function detectCreatedTicket(pathname) {
	const m = pathname.match(TICKET_PATH_RE);
	return m ? { portalId: m[1], ticketKey: m[2] } : null;
}

/**
 * Waits for the ticket heading to enter the document and reads its text.
 * @returns A promise for the trimmed heading text, or null once TITLE_WAIT_MS has passed without
 * the heading appearing.
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
 * Saves a history entry when the current page is the ticket the submitted form created.
 *
 * Every other outcome drops the pending record rather than leaving it alive for a later URL
 * change, so a record survives at most one navigation.
 */
export function finalizeHistoryIfCreated() {
	if (!pendingInsertion) return;

	// The block was inserted but the form was never sent — the user navigated away from it.
	// This is what separates a ticket the form created from one merely opened afterwards, the
	// two being indistinguishable by pathname alone.
	if (!pendingInsertion.submittedAt) {
		pendingInsertion = null;
		return;
	}

	if (Date.now() - pendingInsertion.submittedAt > SUBMIT_TTL_MS) {
		pendingInsertion = null;
		return;
	}

	// Re-checked here and not only at insertion: the whitelist is editable while the form is
	// open, and a pair dropped from it in the meantime must not reach the history.
	if (!isTicketAllowed(pendingInsertion.formPath)) {
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
