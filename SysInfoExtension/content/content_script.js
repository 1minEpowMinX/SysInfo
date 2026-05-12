// Bootstrap entry. All declarations live in lib/*.js (loaded in order
// by the manifest).
//
//   1. loadPortals             — read whitelist arrays from storage.
//   2. finalizeHistoryIfCreated — covers the rare case of reloading the
//                                 page while already on a created ticket.
//   3. setupUrlWatcher         — react to SPA navigation between forms
//                                 and ticket pages.
//   4. startInsertion          — fetch sysinfo, then start the editor
//                                 watcher. Data is captured in closure
//                                 so the watcher tick is synchronous.

slog("bootstrap", { pathname: location.pathname, readyState: document.readyState });

loadPortals();
finalizeHistoryIfCreated();
setupUrlWatcher();
startInsertion();
