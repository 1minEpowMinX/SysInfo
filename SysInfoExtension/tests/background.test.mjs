// The service worker's message routing and the shape it answers in.
//
// The worker registers its listener at import, so each case gets a process of its own.

import { run } from "./runner.mjs";
import { installEnv } from "./harness.mjs";
import { ok, eq, deepEq, matches } from "./assert.mjs";

/** Answers a fetch with a body of the given kind. */
const reply = (kind, value) => Promise.resolve({
	json: () => Promise.resolve(value),
	text: () => Promise.resolve(value)
});

const CASE = process.argv[2] || "";
const REQUIRED = ["http://localhost:8734/*", "https://jira.company.local/*"];
const WITHHELD = "https://jira.company.local/*";

let fetchImpl = () => reply("json", {});
const env = installEnv({
	pathname: "/",
	manifest: {
		version: "2.1.0",
		host_permissions: CASE.startsWith("permissions: a manifest naming no origin") ? [] : REQUIRED
	},
	fetch: (...a) => fetchImpl(...a)
});

// The permission report runs as the worker is imported, so what the browser answers about each
// origin is settled before that.
if (CASE.startsWith("permissions: an origin the browser withholds")) {
	browser.permissions.contains = async ({ origins }) => origins[0] !== WITHHELD;
} else if (CASE.startsWith("permissions: a check the browser refuses")) {
	browser.permissions.contains = async () => { throw new Error("permissions unavailable"); };
}

await import("../background/service_worker.js");
const listen = env.messageListener();
await env.clock.flush();

const OWN = { id: "sysinfo@company.local" };

/** Sends `msg` to the worker and returns what it answered along with its return value. */
async function ask(msg, sender = OWN) {
	let answer = "no reply";
	const returned = listen(msg, sender, (r) => { answer = r; });
	await env.clock.runFor(50);
	return { answer, returned };
}

const cases = {
	"routing: getSystemInfo is answered with the parsed body": async () => {
		fetchImpl = () => reply("json", { hostname: "PC-01" });
		const { answer, returned } = await ask({ action: "getSystemInfo" });
		deepEq(answer, { success: true, data: { hostname: "PC-01" } }, "reply shape");
		eq(returned, true, "the listener keeps the channel open for an async reply");
		matches(env.fetches[0]?.url, /^http:\/\/localhost:8734\/systeminfo$/, "requested URL");
	},

	"routing: getStatus is answered as text": async () => {
		fetchImpl = () => reply("text", "OK 2.1.0");
		const { answer } = await ask({ action: "getStatus" });
		deepEq(answer, { success: true, text: "OK 2.1.0" }, "a text route answers through text, not data");
		matches(env.fetches[0]?.url, /\/status$/, "requested URL");
	},

	"routing: getVersion is answered with the parsed body": async () => {
		fetchImpl = () => reply("json", { version: "2.1.0", build: "20260814" });
		const { answer } = await ask({ action: "getVersion" });
		eq(answer.data?.build, "20260814", "build stamp");
		matches(env.fetches[0]?.url, /\/version$/, "requested URL");
	},

	"request: every call identifies this extension to the agent": async () => {
		fetchImpl = () => reply("json", {});
		await ask({ action: "getSystemInfo" });
		eq(env.fetches[0]?.init?.headers?.["X-Sysinfo-Client"], "sysinfo@company.local", "client header");
	},

	"failure: an unreachable agent answers rather than rejecting": async () => {
		fetchImpl = () => Promise.reject(new Error("Failed to fetch"));
		const { answer } = await ask({ action: "getSystemInfo" });
		deepEq(answer, { success: false, error: "Failed to fetch" },
			"a caller waiting on the reply always settles");
	},

	"failure: a body that will not parse answers as a failure": async () => {
		fetchImpl = () => Promise.resolve({ json: () => Promise.reject(new Error("bad json")) });
		const { answer } = await ask({ action: "getSystemInfo" });
		eq(answer.success, false, "parsing is part of the request");
		eq(answer.error, "bad json", "and its error is carried through");
	},

	"diagPing: answered at once and the channel closed": async () => {
		const { answer, returned } = await ask({ action: "diagPing", source: "content-script-load" });
		deepEq(answer, { ok: true }, "reply");
		eq(returned, false, "nothing is pending, so the channel is not held open");
	},

	"guard: a message from another extension is dropped": async () => {
		fetchImpl = () => reply("json", {});
		const { answer, returned } = await ask({ action: "getSystemInfo" }, { id: "someone-else" });
		eq(answer, "no reply", "no answer is sent");
		eq(returned, undefined, "and the channel is not held open");
		eq(env.fetches.length, 0, "the agent is not called on behalf of a stranger");
	},

	"guard: an unknown action is answered by nobody": async () => {
		const { answer, returned } = await ask({ action: "selfDestruct" });
		eq(answer, "no reply", "no answer");
		eq(returned, undefined, "no open channel");
		ok(env.logs.some(l => l.includes("unknown action")), "and it is logged");
	},

	"guard: a message carrying no action is dropped": async () => {
		const { returned } = await ask({});
		eq(returned, undefined, "nothing to route");
	},

	"guard: a message carrying no sender id is served": async () => {
		fetchImpl = () => reply("json", { hostname: "PC-01" });
		const { answer } = await ask({ action: "getSystemInfo" }, {});
		deepEq(answer, { success: true, data: { hostname: "PC-01" } },
			"an id-less sender is this extension's own popup, not a stranger");
	},

	"permissions: every granted origin is reported"() {
		ok(env.logs.some(l => l.includes("all required origins granted")),
			"the worker reports the outcome of the check");
	},

	"permissions: an origin the browser withholds is named"() {
		const line = env.logs.find(l => l.includes("NOT GRANTED"));
		ok(!!line, "the shortfall is reported");
		ok(line?.includes(WITHHELD), `the origin is named in ${JSON.stringify(line)}`);
	},

	"permissions: a manifest naming no origin says so"() {
		ok(env.logs.some(l => l.includes("declares no host_permissions")),
			"an empty list is reported rather than passed over in silence");
		ok(!env.logs.some(l => l.includes("NOT GRANTED")), "and nothing is called a shortfall");
	},

	"permissions: a check the browser refuses is logged, not thrown"() {
		ok(env.logs.some(l => l.includes("checkHostPermissions failed")), "the failure is reported");
		ok(typeof listen === "function", "and the worker still routes messages");
	}
};

await run(import.meta, cases, { isolate: true });
