// Popup constants. Storage keys and whitelist seeds come from the
// shared module; the settings defaults are popup-specific.

import {
	STORAGE_KEY,
	HISTORY_KEY,
	DEFAULT_PORTAL_IDS,
	DEFAULT_TYPE_IDS,
} from "../../shared/constants.js";
import { resolveFields } from "../../shared/fields.js";

export { STORAGE_KEY, HISTORY_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS };

// The template, never handed out: `portals` and `types` are the shared seed
// arrays themselves, and `fields` is one object. The Settings tab edits
// state.settings in place — a toggle assigns into `fields`, the chip editor
// pushes and splices the arrays — so a caller holding this object would write
// through it to the defaults, and past them into shared/constants.js.
const DEFAULT_SETTINGS = {
	theme: "auto",
	// Which fields exist, and that an unmentioned one is shown, is resolveFields()'s to say.
	fields: resolveFields(),
	portals: DEFAULT_PORTAL_IDS,
	types: DEFAULT_TYPE_IDS
};

/**
 * Returns the built-in settings as an object no other holder shares: the nested `fields` and
 * the two arrays are copies of their own.
 * @returns A settings object safe to edit in place.
 */
export function defaultSettings() {
	return {
		...DEFAULT_SETTINGS,
		fields: { ...DEFAULT_SETTINGS.fields },
		portals: [...DEFAULT_SETTINGS.portals],
		types: [...DEFAULT_SETTINGS.types]
	};
}
