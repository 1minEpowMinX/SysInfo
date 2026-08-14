// SPA navigation watcher.

import { slog } from "./compat.js";
import { URL_TICK_MS } from "./constants.js";
import { finalizeHistoryIfCreated } from "./history.js";

let lastPathname = location.pathname;

/**
 * Detects an SPA navigation by comparing `location.pathname` to the last recorded value.
 *
 * Each one gives the pending insertion its single chance to become a history entry.
 */
function checkUrlChange() {
	if (location.pathname === lastPathname) return;
	const prev = lastPathname;
	lastPathname = location.pathname;
	slog("url change", { from: prev, to: lastPathname });
	finalizeHistoryIfCreated();
}

/**
 * Installs the two hooks that catch an SPA navigation.
 */
export function setupUrlWatcher() {
	// pushState fires no event, so a poll is the only way to see a navigation the router makes
	// on its own; popstate then reports back/forward without waiting for the next tick.
	setInterval(checkUrlChange, URL_TICK_MS);
	window.addEventListener("popstate", checkUrlChange);
}
