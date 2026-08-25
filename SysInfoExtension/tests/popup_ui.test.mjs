// The popup's view layer: the relative times in the history, the link a stored entry becomes,
// and the guard on what may be added to a whitelist.
//
// The tabs render from module state and the whitelist editor mutates it in place, so each case
// gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv, FakeEl } from "./harness.mjs";
import { ok, eq, deepEq } from "./assert.mjs";

const env = installEnv({ pathname: "/" });

// state.js looks this node up as it is imported, and every handler ends by rendering into it.
const rootEl = new FakeEl("div");
rootEl.id = "root";
env.document.body.appendChild(rootEl);

const { state } = await import("../popup/lib/state.js");
const { formatWhen } = await import("../popup/lib/dom.js");
const { renderHistoryTab } = await import("../popup/lib/tab_history.js");
const { renderSettingsTab } = await import("../popup/lib/tab_settings.js");
const { statusInfo } = await import("../popup/lib/render.js");
const { AGENT_ORIGIN } = await import("../shared/build_config.js");

const MINUTE = 60 * 1000;
const HOUR = 60 * MINUTE;
const DAY = 24 * HOUR;

/** Returns `el` and everything under it, in document order. */
function all(el) { return [el, ...el.children.flatMap(all)]; }

/** Returns the nodes under `el` carrying `cls`. */
const byClass = (el, cls) => all(el).filter(n => n.classList.contains(cls));

/** Raises a click on `el`, carrying the event object the popup's handlers read. */
const click = (el) => el.listeners.click[0]({ stopPropagation() {}, preventDefault() {} });

/** Returns the chips that add an ID, the portals one first and the types one second. */
const addChips = () => byClass(renderSettingsTab(), "add");

/** Returns a history entry `age` milliseconds old, pointing at `url`. */
const entry = (id, url, age = 0) =>
	({ id, title: "t" + id, url, when: Date.now() - age });

const cases = {
	"time: the age is coarsened from minutes to days"() {
		const now = Date.now();
		eq(formatWhen(0), "", "an entry with no timestamp shows nothing");
		eq(formatWhen(now - 59 * 1000), "timeJustNow", "under a minute");
		eq(formatWhen(now - MINUTE), "timeMinutesAgo", "a minute exactly");
		eq(formatWhen(now - 59 * MINUTE), "timeMinutesAgo", "under an hour");
		eq(formatWhen(now - HOUR), "timeHoursAgo", "an hour exactly");
		eq(formatWhen(now - 23 * HOUR), "timeHoursAgo", "under a day");
		eq(formatWhen(now - DAY), "timeDaysAgo", "a day exactly");
		eq(formatWhen(now - 400 * DAY), "timeDaysAgo", "and everything older");
	},

	"time: the count reaches the catalogue as a string"() {
		formatWhen(Date.now() - 5 * MINUTE);
		const call = env.i18nCalls.find(c => c.key === "timeMinutesAgo");
		deepEq(call?.substitutions, ["5"],
			"getMessage substitutes strings alone; a number would drop the count from the line");
	},

	"history: an entry becomes a link that cannot run script"() {
		state.history = [
			entry("SD-1", "https://jira.company.local/x"),
			entry("SD-2", "javascript:alert(1)")
		];
		const links = byClass(renderHistoryTab(), "hist-item");
		eq(links.length, 2, "both entries are listed");
		eq(links[0].getAttribute("href"), "https://jira.company.local/x", "an http(s) url is kept");
		eq(links[1].getAttribute("href"), "#", "anything else is defused");
		eq(links[1].getAttribute("rel"), "noopener", "and the new tab cannot reach back");
	},

	"history: an empty list shows the empty state"() {
		state.history = [];
		const tab = renderHistoryTab();
		eq(byClass(tab, "hist-item").length, 0, "no links");
		eq(byClass(tab, "hist-empty").length, 1, "an empty-state line instead");
	},

	"history: at most ten entries are listed"() {
		state.history = Array.from({ length: 14 }, (_, i) =>
			entry("SD-" + i, "https://jira.company.local/" + i));
		eq(byClass(renderHistoryTab(), "hist-item").length, 10, "the tab lists ten");
	},

	"settings: an added ID is trimmed and kept"() {
		state.settings.portals = ["3"];
		env.answerPrompt("  27  ");
		click(addChips()[0]);
		deepEq(state.settings.portals, ["3", "27"], "the whitespace is dropped");
	},

	"settings: an ID that is not a number is dropped without a word"() {
		state.settings.portals = ["3"];
		env.answerPrompt("SD-4");
		click(addChips()[0]);
		deepEq(state.settings.portals, ["3"], "a non-numeric entry could never match a URL");
		eq(env.alerts.length, 0, "and it is refused silently");
	},

	"settings: an ID already listed is announced and changes nothing"() {
		state.settings.portals = ["3"];
		env.answerPrompt("3");
		click(addChips()[0]);
		deepEq(state.settings.portals, ["3"], "the list is unchanged");
		deepEq(env.alerts, ["portalExists"], "and the duplicate is announced");
	},

	"settings: a cancelled prompt changes nothing"() {
		state.settings.portals = ["3"];
		// No answer queued, which is what the harness's prompt returns on a cancel.
		click(addChips()[0]);
		deepEq(state.settings.portals, ["3"], "cancelling adds nothing");
	},

	"settings: an over-long entry is cut to twenty characters"() {
		state.settings.types = ["27"];
		env.answerPrompt("1".repeat(25));
		click(addChips()[1]);
		deepEq(state.settings.types, ["27", "1".repeat(20)], "the entry is capped");
	},

	"settings: the × chip takes its own ID out"() {
		state.settings.portals = ["3", "41", "141"];
		const xs = byClass(renderSettingsTab(), "chip-x");
		eq(xs.length, 3 + state.settings.types.length, "one per ID across both lists");
		click(xs[1]);
		deepEq(state.settings.portals, ["3", "141"], "the ID clicked is the one removed");
	},

	"settings: both lists are edited by the same chips"() {
		const chips = addChips();
		eq(chips.length, 2, "one add chip per list");
		ok(byClass(renderSettingsTab(), "id-list").length === 2, "and one list editor per list");
	},

	"status: the active line names the agent by host"() {
		state.status = "active";
		statusInfo();
		const call = env.i18nCalls.find(c => c.key === "statusActiveSub");
		deepEq(call?.substitutions, [new URL(AGENT_ORIGIN).host],
			"the host reaches the catalogue as a substitution, not as translated text");
	}
};

await run(import.meta, cases, { isolate: true });
