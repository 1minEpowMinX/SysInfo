// Popup constants. Storage keys and whitelist seeds come from the
// shared module; DEFAULT_SETTINGS is popup-specific.

import {
	STORAGE_KEY,
	HISTORY_KEY,
	DEFAULT_PORTAL_IDS,
	DEFAULT_TYPE_IDS,
} from "../../shared/constants.js";

export { STORAGE_KEY, HISTORY_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS };

export const DEFAULT_SETTINGS = {
	theme: "auto",
	fields: { hostname: true, username: true, ip: true, uptime: true },
	portals: DEFAULT_PORTAL_IDS,
	types: DEFAULT_TYPE_IDS
};
