// History tab — recent tickets where data was attached.
// Entries are written by the content script in lib/history.js after a
// successful insertion + redirect to the created ticket page.

import { state } from "./state.js";
import { t } from "./compat.js";
import { el, formatWhen } from "./dom.js";
import { I } from "./icons.js";

/**
 * The function `renderHistoryTab` builds the History tab body, which shows the most recent
 * tickets (up to 10) where sysinfo was inserted. Each entry is a link that opens the ticket in a
 * new tab. When the history list is empty an empty-state message is rendered instead.
 * @returns A `div.pad-tight` element containing the section heading and the history list or
 * empty-state message.
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
		body = el("div", { class: "hist-list" }, items.slice(0, 10).map(it =>
			el("a", {
				class: "hist-item", href: it.url || "#", target: "_blank", rel: "noopener"
			}, [
				el("span", { class: "hist-id" }, it.id || "—"),
				el("div", { class: "hist-content" }, [
					el("div", { class: "hist-title" }, it.title || it.url || "ticket"),
					el("div", { class: "hist-meta" },
						[it.portalId || it.portal, formatWhen(it.when)].filter(Boolean).join(" · "))
				]),
				el("span", { html: I.external, style: { color: "var(--text-sub)", display: "inline-flex" } })
			])
		));
	}

	return el("div", { class: "pad-tight" }, [head, body]);
}
