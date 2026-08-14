// Status tab — agent connection state, version chips, manual retry.

import { state } from "./state.js";
import { t } from "./compat.js";
import { el, svgIcon } from "./dom.js";
import { I } from "./icons.js";
import { statusInfo } from "./render.js";
import { checkStatus } from "./agent.js";

/**
 * Builds the Status tab: the agent's state as a card, a version chip for each side, and a
 * button that pings the agent again.
 * @returns The tab body.
 */
export function renderStatusTab() {
	const sc = statusInfo();

	const dot = el("div", { class: "status-dot-wrap", style: { color: sc.dot } }, [
		el("div", { class: "status-dot", style: { background: sc.dot } }),
		sc.pulse ? el("div", { class: "status-pulse" }) : null
	]);

	const card = el("div", { class: "status-card " + sc.kind }, [
		dot,
		el("div", { style: { flex: "1", minWidth: "0" } }, [
			el("div", { class: "status-label " + sc.kind }, sc.label),
			el("div", { class: "status-sub" }, sc.sub)
		])
	]);

	const agentVer = state.status === "error"
		? "—"
		: (state.agentVersion || "v?");
	const agentVerCls = "val" + (state.status === "version-mismatch" ? " warn" : "");

	// The build stamp shares the version's line so that both chips stand the
	// same height whether or not the agent reported one.
	const agentChip = el("div", { class: "version-chip" }, [
		el("div", { class: "lbl" }, t("agentVersion")),
		el("div", { class: "vals" }, [
			el("span", { class: agentVerCls }, agentVer),
			state.status !== "error" && state.agentBuild
				? el("span", { class: "build" }, " · build " + state.agentBuild)
				: null
		])
	]);

	const extChip = el("div", { class: "version-chip" }, [
		el("div", { class: "lbl" }, t("extVersion")),
		el("div", { class: "vals" }, [el("span", { class: "val" }, "v" + state.extVersion)])
	]);

	const retryBtn = el("button", {
		class: "btn btn-primary",
		onclick: () => { checkStatus(); }
	}, [
		el("span", { style: { display: "inline-flex" } }, svgIcon(I.refresh)),
		el("span", {}, t("retry"))
	]);

	const head = el("div", { class: "section-head" }, [
		el("div", { class: "h" }, t("sectionStatus")),
		el("div", { class: "s" }, t("sectionStatusSub"))
	]);

	return el("div", { class: "pad" }, [
		head,
		card,
		el("div", { class: "version-row" }, [agentChip, extChip]),
		el("div", { class: "action-row" }, [retryBtn])
	]);
}
