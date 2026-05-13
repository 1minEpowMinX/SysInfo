// Mutable popup state + the root DOM node we render into.
// All render functions read from `state`; mutations go through helpers in
// agent.js / storage.js / dom event handlers and end with a `render()` call.

import "./compat.js";
import { DEFAULT_SETTINGS } from "./constants.js";

export let state = {
	tab: "status",
	settings: { ...DEFAULT_SETTINGS },
	status: "loading", // loading | active | error | version-mismatch
	agentVersion: null,
	agentBuild: null,
	extVersion: browser.runtime.getManifest().version,
	sysinfo: null,
	copied: null,
	history: []
};

export const root = document.getElementById("root");
