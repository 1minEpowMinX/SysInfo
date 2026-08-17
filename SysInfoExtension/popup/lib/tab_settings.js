// Settings tab — theme, sysinfo fields, and the two whitelist lists
// (portal IDs + ticket type IDs).

import { state } from "./state.js";
import { t } from "./compat.js";
import { el, segmented, toggleRow } from "./dom.js";
import { saveSettings } from "./storage.js";
import { render } from "./render.js";

/**
 * Builds the Settings tab: the theme control, the field-visibility toggles, and the two
 * whitelist editors.
 *
 * There is no save button — every change is written to storage as it is made.
 * @returns The tab body.
 */
export function renderSettingsTab() {
	const s = state.settings;

	const themeRow = el("div", { class: "set-row" }, [
		el("span", { class: "lbl" }, t("settingTheme")),
		segmented(s.theme, (v) => { s.theme = v; saveSettings(); render(); }, [
			{ v: "auto", l: t("themeAuto") },
			{ v: "light", l: t("themeLight") },
			{ v: "dark", l: t("themeDark") }
		])
	]);

	const fieldsHead = el("div", { class: "set-section" }, [
		el("div", { class: "h" }, t("settingFields")),
		el("div", { class: "s" }, t("settingFieldsSub"))
	]);
	const fieldsList = el("div", { style: { display: "flex", flexDirection: "column", gap: "4px", marginTop: "8px" } },
		[
			["hostname", t("sysinfoHostname")],
			["username", t("sysinfoUsername")],
			["ip", t("sysinfoIP")],
			["lastBootTime", t("sysinfoLastBootTime")]
		].map(([k, lbl]) => toggleRow(lbl, s.fields[k], (v) => { s.fields[k] = v; saveSettings(); render(); }))
	);

	const portalsHead = el("div", { class: "set-section" }, [
		el("div", { class: "h" }, t("settingPortals")),
		el("div", { class: "s" }, t("settingPortalsSub"))
	]);
	const portalsList = renderIdList(s.portals, {
		addLabel: t("addPortal"),
		promptKey: "promptPortalId",
		dupKey: "portalExists"
	});

	const typesHead = el("div", { class: "set-section" }, [
		el("div", { class: "h" }, t("settingTypes")),
		el("div", { class: "s" }, t("settingTypesSub"))
	]);
	const typesList = renderIdList(s.types, {
		addLabel: t("addType"),
		promptKey: "promptTypeId",
		dupKey: "typeExists"
	});

	return el("div", { class: "pad-tight scroll" }, [
		themeRow,
		fieldsHead, fieldsList,
		portalsHead, portalsList,
		typesHead, typesList
	]);
}

/**
 * Builds an editable list of ID chips, each removable, with a chip that prompts for another.
 * @param items - The array of IDs, edited in place.
 * @param addLabel - Label of the chip that adds an ID.
 * @param promptKey - Message key for the prompt shown when adding.
 * @param dupKey - Message key for the alert shown when the ID is already listed.
 * @returns The list element.
 */
function renderIdList(items, { addLabel, promptKey, dupKey }) {
	return el("div", { class: "id-list" }, [
		...items.map((id, idx) =>
			el("span", { class: "chip chip-removable" }, [
				el("span", {}, id),
				el("button", {
					class: "chip-x",
					title: "×",
					onclick: (e) => {
						e.stopPropagation();
						items.splice(idx, 1);
						saveSettings();
						render();
					}
				}, "×")
			])
		),
		el("button", {
			class: "chip add",
			onclick: () => {
				const v = prompt(t(promptKey));
				if (!v) return;
				// Both lists are matched against the digit groups of FORM_PATH_RE, so a
				// non-numeric entry could never match a URL and is dropped without a word.
				const trimmed = v.trim().slice(0, 20);
				if (!trimmed || !/^\d+$/.test(trimmed)) return;
				if (items.includes(trimmed)) {
					alert(t(dupKey));
					return;
				}
				items.push(trimmed);
				saveSettings();
				render();
			}
		}, addLabel)
	]);
}
