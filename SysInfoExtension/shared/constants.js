// Shared constants — single source of truth for storage keys and default
// whitelist seeds used by both the content scripts and the popup.

export const STORAGE_KEY = "sysinfo_settings_v1";
export const HISTORY_KEY  = "sysinfo_history_v1";

// Two independent whitelist seeds — same shape used in storage.
// A form passes only when its URL's portal ID is in DEFAULT_PORTAL_IDS
// AND its ticket type is in DEFAULT_TYPE_IDS.
export const DEFAULT_PORTAL_IDS = ["41", "141"];
export const DEFAULT_TYPE_IDS = [
	"217", "218", "219", "220", "213", "216", "212",
	"181", "182", "183", "184", "185", "187", "192",
];
