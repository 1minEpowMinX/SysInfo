// Toast notification UI — fixed-position, programmatic DOM only.
// Avoids innerHTML.

import { t } from "./compat.js";

const SVG_NS = "http://www.w3.org/2000/svg";

/**
 * Creates an SVG element and sets `attrs` on it.
 * @param name - Tag name of the element, created in the SVG namespace.
 * @param attrs - Attributes set on the new element, one per key.
 * @returns The new element.
 */
function svgEl(name, attrs) {
	const el = document.createElementNS(SVG_NS, name);
	for (const k in attrs) el.setAttribute(k, attrs[k]);
	return el;
}

/** Returns the checkmark icon drawn in the toast's leading badge. */
function buildCheckIcon() {
	const svg = svgEl("svg", {
		width: "11", height: "11", viewBox: "0 0 24 24",
		fill: "none", stroke: "currentColor",
		"stroke-width": "3", "stroke-linecap": "round", "stroke-linejoin": "round"
	});
	svg.appendChild(svgEl("polyline", { points: "20 6 9 17 4 12" }));
	return svg;
}

/** Returns the icon drawn in the toast's dismiss button. */
function buildCloseIcon() {
	const svg = svgEl("svg", {
		width: "14", height: "14", viewBox: "0 0 24 24",
		fill: "none", stroke: "currentColor",
		"stroke-width": "2", "stroke-linecap": "round", "stroke-linejoin": "round"
	});
	svg.appendChild(svgEl("line", { x1: "18", y1: "6", x2: "6", y2: "18" }));
	svg.appendChild(svgEl("line", { x1: "6", y1: "6", x2: "18", y2: "18" }));
	return svg;
}

/**
 * Returns the element every toast is stacked into, creating and attaching it on the first call.
 * @returns The container element.
 */
function ensureToastContainer() {
	let container = document.getElementById("sysinfo-toast-container");
	if (container) return container;
	container = document.createElement("div");
	container.id = "sysinfo-toast-container";
	document.documentElement.appendChild(container);
	return container;
}

/**
 * Shows a toast carrying `message` and dismisses it once `duration` has passed.
 *
 * The dismiss button removes it earlier.
 * @param message - Body text of the toast, shown under the fixed title.
 * @param duration - Milliseconds the toast stays up.
 */
export function showToast(message, duration = 3000) {
	const container = ensureToastContainer();

	const toast = document.createElement("div");
	toast.className = "sysinfo-toast";

	const iconWrap = document.createElement("div");
	iconWrap.className = "sysinfo-toast__icon";
	iconWrap.setAttribute("aria-hidden", "true");
	iconWrap.appendChild(buildCheckIcon());

	const textWrap = document.createElement("div");
	textWrap.className = "sysinfo-toast__text";
	const title = document.createElement("div");
	title.className = "sysinfo-toast__title";
	title.textContent = t("bannerTitle");
	const body = document.createElement("div");
	body.className = "sysinfo-toast__body";
	body.textContent = message;
	textWrap.appendChild(title);
	textWrap.appendChild(body);

	const close = document.createElement("button");
	close.className = "sysinfo-toast__close";
	close.type = "button";
	close.setAttribute("aria-label", "Close");
	close.appendChild(buildCloseIcon());
	close.addEventListener("click", () => removeToast(toast));

	toast.appendChild(iconWrap);
	toast.appendChild(textWrap);
	toast.appendChild(close);

	container.appendChild(toast);
	requestAnimationFrame(() => toast.classList.add("show"));
	setTimeout(() => removeToast(toast), duration);
}

/**
 * Fades `toast` out and detaches it once the fade ends.
 * @param toast - The toast element; one already detached is left alone, so the timer and the
 * dismiss button may both reach it.
 */
function removeToast(toast) {
	if (!toast.isConnected) return;
	toast.classList.remove("show");
	toast.classList.add("sysinfo-toast--fading");
	const cleanup = () => toast.remove();
	// The timer is a backstop: transitionend never fires when the transition does not run,
	// which would strand the node in the DOM.
	toast.addEventListener("transitionend", cleanup, { once: true });
	setTimeout(cleanup, 600);
}
