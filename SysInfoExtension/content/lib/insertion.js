// Editor watcher + insertion of the sysinfo block.

/**
 * The function `insertSysInfoInto` inserts system information into a target element based on certain
 * conditions.
 * @param target - The `target` parameter in the `insertSysInfoInto` function refers to the element in
 * the DOM (Document Object Model) where the system information data will be inserted. This element can
 * be a textarea, input field, or any other element that can display text content. The function checks
 * if the
 * @param data - The `data` parameter in the `insertSysInfoInto` function likely contains information
 * related to system information that needs to be inserted into a target element. This data could
 * include details such as system specifications, user information, or any other relevant information
 * that needs to be displayed within the target element on a
 * @returns If the `insertSysInfoInto` function is called and the conditions in the code are met, the
 * function will insert system information into the specified target element on the webpage. If the
 * conditions are not met, the function will not perform any insertion and will return without making
 * any changes to the target element.
 */
function insertSysInfoInto(target, data) {
	const path = location.pathname;
	const formMatch = path.match(FORM_PATH_RE);
	if (!formMatch || !isTicketAllowed(path)) return;

	const lines = buildSysInfoLines(data);
	const divider = makeDivider(lines);
	const indents = "\n​\n​\n​\n";
	const text = `${indents}${divider}\n${lines.join("\n")}`;

	if (alreadyInserted(target, divider)) return;

	slog("inserting sysinfo block", { path, target: target.tagName });

	if ("value" in target) {
		target.value = text;
		target.dispatchEvent(new Event("input", { bubbles: true }));
	} else {
		target.innerText = text;
	}

	showToast(t("toastReceived"), 10000);
	markPendingInsertion(formMatch[1], formMatch[2]);
}

/**
 * The `watchEditor` function continuously monitors for changes in the document body and inserts system
 * information into the editor element when it detects a change.
 * @param data - The `data` parameter in the `watchEditor` function is an object that contains
 * information to be inserted into the editor element. This data could include various properties or
 * values that are used by the `insertSysInfoInto` function when updating the editor element.
 */
function watchEditor(data) {
	let lastElement = null;

	const check = () => {
		const el = document.querySelector(EDITOR_SELECTOR);
		if (el && el !== lastElement) {
			lastElement = el;
			insertSysInfoInto(el, data);
		}
	};

	slog("editor watcher armed", { selector: EDITOR_SELECTOR, intervalMs: INSERTION_TICK_MS });
	setInterval(check, INSERTION_TICK_MS);
	new MutationObserver(check).observe(document.body, { childList: true, subtree: true });
	check();
}

/**
 * The function `startInsertion` requests system information and then watches the editor based on the
 * received data.
 */
function startInsertion() {
	requestSysInfo((data) => {
		if (!data) {
			swarn("insertion: no data — watcher not started");
			return;
		}
		watchEditor(data);
	});
}
