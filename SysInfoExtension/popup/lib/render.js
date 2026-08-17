// Top-level render orchestration + shared chrome (theme, header, tabs,
// status info).

import { state, root } from "./state.js";
import { t } from "./compat.js";
import { el, svgIcon } from "./dom.js";
import { I } from "./icons.js";
import { renderStatusTab } from "./tab_status.js";
import { renderInfoTab } from "./tab_info.js";
import { renderHistoryTab } from "./tab_history.js";
import { renderSettingsTab } from "./tab_settings.js";

/**
 * Puts the configured theme's class on the root element, following the OS preference when the
 * setting is "auto".
 */
function applyTheme() {
	const choice = state.settings.theme;
	const dark = choice === "dark" || (choice === "auto" && matchMedia("(prefers-color-scheme: dark)").matches);
	root.className = dark ? "theme-dark" : "theme-light";
}

/**
 * Maps the current agent status onto how it is drawn.
 *
 * The header pill and the status card both read it.
 * @returns A descriptor carrying `kind`, `label`, `sub`, `pulse` and `dot`.
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
 * Rebuilds the whole popup from `state` in one synchronous pass.
 *
 * The single entry point for updating the interface: every helper that mutates state ends with
 * a call here rather than patching the nodes it affects.
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
 * Builds the top bar: the logo, the title pair, and a pill mirroring the agent's state.
 * @returns The header element.
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
			src: "../assets/icons/sysinfo_ext.svg",
			alt: ""
		}),
		el("div", { class: "hdr-text" }, [
			el("div", { class: "hdr-title" }, t("extTitle")),
			el("div", { class: "hdr-sub" }, t("extSubtitle"))
		]),
		el("div", { class: "mini-pill" }, [
			dotWrap,
			el("span", {}, sc.label.split(" ")[0])
		])
	]);
}

/**
 * Builds the tab bar, each button switching `state.tab` and re-rendering.
 * @returns The tab bar element, holding one button per tab.
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
			el("span", { style: { display: "inline-flex" } }, svgIcon(tb.icon)),
			el("span", {}, tb.label)
		])
	));
}
