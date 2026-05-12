// Storage keys and built-in defaults for the popup-managed settings.

const STORAGE_KEY = "sysinfo_settings_v1";
const HISTORY_KEY = "sysinfo_history_v1";

// Two independent whitelist seeds — same shape used in storage.
// A form passes only when its URL's portal ID is in DEFAULT_PORTAL_IDS
// AND its ticket type is in DEFAULT_TYPE_IDS.
const DEFAULT_PORTAL_IDS = ["41", "141"];
const DEFAULT_TYPE_IDS = [
	"217", "218", "219", "220", "213", "216", "212",
	"181", "182", "183", "184", "185", "187", "192"
];

// Default settings for the popup
const DEFAULT_SETTINGS = {
	theme: "auto",
	fields: { hostname: true, username: true, ip: true, uptime: true },
	portals: DEFAULT_PORTAL_IDS,
	types: DEFAULT_TYPE_IDS
};
