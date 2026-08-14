// Whitelist: two independent flat lists of IDs.

import { slog, swarn } from "./compat.js";
import { STORAGE_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS, FORM_PATH_RE } from "./constants.js";

let userPortals = null;
let userTypes = null;

/**
 * Reads the user's portal and type ID lists from storage into module state.
 *
 * Each list is taken separately, and one that is absent or malformed leaves its default in
 * effect.
 */
export function loadPortals() {
	try {
		browser.storage.local.get([STORAGE_KEY], (r) => {
			const stored = r && r[STORAGE_KEY];
			if (!stored) {
				slog("portals: no user config");
				return;
			}
			if (isStringArray(stored.portals)) {
				userPortals = stored.portals;
				slog("portals: loaded portal IDs", { count: userPortals.length });
			}
			if (isStringArray(stored.types)) {
				userTypes = stored.types;
				slog("portals: loaded type IDs", { count: userTypes.length });
			}
		});
	} catch (e) {
		swarn("portals: storage exception", e && e.message);
	}
}

/**
 * Reports whether `value` is a non-empty array holding strings alone.
 * @param value - The value to test.
 */
function isStringArray(value) {
	return Array.isArray(value) && value.length > 0 && value.every(v => typeof v === "string");
}

/** Returns the whitelisted portal IDs, falling back to the defaults while none are loaded. */
function getAllowedPortals() { return userPortals || DEFAULT_PORTAL_IDS; }
/** Returns the whitelisted ticket-type IDs, falling back to the defaults while none are loaded. */
function getAllowedTypes() { return userTypes || DEFAULT_TYPE_IDS; }

/**
 * Reports whether the form at `pathname` is one the block may be inserted into.
 *
 * The portal ID and the ticket-type ID are matched against their own lists, so a pair passes
 * only when both are whitelisted.
 * @param pathname - The pathname to match against FORM_PATH_RE.
 */
export function isTicketAllowed(pathname) {
	const match = pathname.match(FORM_PATH_RE);
	if (!match) return false;
	const [, portalId, ticketId] = match;
	return getAllowedPortals().includes(portalId)
		&& getAllowedTypes().includes(ticketId);
}
