"""Render THIRD-PARTY-NOTICES.txt from the SPDX documents Qt publishes.

Qt ships one SPDX document per module under <Qt>/sbom. Each names the binaries
it produces, the components bundled inside them, and the copyright and license
of each. This walks that graph from the DLLs a built delivery actually contains
and renders the notice from what it reaches, so the notice describes the
archive rather than somebody's recollection of it.

Standard SPDX license texts come from licenses/ beside this file. A license the
corpus does not carry aborts the run: a notice missing a text is a notice that
does not satisfy the attribution it claims to, and failing is the only way that
stays visible.

Usage:
    python tools/notices/generate.py --delivery <dir> [--qt <dir>] [--output <file>]

The delivery directory is a built application directory, after windeployqt has
populated it. Run this again whenever Qt is upgraded or the set of linked
modules changes.
"""

import argparse
import collections
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
LICENSE_DIR = os.path.join(HERE, "licenses")

# Directories of a build tree that hold intermediates rather than delivery.
SKIP_DIRS = ("CMakeFiles", "Testing", "_autogen")

# Splits a license expression into the identifiers it names. SPDX joins them
# with AND, OR and WITH, and parenthesises groups.
EXPRESSION_SPLIT = re.compile(r"\b(?:AND|OR|WITH)\b|[()]")


def load_documents(sbom_dir):
    """Read every SPDX document in the directory, keyed by its namespace."""
    docs = {}
    for path in glob.glob(os.path.join(sbom_dir, "*.spdx.json")):
        with open(path, encoding="utf-8") as handle:
            doc = json.load(handle)
        docs[doc["documentNamespace"]] = doc
    if not docs:
        sys.exit(f"no SPDX documents under {sbom_dir}")
    return docs


class Graph:
    """Index the packages, files and relationships of every loaded document."""

    def __init__(self, docs):
        self.docs = docs
        self.package = {}       # (namespace, spdxid) -> package
        self.by_name = {}       # (namespace, name) -> spdxid
        self.file_name = {}     # (namespace, spdxid) -> path as built
        self.edges = collections.defaultdict(list)
        self.external = {}      # namespace -> {external id -> namespace}
        self.license_text = {}  # LicenseRef id -> text

        for namespace, doc in docs.items():
            for package in doc.get("packages", []):
                self.package[(namespace, package["SPDXID"])] = package
                self.by_name[(namespace, package["name"])] = package["SPDXID"]
            for entry in doc.get("files", []):
                self.file_name[(namespace, entry["SPDXID"])] = entry["fileName"]
            for relation in doc.get("relationships", []):
                self.edges[(namespace, relation["spdxElementId"])].append(
                    (relation["relationshipType"], namespace, relation["relatedSpdxElement"])
                )
            self.external[namespace] = {
                ref["externalDocumentId"]: ref["spdxDocument"]
                for ref in doc.get("externalDocumentRefs", [])
            }
            for info in doc.get("hasExtractedLicensingInfos", []):
                self.license_text.setdefault(info["licenseId"], info.get("extractedText", ""))

    def resolve(self, namespace, reference):
        """Return the (namespace, spdxid) a relationship points at, or None.

        A reference carrying a document prefix crosses into another module's
        document; one without is local. Attribution targets are named rather
        than referenced by id, so a name lookup is the fallback.
        """
        if ":" in reference:
            document_id, spdxid = reference.split(":", 1)
            target = self.external.get(namespace, {}).get(document_id)
            if target and (target, spdxid) in self.package:
                return target, spdxid
            return None
        if (namespace, reference) in self.package:
            return namespace, reference
        if (namespace, reference) in self.by_name:
            return namespace, self.by_name[(namespace, reference)]
        return None

    def owners_by_file(self):
        """Map each built file's base name to the package that contains it."""
        owners = {}
        for (namespace, spdxid), relations in self.edges.items():
            for kind, _, reference in relations:
                if kind != "CONTAINS":
                    continue
                path = self.file_name.get((namespace, reference))
                if path:
                    owners[os.path.basename(path).lower()] = (namespace, spdxid)
        return owners

    def attributions_under(self, root, shipped_elsewhere):
        """Return the third-party packages bundled into one delivered library.

        Walks CONTAINS and DEPENDS_ON across documents, but prunes any branch
        reaching a package that produces a delivered library of its own: a
        component inside Qt6Core.dll is not also inside every plugin that links
        it, and listing it against all of them says the opposite. Internal
        targets that produce no library are descended into, because whatever
        they bundle ends up in the library that links them.

        Descent also stops at an attribution: what a bundled component itself
        depends on is Qt's business, not this delivery's.
        """
        found, seen, stack = {}, set(), [root]
        while stack:
            current = stack.pop()
            if current in seen:
                continue
            seen.add(current)
            package = self.package.get(current)
            if package is None:
                continue
            if "_Attribution_" in package["name"]:
                found[package["name"]] = package
                continue
            if current != root and current in shipped_elsewhere:
                continue
            for kind, namespace, reference in self.edges.get(current, []):
                if kind not in ("CONTAINS", "DEPENDS_ON"):
                    continue
                target = self.resolve(namespace, reference)
                if target:
                    stack.append(target)
        return found


def delivered_libraries(delivery_dir):
    """Return the base names of the shared libraries a delivery directory holds."""
    names = set()
    for root, _, files in os.walk(delivery_dir):
        if any(part in root for part in SKIP_DIRS):
            continue
        for name in files:
            if name.lower().endswith((".dll", ".so", ".dylib")):
                names.add(name)
    if not names:
        sys.exit(f"no shared libraries under {delivery_dir}; is it a built delivery?")
    return sorted(names)


