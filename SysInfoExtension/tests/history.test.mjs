// The deferred ticket history: which navigations turn a pending insertion into an entry, and
// which drop it.
//
// The pending record is module state, so each case gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv, FakeEl } from "./harness.mjs";
import { ok, eq } from "./assert.mjs";
import { STORAGE_KEY, HISTORY_KEY } from "../shared/constants.js";

const FORM = "/servicedesk/customer/portal/3/create/27";
const TICKET = "/servicedesk/customer/portal/3/SD-1234";
const OTHER_TICKET = "/servicedesk/customer/portal/9/SD-9999";
const WHITELIST = { portals: ["3"], types: ["27"] };

const CASE = process.argv[2] || "";
const env = installEnv({
	pathname: FORM,
	title: "SD-1234 Broken printer - Jira Service Management",
	storage: {
		[STORAGE_KEY]: WHITELIST,
		[HISTORY_KEY]: CASE.includes("already in the history")
			? [{ id: "SD-1234", title: "seen before" }]
			: CASE.includes("keeps the newest")
				? Array.from({ length: 20 }, (_, i) => ({ id: `OLD-${i}`, title: `old ${i}` }))
				: undefined
	}
});

const history = await import("../content/lib/history.js");
const { SUBMIT_TTL_MS, TITLE_SELECTOR, TITLE_WAIT_MS } = await import("../content/lib/constants.js");
const { loadPortals } = await import("../content/lib/portals.js");

const ready = loadPortals();
env.clock.advance(10);
await ready;

/** Returns the entries currently in storage. */
const entries = () => env.store[HISTORY_KEY] || [];

/** Puts the ticket heading into the page and lets the observer see it. */
function showHeading(text) {
	const h = new FakeEl("span");
	h.textContent = text;
	env.setNode(TITLE_SELECTOR, h);
	env.fireMutation();
}

/** Plays the whole journey: insert into the form, send it, then land on the ticket. */
function insertSubmitAndLand({ land = TICKET } = {}) {
	history.markPendingInsertion("3", "27");
	history.markFormSubmitted();
	env.navigate(land);
	history.finalizeHistoryIfCreated();
}

const cases = {
	"saves an entry for the ticket the sent form created": async () => {
		const started = env.clock.now;
		insertSubmitAndLand();
		showHeading("Broken printer");
		await env.clock.runFor(100);

		eq(entries().length, 1, "one entry");
		const e = entries()[0];
		eq(e.id, "SD-1234", "ticket key");
		eq(e.portalId, "3", "portal ID");
		eq(e.typeId, "27", "ticket type ID");
		eq(e.title, "Broken printer", "title taken from the heading");
		ok(e.url.endsWith(TICKET), "url");
		ok(e.when >= started && e.when <= env.clock.now, "stamped at the moment it was saved");
	},

	"falls back to the page title when no heading appears": async () => {
		insertSubmitAndLand();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries()[0]?.title, "SD-1234 Broken printer",
			"the Jira suffix is stripped from document.title");
	},

	"reads a heading already in the document without waiting": async () => {
		showHeading("Broken printer");
		insertSubmitAndLand();
		await env.clock.runFor(10);
		eq(entries()[0]?.title, "Broken printer", "a heading already there needs no observer");
	},

	"cuts an over-long heading to the stored limit": async () => {
		insertSubmitAndLand();
		showHeading("x".repeat(200));
		await env.clock.runFor(100);
		eq(entries()[0]?.title.length, 120, "the title is capped");
	},

	"falls back to the ticket key when no title can be read": async () => {
		// A page whose title carries the Jira suffix and nothing else leaves an empty title once
		// the suffix is stripped.
		env.document.title = " - Jira Service Management";
		insertSubmitAndLand();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries()[0]?.title, "SD-1234", "the ticket key stands in");
	},

	"drops the record when the form was never sent": async () => {
		history.markPendingInsertion("3", "27");
		env.navigate(TICKET);
		history.finalizeHistoryIfCreated();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "a form left without sending creates nothing");
	},

	"drops the record once the submit window has passed": async () => {
		history.markPendingInsertion("3", "27");
		history.markFormSubmitted();
		await env.clock.runFor(SUBMIT_TTL_MS + 1000);
		env.navigate(TICKET);
		history.finalizeHistoryIfCreated();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "a ticket opened long after the submission is not that submission");
	},

	"ignores a submission raised on another pathname": async () => {
		history.markPendingInsertion("3", "27");
		env.navigate("/servicedesk/customer/portal/3/create/28");
		history.markFormSubmitted();
		env.navigate(TICKET);
		history.finalizeHistoryIfCreated();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "the submission belonged to another form");
	},

	"drops the record when the ticket belongs to another portal": async () => {
		insertSubmitAndLand({ land: OTHER_TICKET });
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "a ticket in another portal is not the one just created");
	},

	"drops the record when the pair left the whitelist meanwhile": async () => {
		history.markPendingInsertion("3", "27");
		history.markFormSubmitted();
		// The popup writes a narrower list while the form is still open; the content script sees
		// it through storage.onChanged.
		browser.storage.local.set({ [STORAGE_KEY]: { portals: ["9"], types: ["27"] } });
		env.navigate(TICKET);
		history.finalizeHistoryIfCreated();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "a pair dropped from the whitelist must not reach the history");
	},

	"does not add a ticket already in the history": async () => {
		insertSubmitAndLand();
		showHeading("Broken printer");
		await env.clock.runFor(100);
		eq(entries().length, 1, "the list is not grown");
		eq(entries()[0].title, "seen before", "and the entry already there is untouched");
	},

	"keeps the newest twenty entries": async () => {
		insertSubmitAndLand();
		showHeading("Broken printer");
		await env.clock.runFor(100);
		eq(entries().length, 20, "the list is capped");
		eq(entries()[0].id, "SD-1234", "the new entry is first");
		eq(entries()[19].id, "OLD-18", "the oldest one falls off");
	},

	"one pending record survives at most one navigation": async () => {
		history.markPendingInsertion("3", "27");
		history.markFormSubmitted();
		env.navigate("/servicedesk/customer/portal/3/user/requests");
		history.finalizeHistoryIfCreated();
		env.navigate(TICKET);
		history.finalizeHistoryIfCreated();
		await env.clock.runFor(TITLE_WAIT_MS + 100);
		eq(entries().length, 0, "a record dropped on one navigation is not revived by the next");
	}
};

await run(import.meta, cases, { isolate: true });
