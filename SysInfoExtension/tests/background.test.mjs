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

let fetchImpl = () => reply("json", {});
const env = installEnv({
	pathname: "/",
	manifest: { version: "2.1.0", host_permissions: ["http://localhost:8734/*"] },
	fetch: (...a) => fetchImpl(...a)
});

await import("../background/service_worker.js");
const listen = env.messageListener();

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
	}
};

await run(import.meta, cases, { isolate: true });
