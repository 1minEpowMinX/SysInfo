// Compatibility layer for WebExtension APIs and utilities.

// Cross-browser alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

// i18n helper. `substitutions` is an array passed straight to
// chrome.i18n.getMessage — combine with $PLACEHOLDER$ tokens in the
// _locales/*.json messages and a matching `placeholders` object there.
export const t = (key, substitutions) => chrome.i18n.getMessage(key, substitutions) || key;

// Diagnostic logger.
const SYSINFO_LOG_TAG = "[SysInfo]";
export const slog = (...args) => console.log(SYSINFO_LOG_TAG, ...args);
export const swarn = (...args) => console.warn(SYSINFO_LOG_TAG, ...args);

// Visible-in-Inspector sentinel — confirms the script ran even with
// console filters: document.documentElement.dataset.sysinfoLoaded
const _sysinfoRoot = document.documentElement;
_sysinfoRoot.dataset.sysinfoLoaded = String(Date.now());
_sysinfoRoot.dataset.sysinfoVersion =
	(chrome.runtime && chrome.runtime.getManifest && chrome.runtime.getManifest().version) || "?";

// Heartbeat to the background — proves to the SW console that the content
// script reached this point even if the page console is filtered.
try {
	browser.runtime.sendMessage(
		{
			action: "diagPing",
			source: "content-script-load",
			href: location.href,
			time: Date.now()
		},
		() => { if (chrome.runtime.lastError) { /* no-op */ } }
	);
} catch (e) { /* swallow */ }

slog("content script loaded", { href: location.href });
