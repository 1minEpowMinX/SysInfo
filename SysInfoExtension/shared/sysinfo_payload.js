// Agent payload parsing — the only place that knows the wire spelling of the
// /systeminfo fields.

// The agent has spelled the last boot time "uptime" since its first release and
// is moving to BOOT_TIME_KEY. Both names are read so that an agent which has
// made the move and one which has not can serve the same extension build: the
// fleet updates over several builds and is never in step.
export const BOOT_TIME_KEY = "lastBootTime";
export const LEGACY_BOOT_TIME_KEY = "uptime";

/**
 * Maps a raw `/systeminfo` payload onto the field names the extension uses internally, resolving
 * the last boot time from whichever of the two wire names the agent sent.
 *
 * A field the payload omits stays undefined rather than being filled in here.
 * @param data - A raw `/systeminfo` payload; a nullish value yields an object of undefined fields.
 * @returns An object carrying `hostname`, `username`, `ip` and `lastBootTime`.
 */
export function normalizeSysInfo(data) {
	const raw = data || {};
	return {
		hostname: raw.hostname,
		username: raw.username,
		ip: raw.ip,
		// ?? and not ||: the legacy name stands in for an absent key alone, so an
		// agent that sends the new key empty is not read as one predating it.
		lastBootTime: raw[BOOT_TIME_KEY] ?? raw[LEGACY_BOOT_TIME_KEY]
	};
}
