// Mutation check: breaks one behaviour at a time and asserts that the case meant to cover it
// fails. A mutation no case notices marks a test that proves nothing.

import { execFileSync } from "node:child_process";
import { readFileSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = fileURLToPath(new URL(".", import.meta.url));
const EXT = join(HERE, "..");
const MARK = "RESULT:";

const MUTATIONS = [
	// The value of EDITOR_WAIT_MS is not mutated: the cases take their timings from the constant,
	// so shortening it moves the test with it. Pinning the number would only catch a deliberate
	// retuning. What is mutated instead is the rule that the wait belongs to one form.
	{
		what: "the wait is not restarted on a new form",
		file: "content/lib/insertion.js",
		from: "deadline = Date.now() + EDITOR_WAIT_MS;",
		to: "deadline = deadline || Date.now() + EDITOR_WAIT_MS;",
		test: "insertion.test.mjs",
		case: "navigation: moving to another form restarts the wait"
	},
	{
		what: "a stale report is never withdrawn",
		file: "content/lib/insertion.js",
		from: "dismissWaitReport();",
		to: "void 0;",
		test: "insertion.test.mjs",
		case: "markup: a report is withdrawn when the editor turns up after all"
	},
	{
		what: "the editor root is not probed separately",
		file: "content/lib/insertion.js",
		from: "const due = rootSeenAt ? Math.min(deadline, rootSeenAt + EDITOR_ROOT_GRACE_MS) : deadline;",
		to: "const due = deadline;",
		test: "insertion.test.mjs",
		case: "markup: a root without its paragraph is answered on the grace period"
	},
	{
		what: "the whitelist is read without gating the watchers",
		file: "content/lib/portals.js",
		from: "watchStorage();",
		to: "void 0;",
		test: "portals.test.mjs",
		case: "edit: reaches an open page without a reload"
	},
	{
		what: "an emptied list is accepted as a list",
		file: "content/lib/portals.js",
		from: "return Array.isArray(value) && value.length > 0",
		to: "return Array.isArray(value) && value.length >= 0",
		test: "portals.test.mjs",
		case: "stored: an emptied list falls back to the default"
	},
	{
		what: "the legacy boot-time key stands in for an empty one",
		file: "shared/sysinfo_payload.js",
		from: "raw[BOOT_TIME_KEY] ?? raw[LEGACY_BOOT_TIME_KEY]",
		to: "raw[BOOT_TIME_KEY] || raw[LEGACY_BOOT_TIME_KEY]",
		test: "pure.test.mjs",
		case: "payload: an empty new key is not read as an absent one"
	},
	{
		what: "the whitelist is not re-checked before saving",
		file: "content/lib/history.js",
		from: "if (!isTicketAllowed(pendingInsertion.formPath)) {",
		to: "if (false) {",
		test: "history.test.mjs",
		case: "drops the record when the pair left the whitelist meanwhile"
	},
	{
		what: "the history is not capped",
		file: "content/lib/history.js",
		from: ".slice(0, 20);",
		to: ".slice(0, 100);",
		test: "history.test.mjs",
		case: "keeps the newest twenty entries"
	},
	{
		what: "a submit event naming no control is rejected",
		file: "content/lib/submit_watcher.js",
		from: "if (event.submitter && !isSubmitControl(event.submitter)) return;",
		to: "if (!isSubmitControl(event.submitter)) return;",
		test: "watchers.test.mjs",
		case: "submit event: an event naming no control is accepted"
	},
	{
		what: "the sender of a message is not checked",
		file: "background/service_worker.js",
		from: "if (sender.id && sender.id !== browser.runtime.id) return;",
		to: "if (false) return;",
		test: "background.test.mjs",
		case: "guard: a message from another extension is dropped"
	},
	{
		what: "the renamed field flag overwrites the current one",
		file: "popup/lib/storage.js",
		from: "if (fields.uptime !== undefined && fields.lastBootTime === undefined) {",
		to: "if (fields.uptime !== undefined) {",
		test: "popup_settings.test.mjs",
		case: "fields: the old name does not overwrite the current one"
	},
	{
		what: "an error toast is given a timer",
		file: "content/lib/toast.js",
		from: "if (duration > 0) setTimeout(() => removeToast(toast), duration);",
		to: "setTimeout(() => removeToast(toast), duration);",
		test: "toast.test.mjs",
		case: "error: no timer, so it stands until it is dismissed"
	},
	{
		what: "the block is written to a form outside the whitelist",
		file: "content/lib/insertion.js",
		from: "if (!formMatch || !isTicketAllowed(path)) return;",
		to: "if (!formMatch) return;",
		test: "insertion.test.mjs",
		case: "silent: a form outside the whitelist gets no block either"
	},
	{
		what: "a storage that throws leaves the whitelist promise pending",
		file: "content/lib/portals.js",
		from: 'swarn("portals: storage exception", e && e.message);',
		to: "return;",
		test: "portals.test.mjs",
		case: "failure: a storage that throws still settles on the defaults"
	},
	{
		what: "a sendMessage that throws is not retried",
		file: "content/lib/sysinfo.js",
		from: "retry(e && e.message);",
		to: "void 0;",
		test: "sysinfo.test.mjs",
		case: "fetch: a sendMessage that throws is retried like a refusal"
	},
	{
		what: "only the poll reports a navigation",
		file: "content/lib/url_watcher.js",
		from: 'window.addEventListener("popstate", checkUrlChange);',
		to: "void 0;",
		test: "watchers.test.mjs",
		case: "url watcher: popstate reports the move without waiting for the poll"
	},

	// The popup's view layer.
	{
		what: "a stored url reaches the href whatever its scheme",
		file: "popup/lib/tab_history.js",
		from: String.raw`const safeUrl = /^https?:\/\//i.test(it.url) ? it.url : "#";`,
		to: "const safeUrl = it.url;",
		test: "popup_ui.test.mjs",
		case: "history: an entry becomes a link that cannot run script"
	},
	{
		what: "the history tab lists every entry it holds",
		file: "popup/lib/tab_history.js",
		from: "items.slice(0, 10)",
		to: "items.slice(0, 100)",
		test: "popup_ui.test.mjs",
		case: "history: at most ten entries are listed"
	},
	{
		what: "an ID that no URL could match is accepted",
		file: "popup/lib/tab_settings.js",
		from: String.raw`if (!trimmed || !/^\d+$/.test(trimmed)) return;`,
		to: "if (!trimmed) return;",
		test: "popup_ui.test.mjs",
		case: "settings: an ID that is not a number is dropped without a word"
	},
	{
		what: "an ID already listed is added a second time",
		file: "popup/lib/tab_settings.js",
		from: "if (items.includes(trimmed)) {",
		to: "if (false) {",
		test: "popup_ui.test.mjs",
		case: "settings: an ID already listed is announced and changes nothing"
	},
	{
		what: "the chip removed is not the one clicked",
		file: "popup/lib/tab_settings.js",
		from: "items.splice(idx, 1);",
		to: "items.splice(0, 1);",
		test: "popup_ui.test.mjs",
		case: "settings: the × chip takes its own ID out"
	},
	{
		what: "the relative count is substituted as a number",
		file: "popup/lib/dom.js",
		from: 't("timeMinutesAgo", [String(Math.floor(diff / 60))])',
		to: 't("timeMinutesAgo", [Math.floor(diff / 60)])',
		test: "popup_ui.test.mjs",
		case: "time: the count reaches the catalogue as a string"
	},

	// The packaging guards, whose subject is a data file rather than a statement.
	{
		what: "a source is changed without rebuilding the bundle",
		file: "content/lib/compat.js",
		from: 'const SYSINFO_LOG_TAG = "[SysInfo]";',
		to: 'const SYSINFO_LOG_TAG = "[SysInfo!]";',
		test: "packaging.test.mjs",
		case: "bundle: the shipped file is what the sources build to"
	},
	{
		what: "the two manifests drift apart",
		file: "firefox_manifest.json",
		from: '"default_locale": "en"',
		to: '"default_locale": "ru"',
		test: "packaging.test.mjs",
		case: "manifests: the two agree on everything but how each browser loads them"
	},
	{
		what: "a translation loses a key the default locale carries",
		file: "_locales/ru/messages.json",
		from: '"timeMinutesAgo"',
		to: '"timeMinutesAgoRenamed"',
		test: "i18n.test.mjs",
		case: "locales: every catalogue carries the key set of the default one"
	}
];

let survived = 0;
for (const m of MUTATIONS) {
	const path = join(EXT, m.file);
	const original = readFileSync(path, "utf8");
	if (!original.includes(m.from)) {
		survived++;
		console.log(`SKIP      ${m.what}\n          anchor no longer in ${m.file}`);
		continue;
	}

	writeFileSync(path, original.replace(m.from, m.to), "utf8");
	const failures = runCase(m.test, m.case);
	writeFileSync(path, original, "utf8");

	if (failures === null) console.log(`caught    ${m.what}\n          ${m.case} — crashed`);
	else if (failures.length) console.log(`caught    ${m.what}\n          ${m.case} — ${failures[0]}`);
	else { survived++; console.log(`SURVIVED  ${m.what}\n          ${m.case} did not notice`); }
}

console.log(`\n${MUTATIONS.length - survived}/${MUTATIONS.length} mutations caught`);
process.exit(survived ? 1 : 0);

/** Runs one case of one test file and returns its failures, or null when the process died. */
function runCase(file, name) {
	try {
		const out = execFileSync(process.execPath, [join(HERE, file), name], { encoding: "utf8" });
		const line = out.split("\n").find(l => l.startsWith(MARK));
		if (!line) return null;
		return JSON.parse(line.slice(MARK.length))[0][1];
	} catch {
		return null;
	}
}
