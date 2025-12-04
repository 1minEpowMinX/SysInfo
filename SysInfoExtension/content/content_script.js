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
 * @example
 * isTicketAllowed("/servicedesk/customer/portal/141/create/217") // true
 * isTicketAllowed("/servicedesk/customer/portal/141/create/222") // false
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
 * Inserts the system information into the target element
 * @param {Element} target - The target element to insert the system information into
 * @param {Object} data - The system information to insert, containing the following properties:
 *   - {string} hostname - The hostname of the device
 *   - {string} username - The username of the currently logged in user
 *   - {string} ip - The IP address of the device
 *   - {string} uptime - The uptime of the device
 *   - {Object<string,string>} labels - Optional labels for the system information, containing the following properties:
 *     - {string} hostname - The label for the hostname
 *     - {string} username - The label for the username
 *     - {string} ip - The label for the IP address
 *     - {string} uptime - The label for the uptime
 */

function insertSysInfoInto(target, data) {
	const path = location.pathname;

	if (!isTicketAllowed(path)) {
		console.log("insertSysInfoInto: ticket type excluded or not applicable");
		return;
	}

	const labelsSafe = data.labels || {};

	const text =
		`${t("sysinfoHeader")}\n` +
		`${t("sysinfoDivider")}\n` +
		`${labelsSafe.hostname || t("sysinfoHostname")}: ${data.hostname}\n` +
		`${labelsSafe.username || t("sysinfoUsername")}: ${data.username}\n` +
		`${labelsSafe.ip || t("sysinfoIP")}: ${data.ip}\n` +
		`${labelsSafe.uptime || t("sysinfoUptime")}: ${data.uptime}`;

	if ('value' in target) {
		if (target.value.includes(data.hostname)) return;
		target.value = text;
		target.dispatchEvent(new Event("input", { bubbles: true }));
	} else {
		if (target.innerText.includes(data.hostname)) return;
		target.innerText = text;
	}

	showToast(t("toastReceived"), 10000);
}

/**
 * Watches for an element matching the given selector and calls the given function when it is found
 * @param {string} selector - The CSS selector to watch for
 * @param {function} onFound - The function to call when the element is found. It will be passed the element as an argument.
 * @example
 * watchElement("#my-element", el => console.log(el));
 */
function watchElement(selector, onFound) {
	let lastElement = null;

	const check = () => {
		const el = document.querySelector(selector);
		if (el && el !== lastElement) {
			lastElement = el;
			onFound(el);
		}
	};

	setInterval(check, 2000);
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
 * @example
 * showToast("Saved successfully", 5000);
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

browser.runtime.sendMessage({ action: "getSystemInfo" }, (res) => {
	if (!res.success) {
		console.error(t("agentError") + ":", res.error);
		return;
	}
	watchElement('#ak-editor-textarea > p', (element) => {
		insertSysInfoInto(element, res.data);
	});
});