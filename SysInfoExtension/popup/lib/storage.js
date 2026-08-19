// Persistence layer — settings and history live in browser.storage.local
// under the keys shared with the content script (see shared/constants.js).

import { STORAGE_KEY, HISTORY_KEY, defaultSettings } from "./constants.js";
import { state } from "./state.js";
import { isIdList } from "../../shared/id_list.js";
import { resolveFields } from "../../shared/fields.js";

/** Writes the current settings to storage under STORAGE_KEY. */
export function saveSettings() {
	browser.storage.local.set({ [STORAGE_KEY]: state.settings });
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
			fields: resolveFields(stored.fields),
			portals: isIdList(stored.portals) ? stored.portals : defaults.portals,
			types: isIdList(stored.types) ? stored.types : defaults.types
		};
		state.history = r[HISTORY_KEY] || [];
	} catch (e) { /* defaults from state.js stay in place */ }
}
