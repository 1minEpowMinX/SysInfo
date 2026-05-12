// Storage keys, URL patterns, defaults, and timing constants.

const STORAGE_KEY = "sysinfo_settings_v1";
const HISTORY_KEY = "sysinfo_history_v1";

// A pending insertion (filled form, not yet submitted) is kept around
// at most this long before being discarded.
const PENDING_TTL_MS = 30 * 60 * 1000;

// Retry policy for the agent fetch: hard-failure retries only, no
// per-call timeout (sendMessage callbacks eventually fire).
const SYSINFO_REQUEST_RETRIES = 5;
const SYSINFO_REQUEST_RETRY_MS = 2000;

// Editor watcher tick + SPA URL polling cadence.
const INSERTION_TICK_MS = 2000;
const URL_TICK_MS = 500;

// Jira SM URL patterns: form (before submit) and ticket page (after).
const FORM_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/create\/(\d+)/;
const TICKET_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/([A-Z][A-Z0-9]+-\d+)(?:\/|$)/;

// AtlasKit editor paragraph inside the description field.
const EDITOR_SELECTOR = "#ak-editor-textarea > p";

// Ticket defer history selector: waits for the heading to appear, then grabs its text as the title.
const TITLE_SELECTOR =
	"#content > div > header > div > div > div.cv-global-level-title > " +
	"div.aui-page-header-main.cv-page-title-main > h1 > span";
const TITLE_WAIT_MS = 5000;

// Two independent whitelist seeds — same shape used in storage.
// A form passes only when its URL's portal ID is in DEFAULT_PORTAL_IDS
// AND its ticket type is in DEFAULT_TYPE_IDS.
const DEFAULT_PORTAL_IDS = ["41", "141"];
const DEFAULT_TYPE_IDS = [
	"217", "218", "219", "220", "213", "216", "212",
	"181", "182", "183", "184", "185", "187", "192",
];