// Whitelist: two independent flat lists of IDs.

import { slog, swarn } from "./compat.js";
import { STORAGE_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS, FORM_PATH_RE } from "./constants.js";

let userPortals = null;
let userTypes = null;

/**
 * The function `loadPortals` retrieves user-configured portal and type IDs from local storage in
 * JavaScript.
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
 * The function `isStringArray` checks if a given value is an array containing only string elements.
 * @param value - The `value` parameter is the input that is being checked to determine if it is an
 * array containing only string values.
 * @returns The function `isStringArray` returns `true` if the input `value` is an array with at least
 * one element and all elements in the array are of type "string". Otherwise, it returns `false`.
 */
function isStringArray(value) {
	return Array.isArray(value) && value.length > 0 && value.every(v => typeof v === "string");
}

/**
 * The function `getAllowedPortals` returns either the user's portals or default portal IDs.
 * @returns The function `getAllowedPortals` returns the value of `userPortals` if it is truthy,
 * otherwise it returns the value of `DEFAULT_PORTAL_IDS`.
 */
function getAllowedPortals() { return userPortals || DEFAULT_PORTAL_IDS; }
/**
 * The function `getAllowedTypes` returns the `userTypes` array if it exists, otherwise it returns the
 * `DEFAULT_TYPE_IDS` array.
 * @returns The function `getAllowedTypes` returns the value of `userTypes` if it is defined, otherwise
 * it returns the value of `DEFAULT_TYPE_IDS`.
 */
function getAllowedTypes() { return userTypes || DEFAULT_TYPE_IDS; }

/**
 * The function `isTicketAllowed` checks if a ticket is allowed based on the portal ID and ticket ID
 * extracted from the pathname.
 * @param pathname - The `pathname` parameter is a string that represents the path of a URL.
 * @returns The function `isTicketAllowed` is returning a boolean value. It returns `true` if the
 * `portalId` extracted from the `pathname` is included in the list of allowed portals returned by
 * `getAllowedPortals()` function, and the `ticketId` extracted from the `pathname` is included in the
 * list of allowed ticket types returned by `getAllowedTypes()` function. Otherwise,
 */
export function isTicketAllowed(pathname) {
	const match = pathname.match(FORM_PATH_RE);
	if (!match) return false;
	const [, portalId, ticketId] = match;
	return getAllowedPortals().includes(portalId)
		&& getAllowedTypes().includes(ticketId);
}
