// The one reading of the stored field-visibility flags, shared by the popup that writes them,
// the popup tab that lists them, and the content script that builds the block from them.

/** The fields the block carries, in the order they are rendered. */
export const FIELD_KEYS = ["hostname", "username", "ip", "lastBootTime"];

/**
 * Maps the stored flags onto the fields in use, filling in every one the stored object omits.
 *
 * The last boot time is stored under `uptime` by builds predating its rename; the current name
 * wins when a stored object carries both. A field the object does not mention is shown, so a
 * profile written before that field existed does not hide it.
 * @param stored - The `fields` object read from storage, or a nullish value when absent.
 * @returns A flag for every key of FIELD_KEYS, in that order, and nothing else.
 */
export function resolveFields(stored) {
	const raw = { ...(stored || {}) };
	if (raw.uptime !== undefined && raw.lastBootTime === undefined) {
		raw.lastBootTime = raw.uptime;
	}

	const fields = {};
	// Only an explicit false hides a field: every other value, an absent key included, leaves it
	// on, so a stored object from an older build cannot blank a line it never knew about.
	for (const key of FIELD_KEYS) {
		fields[key] = raw[key] !== false;
	}
	return fields;
}
