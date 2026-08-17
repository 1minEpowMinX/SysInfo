// Reading the popup's settings out of storage: the merge over the defaults, the rename of the
// boot-time flag, and what an unusable value falls back to.
//
// The state module is shared, so each case gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv } from "./harness.mjs";
import { ok, eq, deepEq } from "./assert.mjs";
import { STORAGE_KEY, HISTORY_KEY, DEFAULT_PORTAL_IDS } from "../shared/constants.js";

const setups = {
	"nothing stored: the defaults apply": {},
	"stored: each setting is taken over its default": {
		[STORAGE_KEY]: { theme: "dark", portals: ["3"], types: ["27"] }
	},
	"stored: an unusable list falls back to its default": {
		[STORAGE_KEY]: { portals: [1, 2], types: "everything" }
	},
	"stored: an emptied list is kept as an emptied list": {
		[STORAGE_KEY]: { portals: [] }
	},
	"fields: a flag written under the old name is carried over": {
		[STORAGE_KEY]: { fields: { uptime: false, hostname: false } }
	},
	"fields: the old name does not overwrite the current one": {
		[STORAGE_KEY]: { fields: { uptime: false, lastBootTime: true } }
	},
	"fields: a flag the stored object omits keeps its default": {
		[STORAGE_KEY]: { fields: { hostname: false } }
	},
	"history: is read alongside the settings": {
		[HISTORY_KEY]: [{ id: "SD-1", title: "one" }]
	},
	"failure: a storage that throws leaves the defaults in place": {}
};

const CASE = process.argv[2] || "";
const env = installEnv({ pathname: "/", storage: setups[CASE] || {} });

if (CASE.startsWith("failure:")) {
	browser.storage.local.get = () => { throw new Error("storage unavailable"); };
}

const { state } = await import("../popup/lib/state.js");
const { loadSettings } = await import("../popup/lib/storage.js");
const { defaultSettings } = await import("../popup/lib/constants.js");

await loadSettings();

const cases = {
	"nothing stored: the defaults apply"() {
		deepEq(state.settings, defaultSettings(), "the whole settings object");
	},

	"stored: each setting is taken over its default"() {
		eq(state.settings.theme, "dark", "theme");
		deepEq(state.settings.portals, ["3"], "portals");
		deepEq(state.settings.types, ["27"], "types");
		deepEq(state.settings.fields, defaultSettings().fields, "an absent group keeps its defaults");
	},

	"stored: an unusable list falls back to its default"() {
		deepEq(state.settings.portals, DEFAULT_PORTAL_IDS, "a list of numbers is not a list of IDs");
		deepEq(state.settings.types, defaultSettings().types, "a string is not a list at all");
	},

	"stored: an emptied list is kept as an emptied list"() {
		// The content script reads an emptied list as "nothing stored" and answers from the
		// defaults; the popup keeps it. The two sides do not agree on what emptying means.
		deepEq(state.settings.portals, [], "the popup keeps the empty list");
	},

	"fields: a flag written under the old name is carried over"() {
		eq(state.settings.fields.lastBootTime, false, "a hidden field stays hidden under the new name");
		eq(state.settings.fields.uptime, undefined, "the old name is not kept");
		eq(state.settings.fields.hostname, false, "the other stored flags are untouched");
	},

	"fields: the old name does not overwrite the current one"() {
		eq(state.settings.fields.lastBootTime, true, "the current name wins");
		eq(state.settings.fields.uptime, undefined, "the old name is dropped either way");
	},

	"fields: a flag the stored object omits keeps its default"() {
		eq(state.settings.fields.hostname, false, "the stored flag");
		eq(state.settings.fields.ip, true, "the omitted one");
	},

	"history: is read alongside the settings"() {
		eq(state.history.length, 1, "one entry");
		eq(state.history[0].id, "SD-1", "the entry");
	},

	"failure: a storage that throws leaves the defaults in place"() {
		deepEq(state.settings, defaultSettings(), "settings");
		ok(Array.isArray(state.history), "and the history is still a list");
	}
};

// The storage a case starts from is picked out of `setups` by the case's own name, so a rename
// that reaches one of the two lists alone would hand the case a fixture meant for another.
for (const name of Object.keys(setups)) {
	if (!(name in cases)) throw new Error(`the fixture "${name}" names no case`);
}
for (const name of Object.keys(cases)) {
	if (!(name in setups)) throw new Error(`the case "${name}" has no fixture`);
}

await run(import.meta, cases, { isolate: true });
