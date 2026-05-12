// DOM helpers: tiny `el()` builder + a couple of reusable widgets.

/**
 * The function `el` creates a DOM element of the given `tag`, applies `attrs`, and appends
 * `children`. The `attrs` object supports a special `"class"` key (maps to `className`), an
 * `"html"` key (sets `innerHTML`), an `"on*"` prefix (attaches event listeners), a `"style"`
 * key (merges into `element.style`), and any other key is set as an attribute via
 * `setAttribute`. String and number children are wrapped in text nodes; `null`/`false` children
 * are skipped.
 * @param tag - The HTML tag name for the element to create (e.g. `"div"`, `"button"`).
 * @param attrs - Object of attributes, event handlers, style, or special keys to apply.
 * @param children - Array of child nodes, strings, or numbers to append to the element.
 * @returns The newly created and populated `HTMLElement`.
 */
export function el(tag, attrs = {}, children = []) {
	const e = document.createElement(tag);
	for (const k in attrs) {
		if (k === "class") e.className = attrs[k];
		else if (k === "html") e.innerHTML = attrs[k];
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
 * The function `segmented` renders a segmented control — a row of buttons where the button
 * matching `value` is marked active. Clicking any button calls `onChange` with that button's
 * value.
 * @param value - The currently selected option value; the matching button receives the
 * `"active"` class.
 * @param onChange - Callback invoked with the new value when the user clicks a button.
 * @param options - Array of `{ v, l }` objects where `v` is the option value and `l` is its
 * display label.
 * @returns A `div.seg` element containing one button per option.
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
 * The function `toggleRow` builds a labelled toggle row widget. Clicking the row inverts the
 * current boolean value and passes the new value to `onChange`.
 * @param label - The text label shown to the left of the toggle.
 * @param value - The current boolean state of the toggle; `true` renders the toggle as active.
 * @param onChange - Callback invoked with the new boolean value when the user clicks.
 * @returns A `label.toggle-row` element containing the label span and the toggle indicator.
 */
export function toggleRow(label, value, onChange) {
	const tog = el("div", { class: "toggle" + (value ? " on" : "") });
	return el("label", {
		class: "toggle-row",
		onclick: (e) => { e.preventDefault(); onChange(!value); }
	}, [el("span", { class: "lbl" }, label), tog]);
}

/**
 * The function `formatWhen` converts a Unix-millisecond timestamp into a human-readable relative
 * time string such as `"just now"`, `"5m ago"`, `"2h ago"`, or `"3d ago"`.
 * @param ts - Unix timestamp in milliseconds, or a falsy value if no timestamp is available.
 * @returns A relative time string, or an empty string when `ts` is falsy.
 */
export function formatWhen(ts) {
	if (!ts) return "";
	const diff = (Date.now() - ts) / 1000;
	if (diff < 60) return "just now";
	if (diff < 3600) return Math.floor(diff / 60) + "m ago";
	if (diff < 86400) return Math.floor(diff / 3600) + "h ago";
	return Math.floor(diff / 86400) + "d ago";
}
