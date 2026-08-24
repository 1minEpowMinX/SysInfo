// Build-time configuration: the values the manifests and the browser code take from a file
// instead of from a literal in the tree.

import { existsSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

/** The extension root, which every path here is relative to. */
export const EXT = join(dirname(fileURLToPath(import.meta.url)), "..");

/**
 * The keys a configuration file carries.
 *
 * A file missing any of them is rejected rather than defaulted: the example is meant to be the
 * whole list of what a deployment sets, and a default inside this module would leave a key out
 * of it.
 */
export const REQUIRED_KEYS = ["agentOrigin", "jiraOrigins", "geckoId", "defaultPortalIds", "defaultTypeIds"];

const REAL = join(EXT, "build.config.json");
const EXAMPLE = join(EXT, "build.config.example.json");

/**
 * Returns the configuration file a run reads.
 * @param argv - The arguments after the script name.
 * @returns The path named by `--config`, the deployment's own file, or the committed example.
 */
export function configPath(argv = []) {
	const i = argv.indexOf("--config");
	if (i === -1) return existsSync(REAL) ? REAL : EXAMPLE;
	const named = argv[i + 1];
	if (!named || named.startsWith("--")) throw new Error("--config names no file");
	return resolve(named);
}

/**
 * Reads the configuration and checks that it is complete.
 * @param argv - The arguments after the script name.
 * @returns The path read and the parsed object.
 */
export function loadConfig(argv = []) {
	const path = configPath(argv);
	if (!existsSync(path)) throw new Error(`configuration file not found: ${path}`);
	const config = JSON.parse(readFileSync(path, "utf8"));
	const missing = REQUIRED_KEYS.filter(k => config[k] === undefined);
	if (missing.length > 0) throw new Error(`${path} is missing ${missing.join(", ")}`);
	return { path, config };
}

/**
 * Returns the version the package declares, which is the one the manifests carry.
 */
export function packageVersion() {
	return JSON.parse(readFileSync(join(EXT, "package.json"), "utf8")).version;
}

/** Returns `origin` as a match pattern covering every path under it. */
function glob(origin) {
	return origin.replace(/\/+$/, "") + "/*";
}

/**
 * Returns the table the manifest templates are rendered against.
 *
 * The match patterns are derived rather than configured: a second list of the same origins with
 * `/*` typed in by hand would drift away from the first one.
 * @param config - A configuration object as `loadConfig` returns it.
 * @param version - The version the manifests carry.
 * @returns A value for every placeholder the templates name.
 */
export function substitutions(config, version) {
	return {
		version,
		geckoId: config.geckoId,
		hostPermissions: [config.agentOrigin, ...config.jiraOrigins].map(glob),
		contentMatches: config.jiraOrigins.map(glob)
	};
}

/**
 * Returns the text of the module the browser code imports.
 *
 * Only the values the extension reads at runtime are emitted. The origins and the gecko id are
 * the manifest's, and the worker reads the granted origins back through `runtime.getManifest()`.
 * @param config - A configuration object as `loadConfig` returns it.
 */
export function buildConfigModule(config) {
	return [
		"// Generated from the build configuration by tools/config.mjs. Do not edit.",
		"",
		`export const AGENT_ORIGIN = ${JSON.stringify(config.agentOrigin)};`,
		`export const DEFAULT_PORTAL_IDS = ${JSON.stringify(config.defaultPortalIds)};`,
		`export const DEFAULT_TYPE_IDS = ${JSON.stringify(config.defaultTypeIds)};`,
		""
	].join("\n");
}

/**
 * Writes the generated module into the shared directory.
 * @param config - A configuration object as `loadConfig` returns it.
 * @returns The path written.
 */
export function writeBuildConfig(config) {
	const path = join(EXT, "shared", "build_config.js");
	writeFileSync(path, buildConfigModule(config));
	return path;
}

// Run directly, the module regenerates the browser's copy of the configuration. Imported, it
// stays a library: the tests call the same functions without writing anything.
if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
	const { path, config } = loadConfig(process.argv.slice(2));
	console.log(`config: ${path}`);
	console.log(`wrote: ${writeBuildConfig(config)}`);
}
