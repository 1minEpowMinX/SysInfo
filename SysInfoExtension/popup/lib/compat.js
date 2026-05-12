// Cross-browser alias
if (typeof browser === "undefined") {
	globalThis.browser = chrome;
}

// i18n helper. `substitutions` is an array passed straight to
// browser.i18n.getMessage — combine with $PLACEHOLDER$ tokens in the
// _locales/*.json messages and a matching `placeholders` object there.
export const t = (k, substitutions) => browser.i18n.getMessage(k, substitutions) || k;
