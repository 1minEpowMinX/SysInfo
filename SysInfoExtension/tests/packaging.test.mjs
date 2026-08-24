// The packaging artefacts: the manifests the templates render to, and the bundle they ship.
//
// Nothing here is reachable through the extension's own imports — the browser loads these files,
// not the tests — so the checks read them off disk or render them in memory.

import { run } from "./runner.mjs";
import { ok, eq, deepEq, threw } from "./assert.mjs";
import { existsSync, mkdtempSync, readdirSync, readFileSync, rmSync } from "node:fs";
import { join, relative, sep } from "node:path";
import { tmpdir } from "node:os";

import { EXT, buildConfigModule, loadConfig, packageVersion, substitutions } from "../tools/config.mjs";
import { TEMPLATE_DIR, isPlainObject, referencedPaths, renderManifest } from "../tools/manifest.mjs";
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

/** The file kinds an asset path can be typed in. */
const SOURCE_RE = /\.(?:js|css|html)$/;

// A reference is written relative to the file holding it, so any number of leading `../` is
// stripped: what the delivery has to carry is the path from the extension root.
const ASSET_RE = /(?:\.\.\/)*(assets\/[\w\-./]+)/g;

/**
 * Collects every path under assets/ that a source file in `dirs` loads by name.
 *
 * The manifest is deliberately not consulted: these are the references the page resolves for
 * itself, which is the set no manifest key names and `referencedPaths` therefore cannot reach.
 * @param dirs - Directories under the extension root to scan.
 * @returns The paths, relative to the extension root, without repeats.
 */
function assetReferences(dirs) {
	const found = new Set();
	for (const dir of dirs) {
		for (const rel of listFiles(join(EXT, dir))) {
			if (!SOURCE_RE.test(rel)) continue;
			for (const m of readFileSync(join(EXT, dir, rel), "utf8").matchAll(ASSET_RE)) found.add(m[1]);
		}
	}
	return [...found].sort();
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

	"delivery: each target takes the icon sizes its manifest names and no others"() {
		// Collected off the manifest rather than listed here, so the case holds no copy of the
		// templates. `referencedPaths` is deliberately not reused: the delivery is built with it,
		// and a case built with it too would agree with whatever it gets wrong.
		const namesIn = (m) => [...new Set([
			...Object.values(m.action.default_icon),
			...Object.values(m.icons)
		])];
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			for (const [target, manifest] of [["chromium", chromium], ["firefox", firefox]]) {
				const named = namesIn(manifest);
				ok(!named.some(p => /_(?:96|256|512)\.png$/.test(p)),
					`${target} names store listing artwork among the icons the browser loads`);

				const files = listFiles(packageTarget({ target, config, version, outRoot }));
				const shipped = files.filter(f => f.startsWith("assets/icons/") && f.endsWith(".png"));
				deepEq(shipped.sort(), named.sort(),
					`${target} ships the sizes its manifest names, no more and no fewer`);
				ok(files.includes("assets/icons/sysinfo_ext.svg"),
					`${target} takes the master, which the popup loads as its header logo`);
			}
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	},

	"delivery: the generated configuration is the one the manifest was rendered from"() {
		// Packaged from a configuration the on-disk shared/build_config.js was not written from,
		// which is the state `--config` leaves the tree in.
		const other = { ...config, agentOrigin: "https://sysinfo-agent.example:9443" };
		ok(other.agentOrigin !== config.agentOrigin,
			"the case needs two different origins to tell the two configurations apart");
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			const dir = packageTarget({ target: "chromium", config: other, version, outRoot });
			const shipped = readFileSync(join(dir, "shared", "build_config.js"), "utf8");
			const manifest = JSON.parse(readFileSync(join(dir, "manifest.json"), "utf8"));

			eq(shipped, buildConfigModule(other), "the module is rendered, not copied out of shared/");
			ok(!shipped.includes(config.agentOrigin),
				"so the origin the on-disk module carries is nowhere in the delivery");
			ok(manifest.host_permissions.includes(other.agentOrigin + "/*"),
				"and the manifest asks for the host the module beside it talks to");
		} finally {
			rmSync(outRoot, { recursive: true, force: true });
		}
	},

	"delivery: an asset a source loads by name ships for every target"() {
		const refs = assetReferences(["popup", "content"]);
		ok(refs.length > 0, "no asset reference was found at all, so this case would check nothing");
		const outRoot = mkdtempSync(join(tmpdir(), "sysinfo-delivery-"));
		try {
			for (const target of Object.keys(TARGETS)) {
				const files = listFiles(packageTarget({ target, config, version, outRoot }));
				for (const ref of refs) {
					ok(files.includes(ref), `${target} loads ${ref} but does not ship it`);
				}
			}
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
