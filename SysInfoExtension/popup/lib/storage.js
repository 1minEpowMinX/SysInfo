// Persistence layer — settings and history live in browser.storage.local
// under the keys shared with the content script (see shared/constants.js).

import { STORAGE_KEY, HISTORY_KEY, defaultSettings } from "./constants.js";
import { state } from "./state.js";

/** Writes the current settings to storage under STORAGE_KEY. */
export function saveSettings() {
	browser.storage.local.set({ [STORAGE_KEY]: state.settings });
}

/**
 * Accepts `value` as a list of IDs.
 * @param value - The value read from storage.
 * @returns The array itself when it holds strings alone, an empty array included, and null
 * otherwise, which lets the caller tell an emptied list from an unusable one.
 */
function asStringList(value) {
	if (!Array.isArray(value)) return null;
	if (value.length === 0) return [];
	return value.every(v => typeof v === "string") ? value : null;
}

/**
 * Renames the stored field-visibility flags written by a build that still called the last boot
 * time "uptime", so that a user who hid the field keeps it hidden.
 * @param stored - The `fields` object read from storage, or a nullish value when absent.
 * @returns A copy carrying the current names alone.
 */
function migrateFields(stored) {
	const fields = { ...(stored || {}) };
	if (fields.uptime !== undefined && fields.lastBootTime === undefined) {
		fields.lastBootTime = fields.uptime;
	}
	delete fields.uptime;
	return fields;
}

/**
 * Reads the settings and the history from storage into the state, each setting merged over its
 * default.
 *
 * Does not throw: a storage error leaves the built-in defaults in place.
 */
export async function loadSettings() {
	try {
		const r = await browser.storage.local.get([STORAGE_KEY, HISTORY_KEY]);
		const stored = r[STORAGE_KEY] || {};
		const defaults = defaultSettings();
		state.settings = {
			theme: stored.theme || defaults.theme,
			fields: { ...defaults.fields, ...migrateFields(stored.fields) },
			portals: asStringList(stored.portals) || defaults.portals,
			types: asStringList(stored.types) || defaults.types
		};
		state.history = r[HISTORY_KEY] || [];
	} catch (e) { /* defaults from state.js stay in place */ }
}
