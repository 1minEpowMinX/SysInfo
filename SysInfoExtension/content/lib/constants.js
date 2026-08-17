// Content-script constants. Storage keys and whitelist seeds come from
// the shared module; all other constants are content-script-specific.

export {
	STORAGE_KEY,
	HISTORY_KEY,
	DEFAULT_PORTAL_IDS,
	DEFAULT_TYPE_IDS,
} from "../../shared/constants.js";

// A submitted form is matched against the ticket page that follows it for at
// most this long. It bounds the one window a false entry can still slip
// through: a submission the portal rejected creates no ticket, and the record
// would otherwise wait for any same-portal ticket the user opens by hand.
// Generous against a slow creation, short against that.
export const SUBMIT_TTL_MS = 2 * 60 * 1000;

// Retry policy for the agent fetch: hard-failure retries only, no
// per-call timeout (sendMessage callbacks eventually fire).
export const SYSINFO_REQUEST_RETRIES = 5;
export const SYSINFO_REQUEST_RETRY_MS = 2000;

// Editor watcher tick + SPA URL polling cadence.
export const INSERTION_TICK_MS = 2000;
export const URL_TICK_MS = 500;

// How long a whitelisted form may go without producing an editor before the failure is reported.
// Long enough for a cold portal load over a slow link, short enough that the user is still on the
// form when the message arrives. A dead selector is caught whatever the value, so the value is
// chosen against reporting a page that is merely slow.
export const EDITOR_WAIT_MS = 30000;

// The same wait once the editor root is in the document, counted from the moment it appears. The
// root mounts before the paragraph it holds, so the grace period covers those few frames and
// nothing longer.
export const EDITOR_ROOT_GRACE_MS = 3000;

// Jira SM URL patterns: form (before submit) and ticket page (after).
export const FORM_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/create\/(\d+)/;
export const TICKET_PATH_RE = /\/servicedesk\/customer\/portal\/(\d+)\/([A-Z][A-Z0-9]+-\d+)(?:\/|$)/;

// AtlasKit editor root and the paragraph inside it that carries the description. The two are
// probed separately: a root without its paragraph is markup this extension does not match,
// while neither of them is a page that has not finished rendering.
export const EDITOR_ROOT_SELECTOR = "#ak-editor-textarea";
export const EDITOR_SELECTOR = `${EDITOR_ROOT_SELECTOR} > p`;

// Controls whose activation sends the request form. A real <form> reports itself
// through the submit event; these cover a portal build that handles the click
// itself and never fires one. Deliberately narrow — a control matched here that
// does not submit would let an unrelated ticket into the history.
//
// The portal's own send button is matched by its class pair inside the form's
// button container rather than by the full path from #content: the ancestor
// chain is ten levels of markup this extension does not own, while the pair is
// what makes the button the send control. A secondary button in the same
// container carries .aui-button without .aui-button-primary and so stays out.
export const SUBMIT_CONTROL_SELECTOR =
	'button[type="submit"], input[type="submit"], ' +
	"form .buttons-container button.aui-button.aui-button-primary";

// Ticket defer history selector: waits for the heading to appear, then grabs its text as the title.
export const TITLE_SELECTOR =
	"#content > div > header > div > div > div.cv-global-level-title > " +
	"div.aui-page-header-main.cv-page-title-main > h1 > span";
export const TITLE_WAIT_MS = 5000;
