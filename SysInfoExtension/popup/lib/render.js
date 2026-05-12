// Top-level render orchestration + shared chrome (theme, header, tabs,
// status info).

import { state, root } from "./state.js";
import { t } from "./compat.js";
import { el } from "./dom.js";
import { I } from "./icons.js";
import { renderStatusTab } from "./tab_status.js";
import { renderInfoTab } from "./tab_info.js";
import { renderHistoryTab } from "./tab_history.js";
import { renderSettingsTab } from "./tab_settings.js";

/**
 * The function `applyTheme` reads `state.settings.theme` and sets `root.className` to either
 * `"theme-dark"` or `"theme-light"`. When the theme is `"auto"` the OS preference is used via
 * `matchMedia`.
 */
function applyTheme() {
	const choice = state.settings.theme;
	const dark = choice === "dark" || (choice === "auto" && matchMedia("(prefers-color-scheme: dark)").matches);
	root.className = dark ? "theme-dark" : "theme-light";
}

/**
 * The function `statusInfo` maps the current `state.status` value to a display descriptor
 * object used by both the mini-pill in the header and the full status card.
 * @returns An object with `kind`, `label`, `sub`, `pulse`, and `dot` properties describing how
 * the current status should be rendered.
 */
export function statusInfo() {
	switch (state.status) {
		case "active": return { kind: "ok", label: t("statusActive"), sub: t("statusActiveSub"), pulse: true, dot: "var(--ok-dot)" };
		case "loading": return { kind: "warn", label: t("statusLoading"), sub: t("statusLoadingSub"), pulse: true, dot: "var(--warn-dot)" };
		case "version-mismatch": return { kind: "warn", label: t("statusVersionMismatch"), sub: t("statusVersionMismatchSub"), pulse: false, dot: "var(--warn-dot)" };
		case "error":
		default: return { kind: "err", label: t("statusError"), sub: t("statusErrorSub"), pulse: false, dot: "var(--err-dot)" };
	}
}

/**
 * The function `render` is the single entry point for updating the popup UI. It applies the
 * current theme, clears `#root`, and rebuilds the full DOM tree — header, tab bar, and the
 * active tab body — from the current `state` in a single synchronous pass.
 */
export function render() {
	applyTheme();
	root.innerHTML = "";
	root.appendChild(renderHeader());
	root.appendChild(renderTabs());
	const body = el("div", { class: "body" });
	if (state.tab === "status") body.appendChild(renderStatusTab());
	else if (state.tab === "info") body.appendChild(renderInfoTab());
	else if (state.tab === "history") body.appendChild(renderHistoryTab());
	else if (state.tab === "settings") body.appendChild(renderSettingsTab());
	root.appendChild(body);
}

/**
 * The function `renderHeader` builds the top bar of the popup containing the extension logo,
 * the title and subtitle, and a mini status pill that mirrors the current agent connection
 * state with an animated dot.
 * @returns A `div.hdr` element representing the popup header.
 */
function renderHeader() {
	const sc = statusInfo();
	const dotWrap = el("div", { class: "dot-wrap", style: { color: sc.dot } }, [
		el("div", { class: "dot", style: { background: sc.dot } }),
		sc.pulse ? el("div", { class: "pulse" }) : null
	]);
	return el("div", { class: "hdr" }, [
		el("img", {
			class: "hdr-logo",
			src: "../assets/SysInfo_Ext_Icon_128.png",
			alt: ""
		}),
		el("div", { class: "hdr-text" }, [
			el("div", { class: "hdr-title" }, t("extTitle")),
			el("div", { class: "hdr-sub" }, t("extSubtitle"))
		]),
		el("div", { class: "mini-pill" }, [
			dotWrap,
			el("span", { style: { fontWeight: "500" } }, sc.label.split(" ")[0])
		])
	]);
}

/**
 * The function `renderTabs` builds the tab navigation bar. Clicking a tab button updates
 * `state.tab` and calls `render()` to switch the active body.
 * @returns A `div.tabs` element containing one button per tab.
 */
function renderTabs() {
	const tabs = [
		{ id: "status", icon: I.status, label: t("tabStatus") },
		{ id: "info", icon: I.data, label: t("tabInfo") },
		{ id: "history", icon: I.history, label: t("tabHistory") },
		{ id: "settings", icon: I.settings, label: t("tabSettings") }
	];
	return el("div", { class: "tabs" }, tabs.map(tb =>
		el("button", {
			class: "tab" + (state.tab === tb.id ? " active" : ""),
			onclick: () => { state.tab = tb.id; render(); }
		}, [
			el("span", { html: tb.icon, style: { display: "inline-flex" } }),
			el("span", {}, tb.label)
		])
	));
}
