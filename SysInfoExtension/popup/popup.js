// Popup bootstrap entry. All declarations live in lib/*.js (loaded in
// order by popup.html). Order of bootstrap calls:
//   1. loadSettings — pulls settings + history from storage into state.
//   2. render        — first paint, so the popup isn't blank while
//                      checkStatus is in flight.
//   3. checkStatus   — pings the agent; on success kicks off fetchSysinfo
//                      and re-renders the affected tabs.

(async function init() {
	await loadSettings();
	render();
	checkStatus();
})();
