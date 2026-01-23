// Firefox support: add an alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

/**
 * Translate a string using chrome.i18n
 * @param {string} key - The key of the string to translate
 * @returns {string} - The translated string, or the original key if no translation is found
 */
function t(key) {
	return chrome.i18n.getMessage(key) || key;
}

/**
 * Verify that this is the IT Services ticket creation page
 * @param {string} pathname - The current URL path
 * @returns {boolean} - True if the page is allowed, false otherwise
 */
function isTicketAllowed(pathname) {
	// Universal regex for extracting portalId and ticketId
	const match = pathname.match(
		/\/servicedesk\/customer\/portal\/(\d+)\/create\/(\d+)/
	);

	if (!match) return false;

	const portalId = match[1];
	const ticketId = match[2];

	// Check if the portal type id is allowed
	const allowedPortals = ["41", "141"];

	if (!allowedPortals.includes(portalId)) return false;

	// Check if the ticket type id is allowed
	const allowedTestIds = ["217", "218", "219", "220", "213", "216", "212"];
	const allowedProdIds = ["181", "182", "183", "184", "185", "187", "192"];

	return allowedTestIds.includes(ticketId) || allowedProdIds.includes(ticketId);
}

/**
 * Generates an array of strings representing the system information data
 * @param {Object} data - The system information data, containing the following properties:
 *   - {string} hostname - The hostname of the device
 *   - {string} username - The username of the currently logged in user
 *   - {string} ip - The IP address of the device
 *   - {string} uptime - The uptime of the device
 * @param {Object<string,string>} labelsSafe - The labels for the system information data, containing the following properties:
 *   - {string} hostname - The label for the hostname
 *   - {string} username - The label for the username
 *   - {string} ip - The label for the IP address
 *   - {string} uptime - The label for the uptime
 * @returns {Array<string>} - An array of strings representing system information data in pairs
 */
function buildSysInfoLines(data, labelsSafe) {
	const raw = [
		`${labelsSafe.hostname || t("sysinfoHostname")}: ${data.hostname}`,
		`${labelsSafe.username || t("sysinfoUsername")}: ${data.username}`,
		`${labelsSafe.ip || t("sysinfoIP")}: ${data.ip}`,
		`${labelsSafe.uptime || t("sysinfoUptime")}: ${data.uptime}`
	];

	// Split the array into pairs
	const result = [];
	for (let i = 0; i < raw.length; i += 2) {
		result.push(raw.slice(i, i + 2).join(", "));
	}

	return result;
}



/**
 * Generates a divider string based on the longest line length in the given array
 * The divider string is repeated until it reaches the specified percentage of the longest line length
 * @param {Array<string>} lines - The array of lines to get the longest length from
 * @param {string} [char="─"] - The character to use for the divider string
 * @param {number} [dividerPercent=0.45] - The percentage of the longest line length to repeat the divider string to
 * @returns {string} - The generated divider string
 */
function makeDivider(lines, char = "─", dividerPercent = 0.45) {
	// Each divider symbol has its own visual width. It is necessary to adjust it in percentage proportions
	const maxLen = Math.max(...lines.map(line => line.length));
	return char.repeat(Math.floor(maxLen * dividerPercent));
}

/**
 * Inserts the system information into the target element
 * @param {Element} target - The target element to insert the system information into
 * @param {Object} data - The system information to insert, containing the following properties:
 *   - {string} hostname - The hostname of the device
 *   - {string} username - The username of the currently logged in user
 *   - {string} ip - The IP address of the device
 *   - {string} uptime - The uptime of the device
 * @param {Object<string,string>} labels - Optional labels for the system information, containing the following properties:
 *   - {string} hostname - The label for the hostname
 *   - {string} username - The label for the username
 *   - {string} ip - The label for the IP address
 *   - {string} uptime - The label for the uptime
 */
function insertSysInfoInto(target, data) {
	const path = location.pathname;

	if (!isTicketAllowed(path)) {
		console.log("insertSysInfoInto: ticket type excluded or not applicable");
		return;
	}

	const labelsSafe = data.labels || {};
	const lines = buildSysInfoLines(data, labelsSafe);
	const divider = makeDivider(lines);
	const indents = "\n\u200B\n\u200B\n\u200B\n"; // Zero-width spaces to create some padding

	const text = `${indents}${divider}\n${lines.join("\n")}`;

	if (target.innerText.includes(divider)) return;

	if ('value' in target) {
		target.value = text;
		target.dispatchEvent(new Event("input", { bubbles: true }));
	} else {
		target.innerText = text;
	}

	showToast(t("toastReceived"), 10000);
}

/**
 * Watches for an element matching the given selector and calls the given function when it is found
 * @param {string} selector - The CSS selector to watch for
 * @param {function} onFound - The function to call when the element is found. It will be passed the element as an argument
 * @param {number} [intervalMs=2000] - The interval in milliseconds to check for the element
 */
function watchElement(selector, onFound, intervalMs = 2000) {
	let lastElement = null;

	const check = () => {
		const el = document.querySelector(selector);
		if (el && el !== lastElement) {
			lastElement = el;
			onFound(el);
		}
	};

	setInterval(check, intervalMs);
	const observer = new MutationObserver(check);
	observer.observe(document.body, { childList: true, subtree: true });
	check();
}

/**
 * Ensures that a toast container exists in the DOM. If the container does not exist, it is created.
 * The container is a div element with the id 'toast-container' and is appended to the documentElement.
 * This function is used by showToast to ensure that the toast container exists before attempting to show a toast.
 * @returns {undefined}
 */
function ensureToastContainer() {
	if (document.getElementById('toast-container')) return;
	const container = document.createElement('div');
	container.id = 'toast-container';
	document.documentElement.appendChild(container);
}

/**
 * Shows a toast message for the given duration
 * @param {string} message - The message to display in the toast
 * @param {number} [duration=3000] - The duration to display the toast for in milliseconds
 */
function showToast(message, duration = 3000) {
	ensureToastContainer();
	const container = document.getElementById('toast-container');
	const t = document.createElement('div');
	t.className = 'toast';
	t.textContent = message;
	container.appendChild(t);
	requestAnimationFrame(() => t.classList.add('show'));
	setTimeout(() => {
		t.classList.remove('show');
		t.addEventListener('transitionend', () => t.remove(), { once: true });
	}, duration);
}

// Request system information from the background script
browser.runtime.sendMessage({ action: "getSystemInfo" }, (res) => {
	if (!res.success) {
		console.error(t("agentError") + ":", res.error);
		return;
	}
	watchElement('#ak-editor-textarea > p', (element) => {
		insertSysInfoInto(element, res.data);
	});
});