// SPA navigation watcher.

let lastPathname = location.pathname;

/**
 * The function `checkUrlChange` detects SPA navigations by comparing the current
 * `location.pathname` to the last recorded value. When a change is found, it updates the stored
 * pathname, emits a diagnostic log, and delegates to `finalizeHistoryIfCreated` so that any
 * pending ticket insertion is committed before the page context shifts.
 */
function checkUrlChange() {
	if (location.pathname === lastPathname) return;
	const prev = lastPathname;
	lastPathname = location.pathname;
	slog("url change", { from: prev, to: lastPathname });
	finalizeHistoryIfCreated();
}

/**
 * The function `setupUrlWatcher` installs the two hooks required to catch SPA navigations: a
 * polling interval for frameworks that mutate `history` without firing `popstate`, and a
 * `popstate` listener for browser back/forward actions. Both routes feed into `checkUrlChange`.
 */
function setupUrlWatcher() {
	setInterval(checkUrlChange, URL_TICK_MS);
	window.addEventListener("popstate", checkUrlChange);
}
