// Bootstrap entry. All declarations live in lib/*.js (imported below).
//
//   1. loadPortals             — read whitelist arrays from storage.
//   2. finalizeHistoryIfCreated — covers the rare case of reloading the
//                                 page while already on a created ticket.
//   3. setupUrlWatcher         — react to SPA navigation between forms
//                                 and ticket pages.
//   4. startInsertion          — fetch sysinfo, then start the editor
//                                 watcher. Data is captured in closure
//                                 so the watcher tick is synchronous.

import { slog } from "./lib/compat.js";
import { loadPortals } from "./lib/portals.js";
import { finalizeHistoryIfCreated } from "./lib/history.js";
import { setupUrlWatcher } from "./lib/url_watcher.js";
import { startInsertion } from "./lib/insertion.js";

slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });

loadPortals();
finalizeHistoryIfCreated();
setupUrlWatcher();
startInsertion();
