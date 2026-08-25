// Bootstrap entry. All declarations live in lib/*.js.

import { slog } from "./lib/compat.js";
import { loadSettings } from "./lib/settings.js";
import { setupUrlWatcher } from "./lib/url_watcher.js";
import { setupSubmitWatcher } from "./lib/submit_watcher.js";
import { startInsertion } from "./lib/insertion.js";

slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });

// Everything downstream asks isTicketAllowed whether it may act, and that function answers from
// the default lists until the stored ones arrive. Arming the watchers before then lets a form the
// user excluded pass on the first check.
loadSettings().then(() => {
	setupUrlWatcher();
	setupSubmitWatcher();
	startInsertion();
});
