// The packaging artefacts: the bundle the manifests ship and the two manifests themselves.
//
// Nothing here is reachable through a module import — the browser loads these files, not the
// tests — so the checks read them off disk.

import { run } from "./runner.mjs";
import { ok, deepEq } from "./assert.mjs";
import { execFileSync } from "node:child_process";
import { existsSync, mkdtempSync, readFileSync, rmSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { fileURLToPath } from "node:url";

const EXT = fileURLToPath(new URL("..", import.meta.url));
const ESBUILD = join(EXT, "node_modules/esbuild/bin/esbuild");

/** Returns the text of a file named relative to the extension root. */
const read = (p) => readFileSync(join(EXT, p), "utf8");
/** Returns the parsed contents of a JSON file named relative to the extension root. */
const readJson = (p) => JSON.parse(read(p));

const chromium = readJson("chromium_manifest.json");
const firefox = readJson("firefox_manifest.json");

// What the extension is and what it may touch. The two manifests part company on how the
// background script is declared and on the icon formats each browser takes, and on nothing else.
const SHARED_FIELDS = [
	"manifest_version", "name", "description", "version", "default_locale",
	"permissions", "host_permissions", "content_scripts"
];

/**
 * Collects every path inside the extension that `manifest` points at.
 * @param manifest - A parsed manifest.
 * @returns The paths, relative to the extension root.
 */
function referencedPaths(manifest) {
	const paths = [];
	const push = (v) => { if (typeof v === "string") paths.push(v); };

	push(manifest.action?.default_popup);
	const icon = manifest.action?.default_icon;
	if (typeof icon === "string") push(icon);
	else Object.values(icon || {}).forEach(push);
	Object.values(manifest.icons || {}).forEach(push);

	for (const cs of manifest.content_scripts || []) {
		(cs.js || []).forEach(push);
		(cs.css || []).forEach(push);
	}
	push(manifest.background?.service_worker);
	(manifest.background?.scripts || []).forEach(push);

	return paths;
}

const cases = {
	"bundle: the shipped file is what the sources build to"() {
		if (!existsSync(ESBUILD)) {
			ok(false, "esbuild is not installed — run npm install before the suite");
			return;
		}

		// The flags are taken from the build script rather than repeated here, so the check
		// cannot drift away from the command that produces the artefact.
		const argv = readJson("package.json").scripts.build.split(/\s+/);
		const shipped = argv.find(a => a.startsWith("--outfile="))?.slice("--outfile=".length);
		if (!shipped) {
			ok(false, "the build script names no --outfile");
			return;
		}

		const dir = mkdtempSync(join(tmpdir(), "sysinfo-bundle-"));
		const fresh = join(dir, "bundle.js");
		try {
			const args = argv.slice(1).map(a => a.startsWith("--outfile=") ? "--outfile=" + fresh : a);
			// esbuild reports the size it wrote on stderr, which would otherwise land in the
			// runner's output next to the case names.
			execFileSync(process.execPath, [ESBUILD, ...args], { cwd: EXT, stdio: "ignore" });
			ok(readFileSync(fresh, "utf8") === read(shipped),
				`${shipped} is not what the sources build to — run npm run build`);
		} finally {
			rmSync(dir, { recursive: true, force: true });
		}
	},

	"manifests: the two agree on everything but how each browser loads them"() {
		for (const field of SHARED_FIELDS) deepEq(firefox[field], chromium[field], field);
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
			"firefox takes a script list");
		deepEq(firefox.background.scripts, [chromium.background.service_worker],
			"and both name the same file");
	}
};

await run(import.meta, cases);
