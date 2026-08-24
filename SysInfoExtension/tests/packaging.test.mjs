// The packaging artefacts: the manifests the templates render to, and the bundle they ship.
//
// Nothing here is reachable through the extension's own imports — the browser loads these files,
// not the tests — so the checks read them off disk or render them in memory.

import { run } from "./runner.mjs";
import { ok, eq, deepEq } from "./assert.mjs";
import { existsSync, readFileSync } from "node:fs";
import { join } from "node:path";

import { EXT, loadConfig, packageVersion, substitutions } from "../tools/config.mjs";
import { TEMPLATE_DIR, referencedPaths, renderManifest } from "../tools/manifest.mjs";

const version = packageVersion();
const values = substitutions(loadConfig([]).config, version);
const chromium = renderManifest("chromium", values);
const firefox = renderManifest("firefox", values);

/** Reports whether `v` is an object a leaf search can walk into. */
function isPlainObject(v) {
	return v !== null && typeof v === "object" && !Array.isArray(v);
}

/** Reports whether `a` and `b` serialize alike, regardless of key order. */
function sameValue(a, b) {
	if (a === b) return true;
	if (!isPlainObject(a) || !isPlainObject(b)) return JSON.stringify(a) === JSON.stringify(b);
	const keys = Object.keys(a);
	return keys.length === Object.keys(b).length && keys.every(k => sameValue(a[k], b[k]));
}

/**
 * Returns the path of every leaf in `overlay` that carries the value `base` already carries at
 * that same path.
 * @param base - The template the overlay is applied to.
 * @param overlay - The template taking precedence.
 * @param prefix - The path of `base`/`overlay` themselves, empty at the top.
 * @returns Dotted paths, one per repeated leaf.
 */
function duplicatedPaths(base, overlay, prefix = "") {
	const dups = [];
	for (const [key, value] of Object.entries(overlay)) {
		const path = prefix ? `${prefix}.${key}` : key;
		const baseValue = isPlainObject(base) ? base[key] : undefined;
		if (isPlainObject(value) && isPlainObject(baseValue)) dups.push(...duplicatedPaths(baseValue, value, path));
		else if (baseValue !== undefined && sameValue(value, baseValue)) dups.push(path);
	}
	return dups;
}

const cases = {
	"manifests: nothing is left unsubstituted"() {
		for (const [name, manifest] of [["chromium", chromium], ["firefox", firefox]]) {
			const text = JSON.stringify(manifest);
			ok(!text.includes("${"), `the ${name} manifest still carries a placeholder`);
		}
	},

	"manifests: both carry the version the package declares"() {
		eq(chromium.version, version, "chromium");
		eq(firefox.version, version, "firefox");
	},

	"manifests: the granted origins cover everything the content script matches"() {
		for (const [name, manifest] of [["chromium", chromium], ["firefox", firefox]]) {
			ok(Array.isArray(manifest.host_permissions),
				`${name} host_permissions is not a list — .includes would test substrings, not membership`);
			let matchCount = 0;
			for (const cs of manifest.content_scripts) {
				for (const match of cs.matches) {
					matchCount++;
					ok(Array.isArray(manifest.host_permissions) && manifest.host_permissions.includes(match),
						`${name} injects into ${match} without asking for it`);
				}
			}
			ok(matchCount > 0, `${name} content script matches nothing, so this case would check nothing`);
		}
	},

	"manifests: every file either one names is in the tree"() {
		for (const [name, manifest] of [["chromium", chromium], ["firefox", firefox]]) {
			for (const p of referencedPaths(manifest)) {
				ok(existsSync(join(EXT, p)), `${name} manifest points at a missing ${p}`);
			}
		}
	},

	"manifests: each declares a background script of the form its browser takes"() {
		ok(typeof chromium.background?.service_worker === "string",
			"chromium takes a service worker");
		ok(Array.isArray(firefox.background?.scripts) && firefox.background.scripts.length > 0,
			"firefox takes a script list — it does not support service_worker at all");
		deepEq(firefox.background.scripts, [chromium.background.service_worker],
			"and both name the same file");
	},

	"manifests: the background is a module in both, so it can import the configuration"() {
		eq(chromium.background?.type, "module", "chromium");
		eq(firefox.background?.type, "module", "firefox");
	},

	"manifests: the shared part exists once"() {
		// Read the templates themselves, unmerged and unsubstituted: the merged manifests can
		// carry the right value at a key whether the overlay names it or inherits it from the
		// base, so only the source templates can tell the two cases apart.
		const read = (name) => JSON.parse(readFileSync(join(TEMPLATE_DIR, name), "utf8"));
		const base = read("base.json");
		for (const file of ["chromium.json", "firefox.json"]) {
			for (const path of duplicatedPaths(base, read(file))) {
				ok(false, `${file} repeats ${path} from base.json — a copy waiting to drift`);
			}
		}
	}
};

await run(import.meta, cases);
