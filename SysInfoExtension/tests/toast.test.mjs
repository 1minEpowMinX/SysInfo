// The toast's kinds, its lifetime and the handle it hands back.

import { run } from "./runner.mjs";
import { installEnv } from "./harness.mjs";
import { ok, eq, matches } from "./assert.mjs";

const env = installEnv({ pathname: "/" });
const { showToast } = await import("../content/lib/toast.js");

/** Returns the toast elements currently attached. */
const nodes = () => env.document.getElementById("sysinfo-toast-container")?.children || [];

/** Takes every toast away so the next case starts on an empty container. */
function clear() {
	for (const n of [...nodes()]) n.remove();
}

const cases = {
	"success: named by its kind, titled from the catalogue, gone when its time is up"() {
		showToast("body text", 3000);
		let t = env.toasts();
		eq(t.length, 1, "one toast");
		eq(t[0].kind, "success", "kind");
		matches(t[0].text, /bannerTitle/, "title key");
		matches(t[0].text, /body text/, "body");

		env.clock.advance(2999);
		eq(env.toasts().length, 1, "still up a millisecond before its time");
		eq(env.toasts()[0].fading, false, "and not yet fading");

		env.clock.advance(500);
		t = env.toasts();
		eq(t.length, 1, "still attached while it fades");
		eq(t[0].fading, true, "and marked as fading");

		env.clock.advance(700);
		eq(env.toasts().length, 0, "detached once the fade is over");
		clear();
	},

	"error: carries its own title and glyph"() {
		showToast("something failed", 0, "error");
		const t = env.toasts();
		eq(t[0]?.kind, "error", "kind");
		matches(t[0]?.text, /toastErrorTitle/, "the failure title, not the success one");
		clear();
	},

	"error: no timer, so it stands until it is dismissed"() {
		showToast("something failed", 0, "error");
		env.clock.advance(600000);
		eq(env.toasts().length, 1, "ten minutes later it is still there");
		clear();
	},

	"kind: an unknown one is shown as a success"() {
		showToast("body", 1000, "catastrophe");
		const t = env.toasts();
		eq(t[0]?.kind, "success", "falls back to success");
		ok(!/catastrophe/.test(nodes()[0]?.className || ""), "an unknown name must not reach the class list");
		clear();
	},

	"handle: dismisses the toast and is safe to call twice"() {
		const dismiss = showToast("body", 0, "error");
		dismiss();
		env.clock.advance(700);
		eq(env.toasts().length, 0, "dismissed");
		dismiss();
		env.clock.advance(700);
		eq(env.toasts().length, 0, "a second call changes nothing");
		clear();
	},

	"handle: dismissing one leaves the others standing"() {
		const first = showToast("first", 0, "error");
		showToast("second", 0, "error");
		first();
		env.clock.advance(700);
		const t = env.toasts();
		eq(t.length, 1, "one left");
		matches(t[0]?.text, /second/, "the one left is the other");
		clear();
	},

	"close button: takes its own toast away"() {
		showToast("body", 0, "error");
		const toast = nodes()[0];
		const close = toast.children.find(c => c.className.includes("__close"));
		ok(!!close, "the toast carries a dismiss button");
		eq(close?.getAttribute("aria-label"), "Close", "the button is labelled for a screen reader");
		close.listeners.click[0]();
		env.clock.advance(700);
		eq(env.toasts().length, 0, "dismissed by the button");
		clear();
	},

	"stacking: toasts share one container"() {
		showToast("a", 0, "error");
		showToast("b", 0, "error");
		const containers = env.document.documentElement.children.filter(c => c.id === "sysinfo-toast-container");
		eq(containers.length, 1, "the container is created once");
		eq(env.toasts().length, 2, "both toasts are in it");
		clear();
	}
};

await run(import.meta, cases);
