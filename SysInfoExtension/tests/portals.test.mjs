// The whitelist: what it admits, what it falls back to, and how an edit reaches an open page.
//
// portals.js memoizes its read, so each case gets a process of its own.

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
	"stored: an emptied list falls back to the default": { [STORAGE_KEY]: { portals: [], types: ["27"] } },
	"paths: a pathname that is not a form is never allowed": { [STORAGE_KEY]: STORED },
	"edit: reaches an open page without a reload": { [STORAGE_KEY]: STORED },
	"edit: another key or another area is ignored": { [STORAGE_KEY]: STORED },
	"edit: removing the settings restores the defaults": { [STORAGE_KEY]: STORED },
	"load: the read happens once": { [STORAGE_KEY]: STORED }
};

const env = installEnv({ pathname: "/", storage: setups[process.argv[2]] || {} });
const portals = await import("../content/lib/portals.js");

const ready = portals.loadPortals();
env.clock.advance(10);
await ready;

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

	"stored: an emptied list falls back to the default"() {
		// The popup keeps an emptied list as a value of its own; here it reads as "nothing
		// stored". Emptying the portals in the popup therefore widens this list rather than
		// closing it.
		ok(portals.isTicketAllowed(`/servicedesk/customer/portal/${DEFAULT_PORTAL_IDS[0]}/create/27`),
			"an emptied portal list is answered from the defaults");
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
		eq(portals.loadPortals(), portals.loadPortals(), "the same promise is handed out");
		eq(env.storageListeners.length, 1, "and the listener is registered once");
	}
};

await run(import.meta, cases, { isolate: true });