def license_ids(expression):
    """Return the individual identifiers an SPDX expression names."""
    parts = (part.strip() for part in EXPRESSION_SPLIT.split(expression))
    return [part for part in parts if part and part != "NOASSERTION"]


def license_texts(expressions, extracted):
    """Return the text of every identifier the expressions name.

    Exits naming the identifiers whose text is absent, rather than rendering a
    notice that claims an attribution it does not carry.
    """
    wanted = sorted({name for expression in expressions for name in license_ids(expression)})
    texts, missing = {}, []
    for name in wanted:
        if name in extracted:
            texts[name] = extracted[name].strip()
            continue
        path = os.path.join(LICENSE_DIR, name + ".txt")
        if os.path.exists(path):
            with open(path, encoding="utf-8") as handle:
                texts[name] = handle.read().strip()
        else:
            missing.append(name)
    if missing:
        sys.exit(
            "no license text for: " + ", ".join(missing)
            + f"\nAdd the SPDX text as {LICENSE_DIR}{os.sep}<identifier>.txt and run again."
        )
    return texts


def qt_version(sbom_dir):
    """Return the Qt version the SPDX documents were produced for."""
    for path in glob.glob(os.path.join(sbom_dir, "qtbase-*.spdx.json")):
        match = re.search(r"qtbase-(.+)\.spdx\.json$", os.path.basename(path))
        if match:
            return match.group(1)
    return "unknown"


def collect(graph, delivery_dir):
    """Return the components the delivery reaches, and the libraries it does not explain.

    A component reached through more than one library is one entry carrying
    every library and Qt target it was reached through.
    """
    owners = graph.owners_by_file()
    libraries = delivered_libraries(delivery_dir)
    # Every package that produces a library of this delivery. A walk that
    # reaches one of these has left the library it started from.
    shipped = {owners[name.lower()] for name in libraries if name.lower() in owners}

    components, unexplained = {}, []
    for library in libraries:
        owner = owners.get(library.lower())
        if owner is None:
            unexplained.append(library)
            continue
        owning_name = graph.package[owner]["name"]
        for name, package in graph.attributions_under(owner, shipped).items():
            target, _, component = name.partition("_Attribution_")
            key = (component, package.get("versionInfo", ""))
            entry = components.setdefault(key, {
                "package": package,
                "targets": set(),
                "libraries": set(),
            })
            entry["targets"].add(target or owning_name)
            entry["libraries"].add(library)
    return components, unexplained


def render(components, unexplained, texts, version):
    """Return the notice as text."""
    lines = [
        "SysInfo - Third-Party Notices",
        "=============================",
        "",
        "Generated by tools/notices/generate.py from the SPDX documents published",
        "with Qt " + version + ". Do not edit by hand.",
        "",
        "Every component below is one Qt attributes to a library this delivery",
        "ships, under a license of its own rather than under the license of that",
        "library. The list follows Qt's own attribution data and so also carries a",
        "few components that are headers, specifications or build modules rather",
        "than linked code. The full text of each license follows the list.",
        "",
        "SysInfo's own license and the licenses of the Qt libraries themselves are",
        "not repeated here; see README.txt.",
        "",
    ]

    lines += ["-" * 79, "Components", "-" * 79, ""]
    for (component, component_version), entry in sorted(components.items()):
        package = entry["package"]
        lines.append(f"{component}" + (f" {component_version}" if component_version else ""))
        lines.append(f"    License:   {package.get('licenseConcluded', 'NOASSERTION')}")
        lines.append(f"    Built into: {', '.join(sorted(entry['targets']))}")
        lines.append(f"    Delivered in: {', '.join(sorted(entry['libraries']))}")
        upstream = package.get("downloadLocation", "NOASSERTION")
        if upstream and upstream != "NOASSERTION":
            lines.append(f"    Upstream:  {upstream}")
        copyright_text = (package.get("copyrightText") or "").strip()
        if copyright_text and copyright_text != "NOASSERTION":
            lines.append("    Copyright:")
            lines += [f"        {line}" for line in copyright_text.splitlines()]
        lines.append("")

    if unexplained:
        lines += [
            "-" * 79,
            "Libraries outside the Qt documents",
            "-" * 79,
            "",
            "The delivery carries these, and no Qt SPDX document accounts for them.",
            "They are covered by their own section of README.txt.",
            "",
        ]
        lines += [f"    {name}" for name in unexplained]
        lines.append("")

    lines += ["-" * 79, "License texts", "-" * 79, ""]
    for name in sorted(texts):
        lines += [f"=== {name} ===", "", texts[name], ""]

    return "\n".join(lines).rstrip() + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--delivery", required=True,
                        help="built application directory, after windeployqt")
    parser.add_argument("--qt", default=os.environ.get("QTDIR"),
                        help="Qt installation directory (default: $QTDIR)")
    parser.add_argument("--output", default=os.path.join("docs", "THIRD-PARTY-NOTICES.txt"))
    args = parser.parse_args()

    if not args.qt:
        sys.exit("name the Qt installation with --qt or set QTDIR")
    sbom_dir = os.path.join(args.qt, "sbom")
    if not os.path.isdir(sbom_dir):
        sys.exit(f"{sbom_dir} does not exist; this Qt installation ships no SPDX documents")

    graph = Graph(load_documents(sbom_dir))
    components, unexplained = collect(graph, args.delivery)
    expressions = [entry["package"].get("licenseConcluded", "") for entry in components.values()]
    texts = license_texts(expressions, graph.license_text)

    notice = render(components, unexplained, texts, qt_version(sbom_dir))
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(notice)

    print(f"{args.output}: {len(components)} components, {len(texts)} license texts")
    if unexplained:
        print("not accounted for by Qt: " + ", ".join(unexplained))


if __name__ == "__main__":
    main()
