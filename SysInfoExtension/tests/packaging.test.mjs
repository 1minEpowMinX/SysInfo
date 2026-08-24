// The packaging artefacts: the manifests the templates render to, and the bundle they ship.
//
// Nothing here is reachable through the extension's own imports — the browser loads these files,
// not the tests — so the checks read them off disk or render them in memory.

import { run } from "./runner.mjs";
import { ok, eq, deepEq, threw } from "./assert.mjs";
import { existsSync, mkdtempSync, readdirSync, readFileSync, rmSync } from "node:fs";
import { join, relative, sep } from "node:path";
import { tmpdir } from "node:os";

import { EXT, loadConfig, packageVersion, substitutions } from "../tools/config.mjs";
import { TEMPLATE_DIR, referencedPaths, renderManifest } from "../tools/manifest.mjs";
import { packageTarget, TARGETS } from "../tools/package.mjs";

const version = packageVersion();
const config = loadConfig([]).config;
const values = substitutions(config, version);
const chromium = renderManifest("chromium", values);
const firefox = renderManifest("firefox", values);

/** Returns every file under `dir`, as paths relative to it with forward slashes. */
function listFiles(dir) {
	return readdirSync(dir, { recursive: true, withFileTypes: true })
		.filter(e => e.isFile())
		.map(e => relative(dir, join(e.parentPath, e.name)).split(sep).join("/"));
}

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
	},

	"delivery: the tree holds what the browser loads and nothing else"() {
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			const dir = packageTarget({ target: "chromium", config, version, outRoot });
			const files = listFiles(dir);

			ok(files.includes("manifest.json"), "the rendered manifest is written");
			ok(files.includes("content/content_script.bundle.js"), "the bundle ships");
			ok(files.includes("popup/popup.html"), "the popup ships");
			ok(files.includes("shared/build_config.js"), "the generated configuration ships");

			ok(!files.some(f => f.startsWith("assets/icons/old_design/")),
				"the retired icon set stays out");
			ok(!files.includes("content/content_script.js"), "the unbundled source stays out");
			ok(!files.some(f => f.startsWith("content/lib/")), "and so do its imports");
			ok(!files.some(f => f.startsWith("tests/") || f.startsWith("tools/")),
				"the development files stay out");
			ok(!files.includes("package.json"), "and so does the manifest of the toolchain");
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	},

	"delivery: each target takes only the icons its manifest names"() {
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			const chromiumFiles = listFiles(packageTarget({ target: "chromium", config, version, outRoot }));
			const firefoxFiles = listFiles(packageTarget({ target: "firefox", config, version, outRoot }));

			ok(chromiumFiles.includes("assets/icons/sysinfo_ext_128.png"), "chromium takes its PNG set");
			ok(!chromiumFiles.includes("assets/icons/sysinfo_ext_512.png"),
				"the store listing artwork is not part of the extension");
			ok(firefoxFiles.includes("assets/icons/sysinfo_ext.svg"), "firefox takes the vector icon");
			ok(!firefoxFiles.some(f => f.endsWith("sysinfo_ext_128.png")),
				"and none of the raster set it never names");
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	},

	"delivery: a version already built is not overwritten by accident"() {
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			packageTarget({ target: "firefox", config, version, outRoot });
			const e = threw(() => packageTarget({ target: "firefox", config, version, outRoot }));
			ok(e !== null, "a second run over the same version stops");
			ok(String(e && e.message).includes("--force"), "and the message says how to mean it");
			ok(threw(() => packageTarget({ target: "firefox", config, version, outRoot, force: true })) === null,
				"--force goes through");
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	},

	"delivery: the directory is named after the browser and the version"() {
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			const dir = packageTarget({ target: "chromium", config, version, outRoot });
			eq(relative(outRoot, dir).split(sep).join("/"),
				`SysInfoExtension_${TARGETS.chromium}_Source/SysInfoExtension_${TARGETS.chromium}_v${version}`,
				"the layout the policy channel already distributes");
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	}
};

await run(import.meta, cases);
