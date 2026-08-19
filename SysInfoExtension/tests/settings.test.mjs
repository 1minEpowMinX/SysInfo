// The whitelist: what it admits, what it falls back to, and how an edit reaches an open page.
//
// settings.js memoizes its read, so each case gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv } from "./harness.mjs";
import { ok, eq } from "./assert.mjs";
import { STORAGE_KEY, DEFAULT_PORTAL_IDS, DEFAULT_TYPE_IDS } from "../shared/constants.js";

const STORED_FORM = "/servicedesk/customer/portal/3/create/27";
const DEFAULT_FORM = `/servicedesk/customer/portal/${DEFAULT_PORTAL_IDS[0]}/create/${DEFAULT_TYPE_IDS[0]}`;
const STORED = { portals: ["3"], types: ["27"] };

const setups = {
	"defaults: apply while nothing is stored": {},
	"stored: both lists take effect": { [STORAGE_KEY]: STORED },
	"stored: a malformed list falls back on its own": { [STORAGE_KEY]: { portals: ["3"], types: [7, 8] } },
	"stored: an emptied list admits nothing": { [STORAGE_KEY]: { portals: [], types: ["27"] } },
	"paths: a pathname that is not a form is never allowed": { [STORAGE_KEY]: STORED },
	"edit: reaches an open page without a reload": { [STORAGE_KEY]: STORED },
	"edit: another key or another area is ignored": { [STORAGE_KEY]: STORED },
	"edit: removing the settings restores the defaults": { [STORAGE_KEY]: STORED },
	"load: the read happens once": { [STORAGE_KEY]: STORED },
	"failure: a storage that throws still settles on the defaults": {}
};

const CASE = process.argv[2] || "";
const env = installEnv({ pathname: "/", storage: setups[CASE] || {} });

if (CASE.startsWith("failure:")) {
	browser.storage.local.get = () => { throw new Error("storage unavailable"); };
}

const portals = await import("../content/lib/settings.js");

// Awaited through a flag rather than directly: a loadSettings that never settles is the failure
// the last case is about, and awaiting the promise here would hang the process instead.
let settled = false;
const ready = portals.loadSettings();
ready.then(() => { settled = true; });
env.clock.advance(10);
await env.clock.flush();

/** Writes settings the way the popup does, which is what fires storage.onChanged. */
const write = (value) => browser.storage.local.set({ [STORAGE_KEY]: value });

const cases = {
	"defaults: apply while nothing is stored"() {
		ok(portals.isTicketAllowed(DEFAULT_FORM), "a default pair is allowed");
		ok(!portals.isTicketAllowed(STORED_FORM), "a pair outside the defaults is not");
	},

	"stored: both lists take effect"() {
		ok(portals.isTicketAllowed(STORED_FORM), "the stored pair is allowed");
		ok(!portals.isTicketAllowed(DEFAULT_FORM), "the defaults no longer apply");
		ok(!portals.isTicketAllowed("/servicedesk/customer/portal/3/create/99"),
			"a stored portal with an unlisted type is refused: both lists must admit the pair");
	},

	"stored: a malformed list falls back on its own"() {
		ok(portals.isTicketAllowed(`/servicedesk/customer/portal/3/create/${DEFAULT_TYPE_IDS[0]}`),
			"the unusable type list falls back while the portal list is kept");
		ok(!portals.isTicketAllowed(DEFAULT_FORM), "the usable portal list still applies");
	},

	"stored: an emptied list admits nothing"() {
		// The one state the two sides used to read differently: the popup keeps an emptied list
		// as a value of its own, so reading it as "nothing stored" would answer from the
		// defaults and widen the whitelist at the moment the user closed it.
		ok(!portals.isTicketAllowed(`/servicedesk/customer/portal/${DEFAULT_PORTAL_IDS[0]}/create/27`),
			"the defaults do not stand in for a list the user emptied");
		ok(!portals.isTicketAllowed(STORED_FORM), "and no portal is admitted by an empty list");
	},

	"paths: a pathname that is not a form is never allowed"() {
		ok(!portals.isTicketAllowed("/servicedesk/customer/portal/3/SD-1"), "a ticket page");
		ok(!portals.isTicketAllowed("/"), "the site root");
		ok(!portals.isTicketAllowed(""), "an empty pathname");
	},

	"edit: reaches an open page without a reload"() {
		ok(portals.isTicketAllowed(STORED_FORM), "allowed to begin with");
		write({ portals: ["9"], types: ["27"] });
		ok(!portals.isTicketAllowed(STORED_FORM), "the edit applies at once");
		write(STORED);
		ok(portals.isTicketAllowed(STORED_FORM), "and so does the edit back");
	},

	"edit: another key or another area is ignored"() {
		browser.storage.local.set({ somethingElse: { portals: ["9"] } });
		ok(portals.isTicketAllowed(STORED_FORM), "a write to another key changes nothing");
		for (const fn of env.storageListeners) fn({ [STORAGE_KEY]: { newValue: { portals: ["9"], types: [] } } }, "sync");
		ok(portals.isTicketAllowed(STORED_FORM), "a change in another storage area changes nothing");
	},

	"edit: removing the settings restores the defaults"() {
		write(undefined);
		ok(!portals.isTicketAllowed(STORED_FORM), "the stored pair is dropped");
		ok(portals.isTicketAllowed(DEFAULT_FORM), "the defaults are back");
	},

	"load: the read happens once"() {
		eq(portals.loadSettings(), portals.loadSettings(), "the same promise is handed out");
		eq(env.storageListeners.length, 1, "and the listener is registered once");
	},

	"failure: a storage that throws still settles on the defaults"() {
		ok(settled, "the promise settles, so the watchers waiting on it are armed");
		ok(portals.isTicketAllowed(DEFAULT_FORM), "the defaults are a working configuration");
		ok(!portals.isTicketAllowed(STORED_FORM), "nothing was read, so nothing is in effect");
	}
};

// The environment a case runs in is picked out of `setups` by the case's own name, so a rename
// that reaches one of the two lists alone would hand the case a fixture meant for another.
for (const name of Object.keys(setups)) {
	if (!(name in cases)) throw new Error(`the fixture "${name}" names no case`);
}
for (const name of Object.keys(cases)) {
	if (!(name in setups)) throw new Error(`the case "${name}" has no fixture`);
}

await run(import.meta, cases, { isolate: true });
