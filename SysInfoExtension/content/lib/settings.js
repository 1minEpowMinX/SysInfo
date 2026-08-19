// The stored settings the content script acts on: the two whitelist ID lists, and which fields
// the block carries. Read once and kept in step with the popup's later writes.

import { slog, swarn } from "./compat.js";
import { STORAGE_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS, FORM_PATH_RE } from "./constants.js";
import { isIdList } from "../../shared/id_list.js";
import { resolveFields } from "../../shared/fields.js";

let userPortals = null;
let userTypes = null;
let userFields = resolveFields();
let loaded = null;

/**
 * Puts the settings carried by `stored` into module state.
 *
 * Each list is taken separately, and one that is absent or malformed leaves its default in
 * effect. The field flags are resolved whole, so an absent group shows every field.
 * @param stored - The value held under STORAGE_KEY, or a falsy value when there is none.
 */
function applyStored(stored) {
	userPortals = isIdList(stored && stored.portals) ? stored.portals : null;
	userTypes = isIdList(stored && stored.types) ? stored.types : null;
	userFields = resolveFields(stored && stored.fields);
	slog("settings: applied", {
		portals: userPortals ? userPortals.length : "default",
		types: userTypes ? userTypes.length : "default",
		fields: userFields
	});
}

/** Applies every later write of the settings to module state. */
function watchStorage() {
	try {
		browser.storage.onChanged.addListener((changes, area) => {
			if (area !== "local" || !changes[STORAGE_KEY]) return;
			// An absent newValue is the key being removed, which applyStored reads as "defaults".
			applyStored(changes[STORAGE_KEY].newValue);
		});
	} catch (e) {
		swarn("settings: onChanged unavailable", e && e.message);
	}
}

/**
 * Reads the settings from storage into module state and keeps them in step with later edits.
 *
 * Calls after the first return the promise of the first.
 * @returns A promise settling once the settings are in module state, whether they were read or
 * left at their defaults. `isTicketAllowed` answers from the defaults until it settles.
 */
export function loadSettings() {
	if (loaded) return loaded;

	loaded = new Promise((resolve) => {
		try {
			browser.storage.local.get([STORAGE_KEY], (r) => {
				applyStored(r && r[STORAGE_KEY]);
				// The popup writes the settings while its own page is open, and this script would
				// otherwise answer from its load-time snapshot until the page is reloaded.
				// Registered after the read so that the snapshot cannot land on top of an edit
				// that arrived while it was in flight.
				watchStorage();
				resolve();
			});
		} catch (e) {
			// Resolved rather than rejected: the defaults are a working configuration, and a
			// storage that cannot be read must not keep the watchers unarmed.
			swarn("settings: storage exception", e && e.message);
			resolve();
		}
	});

	return loaded;
}

/** Returns the whitelisted portal IDs, falling back to the defaults while none is stored. */
function getAllowedPortals() { return userPortals || DEFAULT_PORTAL_IDS; }
/** Returns the whitelisted ticket-type IDs, falling back to the defaults while none is stored. */
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

/**
 * Returns the visibility flag of every field the block can carry.
 *
 * Read at insertion time rather than captured with the payload, so a field switched off while
 * the form is open is left out of the block that follows.
 * @returns A flag per key of FIELD_KEYS.
 */
export function visibleFields() {
	return userFields;
}
