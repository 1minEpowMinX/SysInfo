// Firefox support: add an alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

browser.runtime.onMessage.addListener((msg, sender, sendResponse) => {
	if (msg.action === "getSystemInfo") {
		fetch("http://localhost:8734/systeminfo")
			.then(res => res.json())
			.then(data => sendResponse({ success: true, data }))
			.catch(err => sendResponse({ success: false, error: err.message }));
		return true; // Indicates that we will respond asynchronously
	}

	if (msg.action === "getStatus") {
		fetch("http://localhost:8734/status")
			.then(res => res.text())
			.then(text => sendResponse({ success: true, text }))
			.catch(err => sendResponse({ success: false, error: err.message }));
		return true;
	}
});
