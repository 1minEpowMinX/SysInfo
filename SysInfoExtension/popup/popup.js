// Firefox support: add an alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

/**
 * Applies translations to the page based on the user's
 * preferred language and the extension's localized messages.
 */
function applyI18n() {
	document.querySelectorAll('[i18n-content]').forEach(el => {
		const key = el.getAttribute('i18n-content');
		const msg = browser.i18n.getMessage(key);
		if (msg) el.textContent = msg;
	});
	document.title = browser.i18n.getMessage("extTitle");
}

/**
 * Loads the SysInfo extension's user interface immediately
 * upon loading. This involves applying the user's preferred
 * color scheme and translations, checking the status of the
 * local agent, and displaying a message to the user
 * indicating whether or not the agent is active.
 */
async function loadInfo() {
	const container = document.getElementById("info");
	// Apply translations immediately upon loading
	applyI18n();

	try {
		const status = await new Promise(resolve =>
			browser.runtime.sendMessage({ action: "getStatus" }, resolve)
		);

		container.textContent = ""; // clear old text safely

		const statusDiv = document.createElement("div");
		const hintDiv = document.createElement("div");
		hintDiv.className = "hint";

		if (status.success && status.text.trim() === "OK") {
			statusDiv.className = "status ok";
			statusDiv.textContent = browser.i18n.getMessage("agentActive");
		} else {
			statusDiv.className = "status error";
			statusDiv.textContent = browser.i18n.getMessage("agentInactive");
			hintDiv.textContent = browser.i18n.getMessage("agentHint");
		}

		container.appendChild(statusDiv);
		if (hintDiv.textContent) container.appendChild(hintDiv);

	} catch (err) {
		const statusDiv = document.createElement("div");
		statusDiv.className = "status error";
		statusDiv.textContent = browser.i18n.getMessage("agentInactive");
		container.appendChild(statusDiv);

		const hintDiv = document.createElement("div");
		hintDiv.className = "hint";
		hintDiv.textContent = browser.i18n.getMessage("agentHint");
		container.appendChild(hintDiv);
	}
}

loadInfo();
