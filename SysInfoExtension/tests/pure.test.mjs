// Modules that read no browser API: the payload mapping, the URL patterns and the settings
// template.

import { run } from "./runner.mjs";
import { ok, eq, deepEq } from "./assert.mjs";
import { normalizeSysInfo, BOOT_TIME_KEY, LEGACY_BOOT_TIME_KEY } from "../shared/sysinfo_payload.js";
import { FORM_PATH_RE, TICKET_PATH_RE, SUBMIT_CONTROL_SELECTOR } from "../content/lib/constants.js";
import { defaultSettings } from "../popup/lib/constants.js";
import { DEFAULT_PORTAL_IDS } from "../shared/constants.js";

const cases = {
	"payload: carries the four fields through"() {
		const out = normalizeSysInfo({ hostname: "PC", username: "u", ip: "10.0.0.1", [BOOT_TIME_KEY]: "T" });
		deepEq(out, { hostname: "PC", username: "u", ip: "10.0.0.1", lastBootTime: "T" }, "mapped payload");
	},

	"payload: falls back to the legacy boot-time key"() {
		eq(normalizeSysInfo({ [LEGACY_BOOT_TIME_KEY]: "T" }).lastBootTime, "T", "legacy key read");
	},

	"payload: an empty new key is not read as an absent one"() {
		// ?? and not ||, so an agent that has moved to the new key and reports it unobtainable
		// keeps its empty string instead of showing the legacy value.
		const out = normalizeSysInfo({ [BOOT_TIME_KEY]: "", [LEGACY_BOOT_TIME_KEY]: "stale" });
		eq(out.lastBootTime, "", "empty new key wins over the legacy one");
	},

	"payload: a nullish argument yields undefined fields"() {
		const out = normalizeSysInfo(null);
		deepEq(Object.keys(out).sort(), ["hostname", "ip", "lastBootTime", "username"], "field names");
		eq(out.hostname, undefined, "absent field stays undefined");
	},

	"payload: an omitted field is not filled in"() {
		eq(normalizeSysInfo({ hostname: "PC" }).ip, undefined, "omitted ip");
	},

	"paths: a create form is matched and its two IDs captured"() {
		const m = "/servicedesk/customer/portal/41/create/217".match(FORM_PATH_RE);
		ok(m !== null, "the form path should match");
		deepEq([m?.[1], m?.[2]], ["41", "217"], "portal and type IDs");
	},

	"paths: a form URL with a query and a trailing segment still matches"() {
		ok(FORM_PATH_RE.test("/servicedesk/customer/portal/41/create/217?src=mail"), "query string");
		ok(FORM_PATH_RE.test("/x/servicedesk/customer/portal/41/create/217"), "context path prefix");
	},

	"paths: a ticket page is not a form"() {
		ok(!FORM_PATH_RE.test("/servicedesk/customer/portal/41/SD-1234"), "ticket page must not match");
		ok(!FORM_PATH_RE.test("/servicedesk/customer/portal/41/create/"), "a missing type ID must not match");
		ok(!FORM_PATH_RE.test("/servicedesk/customer/portal/abc/create/217"), "a non-numeric portal must not match");
	},

	"paths: a ticket page is matched and its key captured"() {
		const m = "/servicedesk/customer/portal/41/SD-1234".match(TICKET_PATH_RE);
		deepEq([m?.[1], m?.[2]], ["41", "SD-1234"], "portal ID and ticket key");
		ok(TICKET_PATH_RE.test("/servicedesk/customer/portal/41/SD-1234/"), "a trailing slash is allowed");
	},

	"paths: a lowercase or malformed key is not a ticket"() {
		ok(!TICKET_PATH_RE.test("/servicedesk/customer/portal/41/sd-1234"), "lowercase key");
		ok(!TICKET_PATH_RE.test("/servicedesk/customer/portal/41/SD1234"), "key without a dash");
		ok(!TICKET_PATH_RE.test("/servicedesk/customer/portal/41/create/217"), "a create form is not a ticket");
	},

	"selector: the send control keeps the cancel button beside it out"() {
		// A fake DOM matches by registration rather than by CSS, so whether this string finds
		// anything in the portal's markup is out of reach here. What is pinned is the rule the
		// clauses encode.
		const clauses = SUBMIT_CONTROL_SELECTOR.split(",").map(c => c.trim());
		ok(clauses.includes('button[type="submit"]'), "a declared submit button is a send control");
		ok(clauses.includes('input[type="submit"]'), "and so is a submit input");

		const portal = clauses.find(c => c.includes("buttons-container"));
		ok(!!portal, "the portal's own send button has a clause of its own");
		ok(portal?.includes(".aui-button-primary"),
			"told from the cancel button in the same container by the primary modifier");
	},

	"settings: the template is handed out as copies"() {
		const a = defaultSettings();
		const b = defaultSettings();
		ok(a.portals !== b.portals, "each call gets its own portals array");
		ok(a.fields !== b.fields, "each call gets its own fields object");
		a.portals.push("999");
		a.fields.hostname = false;
		deepEq(b.portals, DEFAULT_PORTAL_IDS, "editing one copy must not reach the seed");
		eq(defaultSettings().fields.hostname, true, "editing one copy must not reach the template");
	}
};

await run(import.meta, cases);
