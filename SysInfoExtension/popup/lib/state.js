// Mutable popup state + the root DOM node the popup renders into.
// All render functions read from `state`; mutations go through helpers in
// agent.js / storage.js / dom event handlers and end with a `render()` call.

import "./compat.js";
import { defaultSettings } from "./constants.js";

export let state = {
	tab: "status",
	settings: defaultSettings(),
	// loading | active | error | version-mismatch. Nothing sets version-mismatch: no
	// compatibility rule is checked against the agent's version yet.
	status: "loading",
	agentVersion: null,
	agentBuild: null,
	extVersion: browser.runtime.getManifest().version,
	sysinfo: null,
	copied: null,
	history: []
};

export const root = document.getElementById("root");
