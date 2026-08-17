// Form submission watcher.

import { slog } from "./compat.js";
import { SUBMIT_CONTROL_SELECTOR } from "./constants.js";
import { markFormSubmitted } from "./history.js";

/**
 * Reports whether `node` is, or sits inside, a control that sends the form.
 * @param node - The node to test; anything other than an element is never a control.
 */
function isSubmitControl(node) {
	// closest() rather than a direct match: an event reaches the icon or the label inside the
	// button far more often than the button itself.
	return node instanceof Element && node.closest(SUBMIT_CONTROL_SELECTOR) !== null;
}

/**
 * Reports a submission of the form the event was raised on.
 * @param event - The captured submit event.
 */
function checkSubmitEvent(event) {
	// The portal's cancel button carries no type attribute, and a button inside a form is a
	// submit control by default, so cancelling raises this event as well. `submitter` names
	// the control that raised it and tells the two apart. An event carrying none is accepted:
	// a form sent from script names no control.
	if (event.submitter && !isSubmitControl(event.submitter)) return;
	slog("form submit event");
	markFormSubmitted();
}

/**
 * Reports a click that lands on a control which sends the form.
 * @param event - The captured click event.
 */
function checkSubmitClick(event) {
	if (!isSubmitControl(event.target)) return;
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
