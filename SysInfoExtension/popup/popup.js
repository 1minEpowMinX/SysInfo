// Popup bootstrap entry. All declarations live in lib/*.js.

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
	// Painted before the agent is reached, so the popup is never blank for the length of a
	// request to a service that may not be running.
	render();
	checkStatus();
})();
