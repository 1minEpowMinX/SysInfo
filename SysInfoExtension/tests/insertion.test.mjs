// The editor watcher: what it writes, what it reports, and what it stays quiet about.
//
// The watcher installs interval and observer hooks that live for the process, so each case gets
// a process of its own.

import { run } from "./runner.mjs";
import { installEnv, FakeEl } from "./harness.mjs";
import { ok, eq, matches } from "./assert.mjs";
import { STORAGE_KEY } from "../shared/constants.js";

const FORM = "/servicedesk/customer/portal/3/create/27";
const FORM_B = "/servicedesk/customer/portal/3/create/28";
const OUTSIDE = "/servicedesk/customer/portal/3/create/99";
const TICKET = "/servicedesk/customer/portal/3/SD-1234";
const WHITELIST = { [STORAGE_KEY]: { portals: ["3"], types: ["27", "28"] } };
const AGENT = { hostname: "PC-01", username: "ivanov", ip: "10.0.0.5", lastBootTime: "2026-08-14 09:00" };

const ROOT = "#ak-editor-textarea";
const PARA = "#ak-editor-textarea > p";

const CASE = process.argv[2] || "";
const env = installEnv({
	pathname: CASE.startsWith("silent: a form outside") ? OUTSIDE
		: CASE.startsWith("silent: a ticket") ? TICKET
			: FORM,
	storage: WHITELIST,
	agent: CASE.startsWith("failure: the agent") ? null : AGENT
});

const { startInsertion } = await import("../content/lib/insertion.js");
const { EDITOR_WAIT_MS, EDITOR_ROOT_GRACE_MS } = await import("../content/lib/constants.js");
const { loadPortals } = await import("../content/lib/portals.js");

const ready = loadPortals();
env.clock.advance(10);
await ready;

/** Puts an editor root, and optionally its paragraph, into the page. */
function mountEditor({ paragraph = true } = {}) {
	const root = new FakeEl("div");
	env.setNode(ROOT, root);
	if (!paragraph) return null;
	const para = new FakeEl("p");
	env.setNode(PARA, para);
	return para;
}

const cases = {
	"insert: the block reaches the editor and one success is shown"() {
		const para = mountEditor();
		startInsertion();
		env.clock.advance(5000);

		matches(para.innerText, /sysinfoHostname: PC-01/, "the hostname line is written");
		matches(para.innerText, /sysinfoLastBootTime: 2026-08-14 09:00/, "the boot time line is written");
		matches(para.innerText, /─/, "the divider is written");
		matches(para.innerText, /​/, "the blank lines carry a zero-width space so Jira keeps them");

		const t = env.toasts();
		eq(t.length, 1, "one toast");
		eq(t[0]?.kind, "success", "kind");
	},

	"insert: an editor already carrying the block is left alone"() {
		const para = mountEditor();
		para.innerText = "already here ──────────────────────────────────────";
		startInsertion();
		env.clock.advance(5000);
		matches(para.innerText, /^already here/, "the editor is not written to");
		eq(env.toasts().length, 0, "and nothing is announced");
	},

	"failure: the agent never answered and the editor is up"() {
		mountEditor();
		startInsertion();
		env.clock.advance(20000);
		const t = env.toasts();
		eq(t.length, 1, "one toast");
		eq(t[0]?.kind, "error", "kind");
		matches(t[0]?.text, /toastAgentUnreachable/, "the message names the agent");
		env.clock.advance(300000);
		eq(env.toasts().length, 1, "an error carries no timer and no second report follows");
	},

	"slow: a form that renders late is not reported on"() {
		startInsertion();
		env.clock.advance(EDITOR_WAIT_MS - 10000);
		eq(env.toasts().length, 0, "nothing while the form is still coming up");
		mountEditor();
		env.clock.advance(5000);
		const t = env.toasts();
		eq(t.length, 1, "one toast");
		eq(t[0]?.kind, "success", "and it is the success, not a report");
	},

	"markup: a root without its paragraph is answered on the grace period"() {
		const started = env.clock.now;
		mountEditor({ paragraph: false });
		startInsertion();
		env.clock.advance(EDITOR_ROOT_GRACE_MS - 1000);
		eq(env.toasts().length, 0, "nothing inside the grace period");
		env.clock.advance(4000);
		const t = env.toasts();
		eq(t.length, 1, "reported once the grace period is over");
		matches(t[0]?.text, /toastEditorMissing/, "the message names the editor");
		ok(env.clock.now - started < EDITOR_WAIT_MS, "and well before the full wait would have been up");
	},

	"markup: neither element ever appears, so the full wait applies"() {
		startInsertion();
		env.clock.advance(EDITOR_WAIT_MS - 5000);
		eq(env.toasts().length, 0, "nothing before the wait is up");
		env.clock.advance(10000);
		eq(env.toasts().length, 1, "one report after it");
		env.clock.advance(600000);
		eq(env.toasts().length, 1, "and no more for the same form");
	},

	"markup: a report is withdrawn when the editor turns up after all"() {
		mountEditor({ paragraph: false });
		startInsertion();
		env.clock.advance(EDITOR_ROOT_GRACE_MS + 2000);
		eq(env.toasts()[0]?.kind, "error", "the wait is reported first");
		mountEditor();
		env.clock.advance(3000);
		const t = env.toasts();
		eq(t.length, 1, "one toast is left");
		eq(t[0]?.kind, "success", "and it is the success");
	},

	"navigation: moving to another form restarts the wait"() {
		startInsertion();
		env.clock.advance(EDITOR_WAIT_MS - 4000);
		env.navigate(FORM_B);
		env.clock.advance(5000);
		eq(env.toasts().length, 0, "the wait of the form left behind does not carry over");
		env.clock.advance(EDITOR_WAIT_MS);
		eq(env.toasts().length, 1, "the new form is reported on its own wait");
	},

	"silent: a form outside the whitelist is never reported on"() {
		startInsertion();
		env.clock.advance(600000);
		eq(env.toasts().length, 0, "no report");
	},

	"silent: a form outside the whitelist gets no block either"() {
		// The watcher hands every editor it finds to the insertion, whatever the path, so the
		// whitelist check inside the insertion is the only thing keeping the block out of a form
		// the user excluded.
		const para = mountEditor();
		para.innerText = "";
		startInsertion();
		env.clock.advance(600000);
		eq(para.innerText, "", "the editor is left as it was");
		eq(env.toasts().length, 0, "and nothing is announced");
	},

	"silent: a ticket page is never reported on"() {
		startInsertion();
		env.clock.advance(600000);
		eq(env.toasts().length, 0, "no report");
	}
};

await run(import.meta, cases, { isolate: true });
