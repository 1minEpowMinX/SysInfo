// Firefox support: add an alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

/**
 * Build a fetch options object that marks the request as coming from SysInfo extension.
 * @returns {object} The fetch options object with the custom header "X-Sysinfo-Client" set to the extension's runtime ID
 */
function sysInfoFetchOptions() {
	return {
		headers: {
			"X-Sysinfo-Client": browser.runtime.id
		}
	};
}

// Handle messages from the content script
browser.runtime.onMessage.addListener((msg, _, sendResponse) => {
	if (msg.action === "getSystemInfo") {
		fetch("http://localhost:8734/systeminfo", sysInfoFetchOptions())
			.then(res => res.json())
			.then(data => sendResponse({ success: true, data }))
			.catch(err => sendResponse({ success: false, error: err.message }));
		return true;
	}

	if (msg.action === "getStatus") {
		fetch("http://localhost:8734/status", sysInfoFetchOptions())
			.then(res => res.text())
			.then(text => sendResponse({ success: true, text }))
			.catch(err => sendResponse({ success: false, error: err.message }));
		return true;
	}
});