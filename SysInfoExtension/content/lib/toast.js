// Toast notification UI — fixed-position, programmatic DOM only.
// Avoids innerHTML.

const SVG_NS = "http://www.w3.org/2000/svg";

/**
 * The function `svgEl` creates and returns a new SVG element with the specified attributes.
 * @param name - The `name` parameter in the `svgEl` function represents the name of the SVG element
 * that you want to create, such as 'circle', 'rect', 'line', etc.
 * @param attrs - The `attrs` parameter in the `svgEl` function is an object that contains the
 * attributes to be set on the SVG element being created. Each key-value pair in the `attrs` object
 * represents an attribute and its corresponding value that will be set on the SVG element.
 * @returns The function `svgEl` is returning a newly created SVG element with the specified name and
 * attributes.
 */
function svgEl(name, attrs) {
	const el = document.createElementNS(SVG_NS, name);
	for (const k in attrs) el.setAttribute(k, attrs[k]);
	return el;
}

/**
 * The function `buildCheckIcon` creates a checkmark icon using SVG elements.
 * @returns An SVG element representing a check icon with a polyline shape.
 */
function buildCheckIcon() {
	const svg = svgEl("svg", {
		width: "11", height: "11", viewBox: "0 0 24 24",
		fill: "none", stroke: "currentColor",
		"stroke-width": "3", "stroke-linecap": "round", "stroke-linejoin": "round"
	});
	svg.appendChild(svgEl("polyline", { points: "20 6 9 17 4 12" }));
	return svg;
}

/**
 * The function `buildCloseIcon` creates a close icon using SVG elements.
 * @returns The `buildCloseIcon` function is returning an SVG element that represents a close icon. The
 * close icon consists of two diagonal lines forming an "X" shape.
 */
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
 * The function `ensureToastContainer` creates a toast container element if it doesn't already exist
 * and returns it.
 * @returns The function `ensureToastContainer()` returns the toast container element with the id
 * "sysinfo-toast-container" from the document if it already exists. If it doesn't exist, it creates a
 * new div element with that id, appends it to the document, and then returns the newly created
 * container element.
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
 * The function `showToast` displays a toast message with a specified message and duration.
 * @param message - The `message` parameter in the `showToast` function is the text that you want to
 * display in the toast notification. It is the information or message that you want to communicate to
 * the user through the toast.
 * @param [duration=3000] - The `duration` parameter in the `showToast` function specifies how long the
 * toast message will be displayed on the screen before automatically disappearing. By default, if no
 * duration is provided, the toast will be displayed for 3000 milliseconds (3 seconds). You can
 * customize this duration by passing a different
 */
function showToast(message, duration = 3000) {
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
 * The function `removeToast` removes a toast element by applying CSS classes for fading out and then
 * removes the element from the DOM after a transition or a timeout.
 * @param toast - The `toast` parameter in the `removeToast` function is a reference to the toast
 * element that you want to remove from the DOM. The function first checks if the `toast` element is
 * connected to the DOM using the `isConnected` property. If it is connected, the function then removes
 * @returns The function `removeToast` will return `undefined`.
 */
function removeToast(toast) {
	if (!toast.isConnected) return;
	toast.classList.remove("show");
	toast.classList.add("sysinfo-toast--fading");
	const cleanup = () => toast.remove();
	toast.addEventListener("transitionend", cleanup, { once: true });
	setTimeout(cleanup, 600);
}
