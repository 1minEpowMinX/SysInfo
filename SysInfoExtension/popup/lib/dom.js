// DOM helpers: tiny `el()` builder + a couple of reusable widgets.

import { t } from "./compat.js";

/**
 * Parses an inline SVG string and returns its root element.
 *
 * The one place in the popup that parses markup, which is why `el()` accepts none. Callers pass
 * the static icons from icons.js and never user-controlled data.
 * @param svgStr - An `<svg>…</svg>` string.
 * @returns The parsed `SVGSVGElement`.
 */
export function svgIcon(svgStr) {
	const tmp = document.createElement("span");
	tmp.innerHTML = svgStr;
	return tmp.firstElementChild;
}

/**
 * Creates an element, applies `attrs` to it and appends `children`.
 * @param tag - Tag name of the element to create.
 * @param attrs - A `class` key sets the class name, an `on`-prefixed key adds the matching event
 * listener, a `style` key is merged into the element's style, and every other key becomes an
 * attribute.
 * @param children - Nodes to append; a string or a number becomes a text node, and a null or
 * false entry is skipped so a caller can inline a condition.
 * @returns The new element.
 */
export function el(tag, attrs = {}, children = []) {
	const e = document.createElement(tag);
	for (const k in attrs) {
		if (k === "class") e.className = attrs[k];
		else if (k.startsWith("on")) e.addEventListener(k.slice(2).toLowerCase(), attrs[k]);
		else if (k === "style") Object.assign(e.style, attrs[k]);
		else e.setAttribute(k, attrs[k]);
	}
	(Array.isArray(children) ? children : [children]).forEach(c => {
		if (c == null || c === false) return;
		if (typeof c === "string" || typeof c === "number") e.appendChild(document.createTextNode(c));
		else e.appendChild(c);
	});
	return e;
}

/**
 * Builds a segmented control — a row of buttons of which the one matching `value` is active.
 * @param value - The selected option's value.
 * @param onChange - Receives the value of the button the user clicks.
 * @param options - The options, each carrying its value as `v` and its label as `l`.
 * @returns The control element, holding one button per option.
 */
export function segmented(value, onChange, options) {
	return el("div", { class: "seg" }, options.map(o =>
		el("button", {
			class: o.v === value ? "active" : "",
			onclick: () => onChange(o.v)
		}, o.l)
	));
}

/**
 * Builds a labelled row carrying a toggle that the whole row's width switches.
 * @param label - The text shown to the left of the toggle.
 * @param value - The toggle's current state.
 * @param onChange - Receives the inverted state when the user clicks.
 * @returns The row element.
 */
export function toggleRow(label, value, onChange) {
	const tog = el("div", { class: "toggle" + (value ? " on" : "") });
	return el("label", {
		class: "toggle-row",
		onclick: (e) => { e.preventDefault(); onChange(!value); }
	}, [el("span", { class: "lbl" }, label), tog]);
}

/**
 * Formats a timestamp as a relative time, coarsening from minutes to days as it ages.
 * @param ts - A Unix timestamp in milliseconds.
 * @returns The relative time in the interface language, or an empty string when `ts` is falsy.
 */
export function formatWhen(ts) {
	if (!ts) return "";
	const diff = (Date.now() - ts) / 1000;
	// getMessage substitutes strings alone; a number reaches the catalogue as an empty
	// placeholder and the count silently disappears from the line.
	if (diff < 60) return t("timeJustNow");
	if (diff < 3600) return t("timeMinutesAgo", [String(Math.floor(diff / 60))]);
	if (diff < 86400) return t("timeHoursAgo", [String(Math.floor(diff / 3600))]);
	return t("timeDaysAgo", [String(Math.floor(diff / 86400))]);
}
