// Form submission watcher.

import { slog } from "./compat.js";
import { EDITOR_SELECTOR, SUBMIT_CONTROL_SELECTOR } from "./constants.js";
import { markFormSubmitted } from "./history.js";
import { isTicketAllowed } from "./portals.js";

/**
 * Returns the request form, identified as the one carrying the description editor.
 *
 * Identified through the editor rather than through a class of its own: the editor is already
 * the anchor insertion depends on, so this adds no second piece of portal markup to keep up
 * with.
 * @returns The form element, or null when the page carries no editor.
 */
function requestForm() {
	const editor = document.querySelector(EDITOR_SELECTOR);
	return editor ? editor.closest("form") : null;
}

/**
 * Returns the control that `node` activates, when it is one that sends the request form.
 * @param node - The node an event was raised on.
 * @returns The control element, or null when the node activates none inside that form.
 */
function sendControlFor(node) {
	if (!(node instanceof Element)) return null;
	// closest() rather than a direct match: an event reaches the icon or the label inside the
	// button far more often than the button itself.
	const control = node.closest(SUBMIT_CONTROL_SELECTOR);
	if (!control) return null;
	const form = requestForm();
	return form && form.contains(control) ? control : null;
}

/**
 * Reports a submission of the request form.
 * @param event - The captured submit event.
 */
function checkSubmitEvent(event) {
	if (!isTicketAllowed(location.pathname)) return;
	// The script is injected into every page of the host, so a login or a search form raises
	// this event too; only the form the block went into can produce a ticket for the history.
	if (event.target !== requestForm()) return;
	// The portal's cancel button carries no type attribute, and a button inside a form is a
	// submit control by default, so cancelling raises this event as well. `submitter` names
	// the control that raised it and tells the two apart. An event carrying none is accepted:
	// a form sent from script names no control.
	if (event.submitter && !sendControlFor(event.submitter)) return;
	slog("form submit event");
	markFormSubmitted();
}

/**
 * Reports a click that lands on a control which sends the request form.
 * @param event - The captured click event.
 */
function checkSubmitClick(event) {
	if (!isTicketAllowed(location.pathname)) return;
	if (!sendControlFor(event.target)) return;
	slog("submit control clicked");
	markFormSubmitted();
}

/**
 * Installs the two hooks that catch a submission of the request form.
 *
 * Both may report the same submission; marking it twice only restamps the record.
 */
export function setupSubmitWatcher() {
	// Capture phase, so a submission the portal preventDefaults or stops the propagation of
	// is still recorded. One the portal rejects creates no ticket and no navigation follows,
	// which leaves the record to expire after SUBMIT_TTL_MS.
	document.addEventListener("submit", checkSubmitEvent, true);
	document.addEventListener("click", checkSubmitClick, true);
}
