// Text building for the inserted block and the retry policy of the agent fetch.
//
// A retry chain outlives the call that started it, so each case gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv } from "./harness.mjs";
import { ok, eq, deepEq, matches } from "./assert.mjs";

// The i18n stub answers with the key, so a label appears in the output under its own key name.
let respond = null;
const env = installEnv({ pathname: "/", respond: (msg, b) => respond(msg, b) });

const { buildSysInfoLines, makeDivider, alreadyInserted, requestSysInfo } =
	await import("../content/lib/sysinfo.js");
const { SYSINFO_REQUEST_RETRIES, SYSINFO_REQUEST_RETRY_MS } =
	await import("../content/lib/constants.js");

const FULL = { hostname: "PC-01", username: "ivanov", ip: "10.0.0.5", lastBootTime: "2026-08-14 09:00" };

/** Drives one requestSysInfo call to its end and returns what the callback received. */
function fetchOnce(waitMs = 60000) {
	let got = "pending";
	requestSysInfo((data) => { got = data; });
	env.clock.advance(waitMs);
	return got;
}

const cases = {
	"lines: two fields per line, four fields over two lines"() {
		const lines = buildSysInfoLines(FULL);
		eq(lines.length, 2, "line count");
		matches(lines[0], /sysinfoHostname: PC-01, sysinfoUsername: ivanov/, "first line");
		matches(lines[1], /sysinfoIP: 10\.0\.0\.5, sysinfoLastBootTime: 2026-08-14 09:00/, "second line");
	},

	"lines: an empty value is spelled by its own fallback"() {
		const lines = buildSysInfoLines({ ...FULL, hostname: "", ip: "" });
		matches(lines[0], /sysinfoHostname: sysinfoUnavailable/, "an empty hostname reads as unavailable");
		matches(lines[1], /sysinfoIP: sysinfoNoIp/, "an empty ip has a fallback of its own");
	},

	"lines: an absent key cannot reach the ticket as the word undefined"() {
		const lines = buildSysInfoLines({});
		ok(!lines.join(" ").includes("undefined"), `no undefined in ${JSON.stringify(lines)}`);
	},

	"divider: spans 45% of the longest line"() {
		const lines = ["1234567890", "12345"];
		eq(makeDivider(lines).length, 4, "floor(10 * 0.45)");
		eq(makeDivider(lines, "=", 1).length, 10, "a full-width divider");
		eq(makeDivider(["abc"], "-", 0.1).length, 0, "a run rounded down to nothing");
	},

	"alreadyInserted: reads value when there is one and innerText otherwise"() {
		eq(alreadyInserted({ value: "text ─── more" }, "───"), true, "found in value");
		eq(alreadyInserted({ value: "" }, "───"), false, "an empty value holds nothing");
		eq(alreadyInserted({ innerText: "a ─── b" }, "───"), true, "found in innerText");
		eq(alreadyInserted({}, "───"), false, "an editor with neither holds nothing");
	},

	"fetch: a successful reply arrives normalized"() {
		respond = () => ({ success: true, data: { hostname: "PC-01", uptime: "T" } });
		const got = fetchOnce();
		deepEq(got, { hostname: "PC-01", username: undefined, ip: undefined, lastBootTime: "T" },
			"the wire name is mapped before the caller sees it");
	},

	"fetch: a success carrying no payload is answered with null"() {
		respond = () => ({ success: true });
		eq(fetchOnce(), null, "a success without data");
	},

	"fetch: a refusing agent is retried and then given up on"() {
		respond = () => ({ success: false, error: "refused" });
		const before = env.messages.length;
		const started = env.clock.now;
		const got = fetchOnce();
		eq(got, null, "callback receives null");
		eq(env.messages.length - before, SYSINFO_REQUEST_RETRIES + 1, "one attempt plus every retry");
		ok(env.clock.now - started >= SYSINFO_REQUEST_RETRIES * SYSINFO_REQUEST_RETRY_MS,
			"the retries are spaced by SYSINFO_REQUEST_RETRY_MS");
	},

	"fetch: a messaging error is retried like a refusal"() {
		let n = 0;
		respond = (_msg, b) => {
			n++;
			if (n < 3) { b.runtime.lastError = { message: "port closed" }; return undefined; }
			return { success: true, data: { hostname: "PC-01" } };
		};
		const before = env.messages.length;
		const got = fetchOnce();
		eq(got?.hostname, "PC-01", "the attempt after the errors is used");
		eq(env.messages.length - before, 3, "two failures then a success");
	},

	"fetch: a sendMessage that throws is retried like a refusal"() {
		// A worker torn down mid-call throws out of sendMessage rather than answering, which is
		// the one failure that reaches the caller outside the reply callback.
		respond = () => ({ success: true, data: { hostname: "PC-01" } });
		const real = browser.runtime.sendMessage;
		let thrown = 0;
		browser.runtime.sendMessage = (msg, cb) => {
			if (thrown < 2) { thrown++; throw new Error("extension context invalidated"); }
			return real(msg, cb);
		};

		const got = fetchOnce();
		browser.runtime.sendMessage = real;

		eq(thrown, 2, "both throwing attempts were made");
		eq(got?.hostname, "PC-01", "the attempt after the throws is used");
	}
};

await run(import.meta, cases, { isolate: true });
