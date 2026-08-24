// The build tooling's pure parts: what a configuration must carry, which file is read, and the
// values the manifest templates are rendered against.

import { run } from "./runner.mjs";
import { ok, eq, deepEq, threw } from "./assert.mjs";
import { mkdtempSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

import { REQUIRED_KEYS, configPath, loadConfig, packageVersion, substitutions } from "../tools/config.mjs";

/** Returns the path of a throwaway configuration file holding `config`. */
function tempConfig(config) {
	const dir = mkdtempSync(join(tmpdir(), "sysinfo-config-"));
	const path = join(dir, "build.config.json");
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
	}
};

await run(import.meta, cases);
