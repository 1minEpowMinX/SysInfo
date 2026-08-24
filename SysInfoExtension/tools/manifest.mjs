// Renders a browser's manifest from the shared base, that browser's overlay and the build
// configuration.

import { readFileSync } from "node:fs";
import { join } from "node:path";
import { EXT } from "./config.mjs";

/** The directory holding the base and the per-browser overlays. */
export const TEMPLATE_DIR = join(EXT, "manifest");

/** Reports whether `v` is a keyed object rather than an array or a primitive. */
export function isPlainObject(v) {
	return v !== null && typeof v === "object" && !Array.isArray(v);
}

/**
 * Returns `overlay` merged onto `base`.
 *
 * Objects are joined key by key; anything else, arrays included, is taken from the overlay whole.
 * @param base - The value the overlay is applied to.
 * @param overlay - The value taking precedence.
 * @returns The merged value.
 */
export function merge(base, overlay) {
	if (!isPlainObject(base) || !isPlainObject(overlay)) return overlay;
	// Keeps every key the overlay does not name, so an overlay naming one key of `action` does
	// not have to repeat the rest of it.
	const out = { ...base };
	for (const [key, value] of Object.entries(overlay)) out[key] = merge(base[key], value);
	return out;
}

// Any run of characters up to the closing brace names a placeholder, not just identifier
// characters: a name spelled with a hyphen or a dot is still meant to resolve, and must reach
// valueOf() to fail loudly instead of surviving into the manifest as literal text.
const WHOLE = /^\$\{([^}]+)\}$/;
const EMBEDDED = /\$\{([^}]+)\}/g;

/** Returns a copy of the value `name` stands for, failing the build when nothing does. */
function valueOf(name, values) {
	if (!(name in values)) throw new Error(`no value for \${${name}}`);
	// Deep-copied so two renders sharing one values table — one per target browser — never hold
	// the same array or object as each other or as the table itself.
	return structuredClone(values[name]);
}

/**
 * Returns `node` with every `${name}` resolved against `values`.
 *
 * A string that is exactly one placeholder becomes the value itself; one embedded in a longer
 * string is resolved in place, as text.
 * @param node - A parsed template, or any part of one.
 * @param values - Placeholder name to value.
 * @returns The resolved copy.
 */
export function substitute(node, values) {
	if (Array.isArray(node)) return node.map(v => substitute(v, values));
	if (isPlainObject(node)) {
		return Object.fromEntries(Object.entries(node).map(([k, v]) => [k, substitute(v, values)]));
	}
	if (typeof node !== "string") return node;

	const whole = WHOLE.exec(node);
	// A whole match returns the value's own type, so a template can hold a list where JSON only
	// allows a string.
	if (whole) return valueOf(whole[1], values);
	return node.replace(EMBEDDED, (_, name) => String(valueOf(name, values)));
}

/**
 * Returns the unsubstituted manifest for `target`.
 * @param target - "chromium" or "firefox".
 * @returns The base merged with that browser's overlay, placeholders still in place.
 */
export function template(target) {
	const read = (name) => JSON.parse(readFileSync(join(TEMPLATE_DIR, name), "utf8"));
	return merge(read("base.json"), read(`${target}.json`));
}

/**
 * Returns the finished manifest for `target`.
 * @param target - "chromium" or "firefox".
 * @param values - The table `substitutions` builds.
 * @returns The manifest, with every placeholder resolved.
 */
export function renderManifest(target, values) {
	return substitute(template(target), values);
}

/**
 * Collects every path inside the extension that `manifest` points at.
 * @param manifest - A rendered manifest.
 * @returns The paths, relative to the extension root.
 */
export function referencedPaths(manifest) {
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
