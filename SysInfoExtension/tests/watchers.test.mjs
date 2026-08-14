// The two watchers that tell the history when a form was sent and when the page moved.
//
// Both write into the history's module state, so each case gets a process of its own. What they
// did is read back from the entry the journey produces, not from the module's internals.

import { run } from "./runner.mjs";
import { installEnv, FakeEl } from "./harness.mjs";
import { ok, eq } from "./assert.mjs";
import { STORAGE_KEY, HISTORY_KEY } from "../shared/constants.js";

const FORM = "/servicedesk/customer/portal/3/create/27";
const TICKET = "/servicedesk/customer/portal/3/SD-1234";

const env = installEnv({
	pathname: FORM,
	title: "SD-1234 Printer - Jira",
	storage: { [STORAGE_KEY]: { portals: ["3"], types: ["27"] } }
});

const { setupSubmitWatcher } = await import("../content/lib/submit_watcher.js");
const { setupUrlWatcher } = await import("../content/lib/url_watcher.js");
const history = await import("../content/lib/history.js");
const { SUBMIT_CONTROL_SELECTOR, URL_TICK_MS } = await import("../content/lib/constants.js");
const { loadPortals } = await import("../content/lib/portals.js");

const ready = loadPortals();
env.clock.advance(10);
await ready;

setupSubmitWatcher();
setupUrlWatcher();

/** Returns a node registered under the send-control selector, optionally wrapping a child. */
function sendButton() {
	return new FakeEl("button", [SUBMIT_CONTROL_SELECTOR]);
}

/** Returns the entries currently in storage. */
const entries = () => env.store[HISTORY_KEY] || [];

/**
 * Marks an insertion, lets the case raise whatever event it likes, then lands on the ticket and
 * reports whether the journey produced an entry.
 */
async function landed() {
	env.navigate(TICKET);
	history.finalizeHistoryIfCreated();
	await env.clock.runFor(6000);
	return entries().length === 1;
}

const cases = {
	"submit event: a send control marks the form as sent": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { submitter: sendButton() });
		ok(await landed(), "the entry is saved");
	},

	"submit event: a control that does not send is ignored": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { submitter: new FakeEl("button") });
		ok(!(await landed()), "cancelling raises the event too and must not count as sending");
	},

	"submit event: an event naming no control is accepted": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", {});
		ok(await landed(), "a form sent from script names no submitter");
	},

	"click: a node inside the send control counts": async () => {
		history.markPendingInsertion("3", "27");
		const label = new FakeEl("span");
		sendButton().appendChild(label);
		env.dispatch("click", { target: label });
		ok(await landed(), "the event reaches the label far more often than the button");
	},

	"click: a node outside any send control does not": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("click", { target: new FakeEl("a") });
		ok(!(await landed()), "an unrelated click must not mark the form");
	},

	"click: something that is not an element does not": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("click", { target: { nodeType: 3 } });
		ok(!(await landed()), "a text node carries no closest()");
	},

	"url watcher: the poll turns a navigation into an entry": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { submitter: sendButton() });
		env.navigate(TICKET);
		// No direct call to finalize here: the poll is what has to notice the move.
		await env.clock.runFor(URL_TICK_MS + 6000);
		eq(entries().length, 1, "the poll finalized the record");
	},

	"url watcher: staying on the same path finalizes nothing": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { submitter: sendButton() });
		await env.clock.runFor(URL_TICK_MS * 10);
		eq(entries().length, 0, "no navigation, no entry");
	},

	"url watcher: popstate reports the move without waiting for the poll": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { submitter: sendButton() });
		env.navigate(TICKET);
		env.dispatchWindow("popstate", {});
		await env.clock.runFor(6000);
		eq(entries().length, 1, "back and forward are reported at once");
	}
};

await run(import.meta, cases, { isolate: true });
