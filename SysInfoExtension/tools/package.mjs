// Lays out the source tree each browser's store and policy channel takes.

import { cpSync, existsSync, mkdirSync, rmSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { pathToFileURL } from "node:url";

import { EXT, buildConfigModule, loadConfig, packageVersion, substitutions } from "./config.mjs";
import { referencedPaths, renderManifest } from "./manifest.mjs";

/** The browsers a build targets, and the name each one's directories carry. */
export const TARGETS = { chromium: "Chromium", firefox: "Firefox" };

/**
 * What every target ships beyond what its manifest names.
 *
 * Anything a manifest key points at is collected by `referencedPaths` instead and is absent here
 * on purpose: a path typed in both places is a hand-kept copy of a templated one, free to drift.
 */
export const DELIVERY = [
	"_locales",
	// The popup page loads popup.css, popup.js and popup/lib/* itself; the manifest names only
	// popup.html.
	"popup",
	"shared",
	// Loaded by popup/lib/render.js as the header logo. No Chromium manifest key names it, so
	// without this entry the Chromium delivery opens the popup to a broken image.
	"assets/icons/sysinfo_ext.svg"
];

/**
 * Writes the tree `target` ships.
 * @param options.target - A key of TARGETS.
 * @param options.config - A configuration object as `loadConfig` returns it.
 * @param options.version - The version the manifest carries and the directory is named for.
 * @param options.outRoot - The directory the per-browser source directories sit in.
 * @param options.force - Replaces a directory that already exists.
 * @returns The directory written.
 */
export function packageTarget({ target, config, version, outRoot, force = false }) {
	const name = TARGETS[target];
	if (!name) throw new Error(`unknown target: ${target}`);

	const dir = join(outRoot, `SysInfoExtension_${name}_Source`, `SysInfoExtension_${name}_v${version}`);
	// A version already handed out through the policy channel must not change under the machines
	// that have it, so replacing one is something the caller says out loud.
	if (existsSync(dir) && !force) {
		throw new Error(`${dir} exists already — pass --force to rebuild a version that was published`);
	}
	rmSync(dir, { recursive: true, force: true });
	mkdirSync(dir, { recursive: true });

	const manifest = renderManifest(target, substitutions(config, version));
	for (const rel of [...DELIVERY, ...referencedPaths(manifest)]) {
		const from = join(EXT, rel);
		if (!existsSync(from)) throw new Error(`the delivery names a missing ${rel}`);
		mkdirSync(dirname(join(dir, rel)), { recursive: true });
		cpSync(from, join(dir, rel), { recursive: true });
	}
	writeFileSync(join(dir, "manifest.json"), JSON.stringify(manifest, null, "\t") + "\n");
	// Written from `config` rather than left as the copy of shared/build_config.js that came off
	// disk: npm forwards no `--` argument to a pre-script, so the module on disk answers to
	// whichever configuration the last build read, not to the one this manifest was rendered from.
	writeFileSync(join(dir, "shared", "build_config.js"), buildConfigModule(config));
	return dir;
}

/** Returns the targets `argv` asks for, which is all of them unless --target names one. */
function pickTargets(argv) {
	const i = argv.indexOf("--target");
	if (i === -1) return Object.keys(TARGETS);
	const named = argv[i + 1];
	if (!TARGETS[named]) throw new Error(`--target takes one of ${Object.keys(TARGETS).join(", ")}`);
	return [named];
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
	const argv = process.argv.slice(2);
	const { path, config } = loadConfig(argv);
	const version = packageVersion();
	console.log(`config: ${path}`);

	// The bundle is the prepackage script's to produce: it runs the build, which owns the esbuild
	// flags, so they are not repeated here. The generated configuration is not — `packageTarget`
	// writes it into each tree from the configuration named here.
	for (const target of pickTargets(argv)) {
		const dir = packageTarget({
			target, config, version,
			outRoot: join(EXT, "..", "build"),
			force: argv.includes("--force")
		});
		console.log(`wrote: ${dir}`);
	}
}
