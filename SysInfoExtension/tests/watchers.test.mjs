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
const { SUBMIT_CONTROL_SELECTOR, EDITOR_SELECTOR, URL_TICK_MS, TITLE_SELECTOR } =
	await import("../content/lib/constants.js");
const { loadSettings } = await import("../content/lib/settings.js");

const ready = loadSettings();
env.clock.advance(10);
await ready;

setupSubmitWatcher();
setupUrlWatcher();

/**
 * Puts the request form into the page, identified the way the watcher identifies it: by the
 * description editor it carries.
 *
 * The page holds one editor, so this is the form every send control has to sit inside.
 * @returns The form element.
 */
function mountRequestForm() {
	const form = new FakeEl("form", ["form"]);
	env.setNode(EDITOR_SELECTOR, form.appendChild(new FakeEl("p")));
	return form;
}

/** Returns a form of the page that is not the request form — a login or a search box. */
function otherForm() {
	return new FakeEl("form", ["form"]);
}

const requestForm = mountRequestForm();

/**
 * Returns a node registered under the send-control selector, placed inside `parent`.
 * @param parent - The form the control belongs to; the request form unless a case says otherwise.
 */
function sendButton(parent = requestForm) {
	return parent.appendChild(new FakeEl("button", [SUBMIT_CONTROL_SELECTOR]));
}

/** Returns the entries currently in storage. */
const entries = () => env.store[HISTORY_KEY] || [];

/**
 * Puts the ticket heading into the page, so that finalizing needs no wait of its own and the
 * clock can be held short enough to tell one hook apart from the other.
 */
function showHeading(text) {
	const h = new FakeEl("span");
	h.textContent = text;
	env.setNode(TITLE_SELECTOR, h);
}

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
		env.dispatch("submit", { target: requestForm, submitter: sendButton() });
		ok(await landed(), "the entry is saved");
	},

	"submit event: a control that does not send is ignored": async () => {
		history.markPendingInsertion("3", "27");
		const cancel = requestForm.appendChild(new FakeEl("button"));
		env.dispatch("submit", { target: requestForm, submitter: cancel });
		ok(!(await landed()), "cancelling raises the event too and must not count as sending");
	},

	"submit event: an event naming no control is accepted": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: requestForm });
		ok(await landed(), "a form sent from script names no submitter");
	},

	// Named no control, so the form it was raised on is the only thing telling it apart from a
	// send: pressing Enter in a login field raises exactly this event.
	"submit event: another form of the page is ignored": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: otherForm() });
		ok(!(await landed()), "the script runs on every page of the host, login form included");
	},

	"submit event: a submission the portal stops is still recorded": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: requestForm, submitter: sendButton() }, { stopped: true });
		ok(await landed(), "the capture phase sees an event the page never lets reach the document");
	},

	"click: a node inside the send control counts": async () => {
		history.markPendingInsertion("3", "27");
		const label = sendButton().appendChild(new FakeEl("span"));
		env.dispatch("click", { target: label });
		ok(await landed(), "the event reaches the label far more often than the button");
	},

	"click: a node outside any send control does not": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("click", { target: new FakeEl("a") });
		ok(!(await landed()), "an unrelated click must not mark the form");
	},

	"click: a send control of another form does not": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("click", { target: sendButton(otherForm()) });
		ok(!(await landed()), "a control matching the selector still has to sit in the request form");
	},

	"click: something that is not an element does not": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("click", { target: { nodeType: 3 } });
		ok(!(await landed()), "a text node carries no closest()");
	},

	"click: a click the portal stops is still recorded": async () => {
		history.markPendingInsertion("3", "27");
		const label = sendButton().appendChild(new FakeEl("span"));
		env.dispatch("click", { target: label }, { stopped: true });
		ok(await landed(), "the click hook is in the capture phase too");
	},

	"url watcher: the poll turns a navigation into an entry": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: requestForm, submitter: sendButton() });
		env.navigate(TICKET);
		// No direct call to finalize here: the poll is what has to notice the move.
		await env.clock.runFor(URL_TICK_MS + 6000);
		eq(entries().length, 1, "the poll finalized the record");
	},

	"url watcher: staying on the same path finalizes nothing": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: requestForm, submitter: sendButton() });
		await env.clock.runFor(URL_TICK_MS * 10);
		eq(entries().length, 0, "no navigation, no entry");
	},

	"url watcher: popstate reports the move without waiting for the poll": async () => {
		history.markPendingInsertion("3", "27");
		env.dispatch("submit", { target: requestForm, submitter: sendButton() });
		showHeading("Printer");
		env.navigate(TICKET);
		env.dispatchWindow("popstate", {});
		// Held under one tick of the poll, which is the only way the entry can be credited to
		// popstate rather than to the poll that would have followed it.
		await env.clock.runFor(URL_TICK_MS - 1);
		eq(entries().length, 1, "back and forward are reported before the next poll");
	}
};

await run(import.meta, cases, { isolate: true });
