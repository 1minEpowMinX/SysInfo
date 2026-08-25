// The one reading of a stored whitelist, shared by the popup that writes the lists and the
// content script that enforces them. Held here because the two used to carry a copy each, and
// they disagreed over the empty list.

/**
 * Reports whether `value` is a stored ID list.
 *
 * An empty array qualifies: a list the user emptied is a list that admits nothing, and reading it
 * as "nothing stored" would widen the whitelist at the moment they closed it. Anything that is
 * not an array of strings is not a list at all, and leaves the built-in defaults in force.
 * @param value - The value read from storage.
 */
export function isIdList(value) {
	return Array.isArray(value) && value.every(v => typeof v === "string");
}
