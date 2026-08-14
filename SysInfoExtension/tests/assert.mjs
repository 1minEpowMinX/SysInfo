// Assertions. Every failure is collected rather than thrown, so one case reports all of its
// problems instead of only the first.

const failures = [];

/** Records `what` as a failure unless `cond` holds. */
export function ok(cond, what) {
	if (!cond) failures.push(what);
}

/** Records a failure unless `actual` and `expected` are the same value. */
export function eq(actual, expected, what) {
	if (!Object.is(actual, expected)) failures.push(`${what}: expected ${fmt(expected)}, got ${fmt(actual)}`);
}

/** Records a failure unless `actual` and `expected` serialize alike. */
export function deepEq(actual, expected, what) {
	const a = JSON.stringify(actual), b = JSON.stringify(expected);
	if (a !== b) failures.push(`${what}: expected ${b}, got ${a}`);
}

/** Records a failure unless `re` matches `text`. */
export function matches(text, re, what) {
	if (!re.test(String(text))) failures.push(`${what}: ${fmt(text)} does not match ${re}`);
}

/** Returns the failures recorded so far and clears the list. */
export function drain() {
	return failures.splice(0, failures.length);
}

/** Returns a short readable form of `v`. */
function fmt(v) {
	if (typeof v === "string") return JSON.stringify(v);
	if (v === undefined) return "undefined";
	try { return JSON.stringify(v); } catch { return String(v); }
}
