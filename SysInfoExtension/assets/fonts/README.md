# Bundled fonts

JetBrains Mono, licensed under the SIL Open Font License 1.1. The copyright
notice, the licence statement and its URL are carried in each file's `name`
table (IDs 0, 13 and 14) and can be read with any font inspector:

```bash
python -c "from fontTools.ttLib import TTFont; print(TTFont('JetBrainsMono-Regular.woff2')['name'].getDebugName(13))"
```

Upstream: https://github.com/JetBrains/JetBrainsMono

The `.woff2` files here are subsets, not the shipped originals. Regenerate them
with `tools/subset-fonts.py`, which owns the list of faces and the kept
character ranges.
