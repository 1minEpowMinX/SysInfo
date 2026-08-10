// Popup bootstrap entry. All declarations live in lib/*.js (imported
// below). Order of bootstrap calls:
//   1. loadSettings — pulls settings + history from storage into state.
//   2. render        — first paint, so the popup isn't blank while
//                      checkStatus is in flight.
//   3. checkStatus   — pings the agent; on success kicks off fetchSysinfo
//                      and re-renders the affected tabs.

import { loadSettings } from "./lib/storage.js";
import { render } from "./lib/render.js";
import { checkStatus } from "./lib/agent.js";
import { t } from "./lib/compat.js";

(async function init() {
	// Carried in the catalogues themselves because browser.i18n reports the
	// browser's UI language, which is not the language on screen once a
	// missing catalogue sends the messages back to the default locale.
	document.documentElement.lang = t("uiLocale");
	await loadSettings();
	render();
	checkStatus();
})();
