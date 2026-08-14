// Bootstrap entry. All declarations live in lib/*.js.

import { slog } from "./lib/compat.js";
import { loadPortals } from "./lib/portals.js";
import { setupUrlWatcher } from "./lib/url_watcher.js";
import { setupSubmitWatcher } from "./lib/submit_watcher.js";
import { startInsertion } from "./lib/insertion.js";

slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });

loadPortals();
setupUrlWatcher();
setupSubmitWatcher();
startInsertion();
