// Settings tab — theme, sysinfo fields, and the two whitelist lists
// (portal IDs + ticket type IDs).

/**
 * The function `renderSettingsTab` builds the Settings tab body, which contains a theme
 * segmented control, a group of field visibility toggles, and two editable chip lists for
 * allowed portal IDs and ticket type IDs. Every change is persisted immediately via
 * `saveSettings` and the popup is re-rendered.
 * @returns A `div.pad-tight.scroll` element containing all settings sections.
 */
function renderSettingsTab() {
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
			["uptime", t("sysinfoUptime")]
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

// Generic chip list for plain string IDs (portals + types share the same
// shape, so the editor is shared too).
/**
 * The function `renderIdList` builds a generic chip list for a flat array of string IDs. Each
 * chip shows the ID and an inline remove button. An add button opens a `prompt` dialog; the new
 * ID is rejected silently if empty or with an `alert` if it is already in the list.
 * @param items - The mutable array of string IDs to display and edit in place.
 * @param addLabel - Label text for the add button chip.
 * @param promptKey - i18n key for the `prompt` dialog message shown when adding a new ID.
 * @param dupKey - i18n key for the `alert` message shown when the entered ID already exists.
 * @returns A `div.id-list` element containing the existing ID chips and the add button.
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
				const trimmed = v.trim();
				if (!trimmed) return;
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
