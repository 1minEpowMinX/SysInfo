// Rasterizes the extension master icon into the PNG set referenced by the Chromium manifest,
// the Chrome Web Store listing and the AMO listing. Run it after every edit of the master SVG.

import { readFile, writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";
import sharp from "sharp";

const ICONS = path.join(path.dirname(fileURLToPath(import.meta.url)), "..", "assets", "icons");
const MASTER = path.join(ICONS, "sysinfo_ext.svg");

/**
 * Edge lengths of the generated icons, in device-independent pixels.
 *
 * manifest/base.json names the subset the browsers load and is the source of truth for which size
 * serves which key. The sizes it leaves out are listing artwork for the Chrome Web Store and AMO.
 */
const SIZES = [16, 24, 32, 48, 64, 96, 128, 256, 512];

/** Encoder settings applied to every generated file. */
// palette stays off: quantising the background gradient to 256 entries bands it visibly at
// 256 px and above.
const PNG_OPTIONS = { compressionLevel: 9, adaptiveFiltering: true, palette: false };

/**
 * Rasterizes the master SVG and scales the result to a square of `size` edge length.
 *
 * The SVG is rasterized at its intrinsic 512 px and downsampled with a Lanczos-3 kernel, which
 * supersamples the small sizes instead of aliasing their thin strokes.
 * @param master - Contents of the master SVG file.
 * @param size - Edge length of the requested icon, in pixels.
 * @returns A promise for the encoded PNG.
 */
function renderIcon(master, size) {
	return sharp(master)
		.resize(size, size, { kernel: "lanczos3" })
		.png(PNG_OPTIONS)
		.toBuffer();
}

/**
 * Regenerates every icon in `SIZES` and reports the byte count of each file it writes.
 * @returns A promise that settles once the whole set has been written.
 */
async function main() {
	const master = await readFile(MASTER);
	for (const size of SIZES) {
		const png = await renderIcon(master, size);
		const target = path.join(ICONS, `sysinfo_ext_${size}.png`);
		await writeFile(target, png);
		console.log(`${path.basename(target).padEnd(22)} ${String(png.length).padStart(6)} B`);
	}
}

await main();
