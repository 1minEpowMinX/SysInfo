"""Build the popup's bundled JetBrains Mono subsets.

Reads the installed JetBrains Mono static faces and writes woff2 subsets into
assets/fonts/. Run after changing which characters the monospace elements can
display.

Requires fonttools and brotli:

    pip install fonttools brotli
"""

import sys
from pathlib import Path

from fontTools.subset import Options, Subsetter, load_font, save_font

# Faces are keyed by the CSS font-weight they are declared under in popup.css.
FACES = {400: "JetBrainsMono-Regular.ttf", 600: "JetBrainsMono-SemiBold.ttf"}

DEFAULT_SOURCE = Path("C:/Windows/Fonts")
OUT_DIR = Path(__file__).resolve().parent.parent / "assets" / "fonts"

# Kept ranges. Cyrillic is included because ticket identifiers reaching
# .hist-id are free-form: a character outside the subset falls back to another
# family for that glyph alone, which is worse than shipping the extra glyphs.
UNICODES = [
	(0x0020, 0x007E),  # basic latin
	(0x00A0, 0x00A0),
	(0x00AB, 0x00AB),
	(0x00BB, 0x00BB),
	(0x00B7, 0x00B7),
	(0x00D7, 0x00D7),  # the multiplication sign drawn by .chip-x
	(0x0400, 0x04FF),  # cyrillic
	(0x2010, 0x2015),  # hyphens and dashes, including the em dash placeholder
	(0x2018, 0x201D),
	(0x2026, 0x2026),
	(0x2116, 0x2116),
]


def build(source_dir):
	"""Write one woff2 subset per face and return their sizes in bytes."""
	OUT_DIR.mkdir(parents=True, exist_ok=True)
	sizes = {}

	for weight, filename in FACES.items():
		src = source_dir / filename
		if not src.is_file():
			raise SystemExit(f"missing source face: {src}")

		options = Options()
		options.flavor = "woff2"
		# The OFL is satisfied through the font's own metadata, so every name
		# record has to survive subsetting — including 0, 13 and 14.
		options.name_IDs = ["*"]
		options.name_legacy = True
		options.notdef_outline = True
		# Text renders at 10-13px on Windows, where the shipped ttfautohint
		# instructions still make a visible difference.
		options.hinting = True

		font = load_font(str(src), options)
		subsetter = Subsetter(options=options)
		subsetter.populate(unicodes=[c for lo, hi in UNICODES for c in range(lo, hi + 1)])
		subsetter.subset(font)

		out = OUT_DIR / (src.stem + ".woff2")
		save_font(font, str(out), options)
		font.close()
		sizes[filename] = out.stat().st_size

	return sizes


if __name__ == "__main__":
	source = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_SOURCE
	for name, size in build(source).items():
		print(f"{name} -> {size / 1024:.1f} KB")
