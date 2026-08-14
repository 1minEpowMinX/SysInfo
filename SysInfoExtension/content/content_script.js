// Bootstrap entry. All declarations live in lib/*.js.

import { slog } from "./lib/compat.js";
import { loadPortals } from "./lib/portals.js";
import { setupUrlWatcher } from "./lib/url_watcher.js";
import { setupSubmitWatcher } from "./lib/submit_watcher.js";
import { startInsertion } from "./lib/insertion.js";

slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });

// Everything downstream asks isTicketAllowed whether it may act, and that function answers from
// the default lists until the stored ones arrive. Arming the watchers before then lets a form the
// user excluded pass on the first check.
loadPortals().then(() => {
	setupUrlWatcher();
	setupSubmitWatcher();
	startInsertion();
});
