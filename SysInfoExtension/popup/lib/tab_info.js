// Data tab — current sysinfo values from the agent + per-row copy buttons.
// Rows are filtered by state.settings.fields so the user can hide fields.

import { state } from "./state.js";
import { t } from "./compat.js";
import { el, svgIcon } from "./dom.js";
import { I } from "./icons.js";
import { fetchSysinfo, copyValue } from "./agent.js";

/**
 * The function `renderInfoTab` builds the Data tab body, which lists the current sysinfo fields
 * filtered by `state.settings.fields`. Each row shows an icon, label, value, and a copy button
 * that briefly switches to a checkmark after a successful clipboard write. A refresh button
 * triggers a new `fetchSysinfo` call.
 * @returns A `div.pad-tight` element containing the section heading, field rows, and refresh
 * action.
 */
export function renderInfoTab() {
	const sysinfo = state.sysinfo;
	const isError = state.status === "error" || !sysinfo;
	const fields = state.settings.fields;
	const rows = [
		{ key: "hostname", label: t("sysinfoHostname"), icon: I.device, value: sysinfo?.hostname || "—" },
		{ key: "username", label: t("sysinfoUsername"), icon: I.user, value: sysinfo?.username || "—" },
		{ key: "ip", label: t("sysinfoIP"), icon: I.ip, value: sysinfo?.ip || "—" },
		{ key: "lastBootTime", label: t("sysinfoLastBootTime"), icon: I.lastBootTime, value: sysinfo?.lastBootTime || "—" }
	].filter(r => fields[r.key]);

	const head = el("div", { class: "info-head" }, [
		el("div", { class: "h" }, t("sectionSysinfo")),
		el("div", { class: "s" }, t("sectionSysinfoSub"))
	]);

	const list = el("div", {}, rows.map(r => {
		const isCopied = state.copied === r.key;
		return el("div", { class: "info-row" + (isError ? " disabled" : "") }, [
			el("div", { class: "info-icon" }, svgIcon(r.icon)),
			el("div", { class: "info-content" }, [
				el("div", { class: "info-label" }, r.label),
				el("div", { class: "info-value" }, r.value)
			]),
			el("button", {
				class: "copy-btn" + (isCopied ? " copied" : ""),
				title: isCopied ? t("copied") : t("copy"),
				onclick: () => !isError && copyValue(r.key, r.value),
			}, svgIcon(isCopied ? I.check : I.copy))
		]);
	}));

	const refreshBtn = el("button", {
		class: "btn btn-primary",
		onclick: () => { fetchSysinfo(); }
	}, [
		el("span", { style: { display: "inline-flex" } }, svgIcon(I.refresh)),
		el("span", {}, t("refresh"))
	]);

	return el("div", { class: "pad-tight", style: { padding: "12px 8px 8px" } }, [
		head,
		list,
		el("div", { class: "divider info-actions" }, [refreshBtn])
	]);
}
