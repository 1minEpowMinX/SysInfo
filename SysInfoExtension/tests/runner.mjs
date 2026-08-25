// Case runner shared by every test file.
//
// A module keeps its state for the life of the process, so a file whose cases would see each
// other's state asks for isolation and is spawned once per case.

import { execFileSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import { drain } from "./assert.mjs";

const MARK = "RESULT:";

/**
 * Runs the cases of the calling test file and prints their result.
 * @param meta - The file's `import.meta`, naming the file to re-spawn.
 * @param cases - Case name to async function; each receives no arguments.
 * @param options - `isolate` spawns one process per case.
 */
export async function run(meta, cases, options = {}) {
	const file = fileURLToPath(meta.url);
	const only = process.argv[2];

	if (only) {
		await runCase(cases, only);
		emit([[only, drain()]]);
		return;
	}

	if (options.isolate) {
		const results = [];
		for (const name of Object.keys(cases)) {
			try {
				const out = execFileSync(process.execPath, [file, name], { encoding: "utf8" });
				const line = out.split("\n").find(l => l.startsWith(MARK));
				if (line) results.push(...JSON.parse(line.slice(MARK.length)));
				else results.push([name, ["no result line — the case printed nothing"]]);
			} catch (e) {
				results.push([name, ["crashed: " + lastLines(e.stderr || e.message)]]);
			}
		}
		emit(results);
		return;
	}

	const results = [];
	for (const name of Object.keys(cases)) {
		await runCase(cases, name);
		results.push([name, drain()]);
	}
	emit(results);
}

/** Runs one case, turning a thrown error into a failure of that case. */
async function runCase(cases, name) {
	const fn = cases[name];
	if (!fn) { drain(); return; }
	try {
		await fn();
	} catch (e) {
		const { ok } = await import("./assert.mjs");
		ok(false, "threw: " + (e && e.stack ? e.stack.split("\n").slice(0, 3).join(" | ") : e));
	}
}

/** Prints the result line the parent reads. */
function emit(results) {
	process.stdout.write(MARK + JSON.stringify(results) + "\n");
}

/** Returns the tail of `text`, which is where a stack's cause usually sits. */
function lastLines(text) {
	return String(text).trim().split("\n").slice(0, 4).join(" | ");
}
