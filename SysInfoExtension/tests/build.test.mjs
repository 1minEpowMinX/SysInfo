// The build tooling's pure parts: what a configuration must carry, which file is read, and the
// values the manifest templates are rendered against.

import { run } from "./runner.mjs";
import { ok, eq, deepEq, threw } from "./assert.mjs";
import { mkdtempSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { pathToFileURL } from "node:url";

import { REQUIRED_KEYS, buildConfigModule, configPath, loadConfig, packageVersion, substitutions, writeBuildConfig } from "../tools/config.mjs";
import { merge, substitute } from "../tools/manifest.mjs";

/** Directories `tempDir()` has created, removed once every case has run. */
const tempDirs = [];

/** Returns a throwaway directory of this run's own. */
function tempDir() {
	const dir = mkdtempSync(join(tmpdir(), "sysinfo-config-"));
	tempDirs.push(dir);
	return dir;
}

/** Returns the path of a throwaway configuration file holding `config`. */
function tempConfig(config) {
	const path = join(tempDir(), "build.config.json");
	writeFileSync(path, JSON.stringify(config));
	return path;
}

const EXAMPLE = { ...loadConfig([]).config };

const cases = {
	"config: the example carries every required key"() {
		const { path, config } = loadConfig([]);
		ok(path.endsWith("build.config.example.json") || path.endsWith("build.config.json"),
			"the example is read when no real configuration is present");
		for (const key of REQUIRED_KEYS) {
			ok(config[key] !== undefined, `the example is missing ${key}`);
		}
	},

	"config: a file missing a key is rejected"() {
		const partial = { ...EXAMPLE };
		delete partial.geckoId;
		const e = threw(() => loadConfig(["--config", tempConfig(partial)]));
		ok(e !== null, "a configuration without geckoId is an error");
		ok(String(e && e.message).includes("geckoId"), "and the message names the missing key");
	},

	"config: --config names the file that is read"() {
		const path = tempConfig(EXAMPLE);
		eq(configPath(["--config", path]), path, "the named file wins over both defaults");
	},

	"config: --config without a path is an error"() {
		ok(threw(() => configPath(["--config"])) !== null, "the flag needs its argument");
	},

	"substitutions: the agent and every Jira origin become host permissions"() {
		const values = substitutions(EXAMPLE, "9.9.9");
		eq(values.version, "9.9.9", "the version comes from the caller");
		eq(values.geckoId, EXAMPLE.geckoId, "the gecko id is passed through");
		deepEq(values.hostPermissions,
			[EXAMPLE.agentOrigin, ...EXAMPLE.jiraOrigins].map(o => o + "/*"),
			"the agent leads, then the Jira origins, each with a path glob");
	},

	"substitutions: only the Jira origins are matched by the content script"() {
		const values = substitutions(EXAMPLE, "9.9.9");
		deepEq(values.contentMatches, EXAMPLE.jiraOrigins.map(o => o + "/*"),
			"the agent origin is granted but never injected into");
	},

	"substitutions: a trailing slash does not double up"() {
		const values = substitutions({ ...EXAMPLE, jiraOrigins: ["https://jira.example/"] }, "1.0.0");
		deepEq(values.contentMatches, ["https://jira.example/*"], "one slash, whatever the file carried");
	},

	"version: the package declares one"() {
		ok(/^\d+\.\d+\.\d+$/.test(packageVersion()), "package.json carries a three-part version");
	},

	"generated module: it exports what the browser code reads"() {
		const text = buildConfigModule(EXAMPLE);
		ok(text.includes("Do not edit"), "the file says it is generated");
		ok(text.includes(`export const AGENT_ORIGIN = ${JSON.stringify(EXAMPLE.agentOrigin)};`),
			"the agent origin is exported");
		ok(text.includes(`export const DEFAULT_PORTAL_IDS = ${JSON.stringify(EXAMPLE.defaultPortalIds)};`),
			"the portal seed is exported");
		ok(text.includes(`export const DEFAULT_TYPE_IDS = ${JSON.stringify(EXAMPLE.defaultTypeIds)};`),
			"the type seed is exported");
	},

	"generated module: the manifest's own values stay out of it"() {
		const text = buildConfigModule(EXAMPLE);
		ok(!text.includes(EXAMPLE.geckoId), "the gecko id is the manifest's, not the code's");
		ok(!text.includes(EXAMPLE.jiraOrigins[0]),
			"host permissions are read back from runtime.getManifest(), not duplicated here");
	},

	async "generated module: what is written is what is imported"() {
		// Written into a directory of this case's own: the default target is the build artefact
		// the browser code imports, and a test that rewrites it hides a stale one.
		const dir = tempDir();
		const path = writeBuildConfig(EXAMPLE, dir);
		ok(path.startsWith(dir), "the module lands in the directory it was given");
		const mod = await import(pathToFileURL(path).href);
		eq(mod.AGENT_ORIGIN, EXAMPLE.agentOrigin, "the written module parses and exports");
		deepEq(mod.DEFAULT_PORTAL_IDS, EXAMPLE.defaultPortalIds, "and carries the seeds");
	},

	"merge: an overlay object joins the base one instead of replacing it"() {
		const merged = merge(
			{ action: { default_popup: "popup/popup.html" }, name: "base" },
			{ action: { default_icon: "icon.svg" } });
		deepEq(merged.action, { default_popup: "popup/popup.html", default_icon: "icon.svg" },
			"the popup survives an overlay that only names an icon");
		eq(merged.name, "base", "a key the overlay omits is left alone");
	},

	"merge: an overlay array replaces the base one"() {
		const merged = merge({ permissions: ["a", "b"] }, { permissions: ["c"] });
		deepEq(merged.permissions, ["c"], "a list is taken whole, never appended to");
	},

	"substitute: a whole placeholder takes the value's own type"() {
		const out = substitute({ hosts: "${hostPermissions}" }, { hostPermissions: ["a/*", "b/*"] });
		deepEq(out.hosts, ["a/*", "b/*"], "a string in the template is a list in the result");
	},

	"substitute: a placeholder inside a longer string is inserted as text"() {
		const out = substitute({ id: "sysinfo-${version}" }, { version: "2.1.0" });
		eq(out.id, "sysinfo-2.1.0", "the surrounding text is kept");
	},

	"substitute: an unknown placeholder is an error"() {
		const e = threw(() => substitute({ v: "${nope}" }, { version: "1.0.0" }));
		ok(e !== null, "an unresolved name fails the build");
		ok(String(e && e.message).includes("nope"), "and the message names it");
	},

	"substitute: nested values are reached"() {
		const out = substitute(
			{ a: [{ matches: "${contentMatches}" }] },
			{ contentMatches: ["x/*"] });
		deepEq(out.a[0].matches, ["x/*"], "arrays and objects are walked through");
	},

	"substitute: a placeholder name outside the identifier charset is still an error, not literal text"() {
		const e = threw(() => substitute({ id: "${gecko-id}" }, {}));
		ok(e !== null, "a hyphenated name fails the build rather than surviving into the manifest");
		ok(String(e && e.message).includes("gecko-id"), "and the message names it");
	}
};

await run(import.meta, cases);

for (const dir of tempDirs) rmSync(dir, { recursive: true, force: true });
