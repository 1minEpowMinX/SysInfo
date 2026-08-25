// History tab — recent tickets where data was attached.
// Entries are written by the content script in lib/history.js after a
// successful insertion + redirect to the created ticket page.

import { state } from "./state.js";
import { t } from "./compat.js";
import { el, formatWhen, svgIcon } from "./dom.js";
import { I } from "./icons.js";

/**
 * Builds the History tab: the most recent tickets the block was inserted into, each a link
 * opening in a new tab, or an empty-state message when there are none.
 * @returns The tab body.
 */
export function renderHistoryTab() {
	const items = state.history || [];
	const head = el("div", { style: { paddingBottom: "8px" } }, [
		el("div", { class: "h", style: { fontSize: "12px", fontWeight: "600" } }, t("sectionHistory")),
		el("div", { style: { fontSize: "11px", color: "var(--text-sub)", marginTop: "2px" } }, t("sectionHistorySub"))
	]);

	let body;
	if (items.length === 0) {
		body = el("div", { class: "hist-empty" }, t("historyEmpty"));
	} else {
		body = el("div", { class: "hist-list" }, items.slice(0, 10).map(it => {
			// The entry comes from storage, which a page cannot reach but the user can edit
			// through devtools; anything but http(s) here would make the link a script sink.
			const safeUrl = /^https?:\/\//i.test(it.url) ? it.url : "#";
			return el("a", {
				class: "hist-item", href: safeUrl, target: "_blank", rel: "noopener"
			}, [
				el("span", { class: "hist-id" }, it.id || "—"),
				el("div", { class: "hist-content" }, [
					el("div", { class: "hist-title" }, it.title || it.url || "ticket"),
					el("div", { class: "hist-meta" },
						[it.portalId || it.portal, formatWhen(it.when)].filter(Boolean).join(" · "))
				]),
				el("span", { style: { color: "var(--text-sub)", display: "inline-flex" } }, svgIcon(I.external))
			]);
		}));
	}

	return el("div", { class: "pad-tight" }, [head, body]);
}
