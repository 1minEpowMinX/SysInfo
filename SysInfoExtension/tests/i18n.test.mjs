// The message catalogues: that the locales carry one key set, that a source naming a key finds
// it there, and that the substitutions line up.
//
// A missing key is otherwise silent — `t` answers with the key itself, so the interface shows
// the string `timeMinutesAgo` and nothing fails.
//
// Only a key spelled as a literal is checked: `t("key")` and any `somethingKey: "key"` property.
// One reaching `t` through a variable is invisible here, which is why the check never reports an
// unreferenced catalogue entry — it could not tell a dead one from an indirect one.

import { run } from "./runner.mjs";
import { ok, deepEq } from "./assert.mjs";
import { readdirSync, readFileSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";

const EXT = fileURLToPath(new URL("..", import.meta.url));
const LOCALES = join(EXT, "_locales");

// The directories a message key may be named in. The bundle is left out: it is the build's copy
// of content/, and a key missing from it is a stale bundle, which packaging.test.mjs reports.
const SOURCE_DIRS = ["background", "content", "popup", "shared"];

const DEFAULT_LOCALE = JSON.parse(readFileSync(join(EXT, "chromium_manifest.json"), "utf8")).default_locale;

const locales = readdirSync(LOCALES, { withFileTypes: true })
	.filter(e => e.isDirectory())
	.map(e => e.name);

const catalogues = Object.fromEntries(locales.map(l =>
	[l, JSON.parse(readFileSync(join(LOCALES, l, "messages.json"), "utf8"))]));

/** Returns the keys named by a literal in the extension's own sources. */
function keysNamedInSources() {
	const found = new Set();
	for (const dir of SOURCE_DIRS) {
		const entries = readdirSync(join(EXT, dir), { withFileTypes: true, recursive: true });
		for (const e of entries) {
			if (!e.isFile() || !e.name.endsWith(".js") || e.name.endsWith(".bundle.js")) continue;
			const text = readFileSync(join(e.parentPath, e.name), "utf8");
			for (const m of text.matchAll(/\bt\(\s*["']([A-Za-z0-9_]+)["']/g)) found.add(m[1]);
			for (const m of text.matchAll(/\b\w+Key:\s*["']([A-Za-z0-9_]+)["']/g)) found.add(m[1]);
		}
	}
	return [...found];
}

/** Returns the keys named as `__MSG_key__` anywhere in `text`. */
function keysNamedInManifest(text) {
	return [...text.matchAll(/__MSG_([A-Za-z0-9_]+)__/g)].map(m => m[1]);
}

const cases = {
	"locales: the default locale the manifests name has a catalogue"() {
		ok(locales.includes(DEFAULT_LOCALE), `${DEFAULT_LOCALE} is missing from _locales`);
	},

	"locales: every catalogue carries the key set of the default one"() {
		const expected = new Set(Object.keys(catalogues[DEFAULT_LOCALE]));
		for (const l of locales) {
			if (l === DEFAULT_LOCALE) continue;
			const got = new Set(Object.keys(catalogues[l]));
			// Reported as the difference rather than as the two sets: a catalogue runs to dozens
			// of keys, and a full dump buries the one that moved.
			const missing = [...expected].filter(k => !got.has(k));
			const extra = [...got].filter(k => !expected.has(k));
			ok(missing.length === 0, `${l} is missing ${missing.join(", ")}`);
			ok(extra.length === 0, `${l} carries ${extra.join(", ")}, which ${DEFAULT_LOCALE} does not`);
		}
	},

	"locales: no message is left empty"() {
		for (const l of locales) {
			for (const [key, entry] of Object.entries(catalogues[l])) {
				ok(typeof entry.message === "string" && entry.message.trim() !== "",
					`${l}/${key} carries no message`);
			}
		}
	},

	"placeholders: every token a message uses is declared"() {
		for (const l of locales) {
			for (const [key, entry] of Object.entries(catalogues[l])) {
				// getMessage matches a placeholder name without regard to case, so the token and
				// the declaration are compared the same way.
				const declared = Object.keys(entry.placeholders || {}).map(p => p.toLowerCase());
				for (const m of String(entry.message).matchAll(/\$([A-Za-z0-9_]+)\$/g)) {
					ok(declared.includes(m[1].toLowerCase()),
						`${l}/${key} substitutes $${m[1]}$ without declaring it`);
				}
			}
		}
	},

	"placeholders: a key takes the same substitutions in every locale"() {
		for (const key of Object.keys(catalogues[DEFAULT_LOCALE])) {
			const expected = Object.keys(catalogues[DEFAULT_LOCALE][key].placeholders || {}).sort();
			for (const l of locales) {
				if (l === DEFAULT_LOCALE) continue;
				deepEq(Object.keys(catalogues[l][key]?.placeholders || {}).sort(), expected, `${l}/${key}`);
			}
		}
	},

	"keys: every key a source names is in the catalogue"() {
		const catalogue = catalogues[DEFAULT_LOCALE];
		for (const key of keysNamedInSources()) {
			ok(key in catalogue, `${key} is named in the sources but missing from ${DEFAULT_LOCALE}`);
		}
	},

	"keys: every key a manifest names is in the catalogue"() {
		const catalogue = catalogues[DEFAULT_LOCALE];
		for (const file of ["chromium_manifest.json", "firefox_manifest.json"]) {
			for (const key of keysNamedInManifest(readFileSync(join(EXT, file), "utf8"))) {
				ok(key in catalogue, `${file} names ${key}, which is missing from ${DEFAULT_LOCALE}`);
			}
		}
	}
};

await run(import.meta, cases);
