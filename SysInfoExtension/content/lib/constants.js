// Content-script constants. Storage keys and whitelist seeds come from
// the shared module; all other constants are content-script-specific.

export {
	STORAGE_KEY,
	HISTORY_KEY,
	DEFAULT_PORTAL_IDS,
	DEFAULT_TYPE_IDS,
} from "../../shared/constants.js";

// A pending insertion (filled form, not yet submitted) is kept around
// at most this long before being discarded.
export const PENDING_TTL_MS = 30 * 60 * 1000;

// Retry policy for the agent fetch: hard-failure retries only, no
// per-call timeout (sendMessage callbacks eventually fire).
export const SYSINFO_REQUEST_RETRIES = 5;
export const SYSINFO_REQUEST_RETRY_MS = 2000;

// Editor watcher tick + SPA URL polling cadence.
export const INSERTION_TICK_MS = 2000;
export const URL_TICK_MS = 500;

// Jira SM URL patterns: form (before submit) and ticket page (after).
export const FORM_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/create\/(\d+)/;
export const TICKET_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/([A-Z][A-Z0-9]+-\d+)(?:\/|$)/;

// AtlasKit editor paragraph inside the description field.
export const EDITOR_SELECTOR = "#ak-editor-textarea > p";

// Ticket defer history selector: waits for the heading to appear, then grabs its text as the title.
export const TITLE_SELECTOR =
	"#content > div > header > div > div > div.cv-global-level-title > " +
	"div.aui-page-header-main.cv-page-title-main > h1 > span";
export const TITLE_WAIT_MS = 5000;
