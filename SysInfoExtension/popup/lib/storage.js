// Persistence layer — settings and history live in browser.storage.local
// under the same keys the content script uses (see content/lib/constants.js).

import { STORAGE_KEY, HISTORY_KEY, DEFAULT_SETTINGS } from "./constants.js";
import { state } from "./state.js";

/**
 * The function `saveSettings` persists the current `state.settings` object to
 * `browser.storage.local` under `STORAGE_KEY`.
 */
export function saveSettings() {
	browser.storage.local.set({ [STORAGE_KEY]: state.settings });
}

/**
 * The function `asStringList` validates that `value` is an array containing only strings and
 * returns it as-is, including an empty array. Returns `null` when the value fails validation so
 * callers can fall back to a default.
 * @param value - The value to validate.
 * @returns The original array if it is a valid string array (including empty), or `null` if
 * validation fails.
 */
function asStringList(value) {
	if (!Array.isArray(value)) return null;
	if (value.length === 0) return [];
	return value.every(v => typeof v === "string") ? value : null;
}

/**
 * The function `loadSettings` reads settings and history from `browser.storage.local`, merges
 * them with defaults, and writes the result into `state.settings` and `state.history`. On any
 * storage error the state is left untouched so the popup renders with built-in defaults.
 */
export async function loadSettings() {
	try {
		const r = await browser.storage.local.get([STORAGE_KEY, HISTORY_KEY]);
		const stored = r[STORAGE_KEY] || {};
		state.settings = {
			theme: stored.theme || DEFAULT_SETTINGS.theme,
			fields: { ...DEFAULT_SETTINGS.fields, ...(stored.fields || {}) },
			portals: asStringList(stored.portals) || [...DEFAULT_SETTINGS.portals],
			types: asStringList(stored.types) || [...DEFAULT_SETTINGS.types]
		};
		state.history = r[HISTORY_KEY] || [];
	} catch (e) { }
}
