// Runs every *.test.mjs beside this file and prints the tally.

import { execFileSync } from "node:child_process";
import { readdirSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = fileURLToPath(new URL(".", import.meta.url));
const MARK = "RESULT:";

const files = readdirSync(HERE).filter(f => f.endsWith(".test.mjs")).sort();
let passed = 0, failed = 0;

for (const file of files) {
	process.stdout.write(`\n${file}\n`);
	let out;
	try {
		out = execFileSync(process.execPath, [join(HERE, file)], { encoding: "utf8" });
	} catch (e) {
		console.log(`  FAIL  <file crashed>\n        ${String(e.stderr || e.message).trim().split("\n").slice(0, 5).join("\n        ")}`);
		failed++;
		continue;
	}
	const line = out.split("\n").find(l => l.startsWith(MARK));
	if (!line) { console.log("  FAIL  <no result line>"); failed++; continue; }

	for (const [name, failures] of JSON.parse(line.slice(MARK.length))) {
		if (failures.length === 0) { passed++; console.log(`  ok    ${name}`); }
		else {
			failed++;
			console.log(`  FAIL  ${name}`);
			for (const f of failures) console.log(`        ${f}`);
		}
	}
}

console.log(`\n${passed} passed, ${failed} failed, ${passed + failed} total`);
process.exit(failed ? 1 : 0);
