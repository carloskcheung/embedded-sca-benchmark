#!/usr/bin/env python3
"""Score a scanner's SBOM against benchmark/expected-components.yaml.

Accepts CycloneDX JSON or SPDX 2.x JSON (a single document, or a directory of
SPDX documents such as the Yocto create-spdx output). For each ground-truth
component it reports whether it was found and whether version and license
were identified correctly.

    python3 benchmark/score.py fossa-export.cdx.json
    python3 benchmark/score.py --scope yocto benchmark/sbom/spdx/
    python3 benchmark/score.py --format json export.spdx.json > result.json

Requires PyYAML.
"""
import argparse
import json
import re
import sys
from pathlib import Path

import yaml

GROUND_TRUTH = Path(__file__).with_name("expected-components.yaml")
SPDX_OPERATORS = {"AND", "OR", "WITH"}


def norm_name(s):
    return re.sub(r"[^a-z0-9]", "", s.lower())


def norm_version(s):
    s = (s or "").strip().lower()
    s = re.sub(r"^(v|release-|stable-)", "", s)
    s = s.replace("_release", "").replace("_", ".")
    return s


def license_ids(expr):
    if not expr:
        return set()
    tokens = re.findall(r"[A-Za-z0-9.\-+:]+", expr.replace("|", " OR ").replace("&", " AND "))
    return {t for t in tokens if t.upper() not in SPDX_OPERATORS and t != "NOASSERTION"}


def yocto_ids(ids):
    # create-spdx emits non-SPDX Yocto names as [DocumentRef-x:]LicenseRef-<name>
    return {re.sub(r"^(DocumentRef-[^:]+:)?LicenseRef-", "", i) for i in ids}


# --- SBOM readers -------------------------------------------------------------

def read_cyclonedx(doc):
    for c in doc.get("components", []):
        lic = []
        for entry in c.get("licenses", []):
            if "expression" in entry:
                lic.append(entry["expression"])
            elif "license" in entry:
                lic.append(entry["license"].get("id") or entry["license"].get("name", ""))
        yield {
            "name": c.get("name", ""),
            "version": c.get("version", ""),
            "purl": c.get("purl", ""),
            "license": " AND ".join(x for x in lic if x),
        }


def read_spdx(doc):
    for p in doc.get("packages", []):
        purl = ""
        for ref in p.get("externalRefs", []):
            if ref.get("referenceType") == "purl":
                purl = ref.get("referenceLocator", "")
        lic = p.get("licenseConcluded") or ""
        if lic in ("", "NOASSERTION"):
            lic = p.get("licenseDeclared") or ""
        yield {
            "name": p.get("name", ""),
            "version": p.get("versionInfo", ""),
            "purl": purl,
            "license": lic,
        }


def load_reported(path):
    path = Path(path)
    files = sorted(path.rglob("*.json")) if path.is_dir() else [path]
    found = []
    for f in files:
        try:
            doc = json.loads(f.read_text())
        except (json.JSONDecodeError, UnicodeDecodeError):
            continue
        if doc.get("bomFormat") == "CycloneDX":
            found.extend(read_cyclonedx(doc))
        elif "spdxVersion" in doc:
            found.extend(read_spdx(doc))
    return found


# --- scoring ------------------------------------------------------------------

def candidates(expected, reported):
    keys = {norm_name(a) for a in [expected["name"], *expected.get("aliases", [])]}
    for r in reported:
        purl_name = norm_name(r["purl"].split("@")[0].rsplit("/", 1)[-1]) if r["purl"] else ""
        if norm_name(r["name"]) in keys or (purl_name and purl_name in keys):
            yield r


def version_ok(expected, got):
    want, have = norm_version(str(expected)), norm_version(got)
    if not have:
        return False
    if re.fullmatch(r"[0-9a-f]{40}", want):  # pinned commit
        return len(have) >= 7 and want.startswith(have)
    return want == have or have.startswith(want + ".") or have.startswith(want + "+") or have.startswith(want + "-")


def license_status(expected, got):
    spdx = license_ids(expected["license"])
    yocto = license_ids(expected.get("yocto_license"))
    have = license_ids(got)
    if not have:
        return "missing"
    if have == spdx:
        return "ok"
    if yocto and yocto_ids(have) == yocto:
        return "yocto-name"  # right license, but left in Yocto's naming
    return "partial" if yocto_ids(have) & (spdx | yocto) else "wrong"


def score(expected_components, reported):
    rows = []
    for exp in expected_components:
        cands = list(candidates(exp, reported))
        best = None
        for c in cands:
            if version_ok(exp["version"], c["version"]):
                best = c
                break
        best = best or (cands[0] if cands else None)
        rows.append({
            "component": exp["name"],
            "scope": exp["scope"],
            "inclusion": exp["inclusion"],
            "expected_version": str(exp["version"]),
            "found": best is not None,
            "reported_name": best["name"] if best else "",
            "reported_version": best["version"] if best else "",
            "version_ok": bool(best) and version_ok(exp["version"], best["version"]),
            "license": license_status(exp, best["license"]) if best else "n/a",
        })
    return rows


def print_markdown(rows):
    mark = lambda b: "yes" if b else "NO"
    print("| component | scope | inclusion | found | version | license |")
    print("|---|---|---|---|---|---|")
    for r in rows:
        ver = f"{mark(r['version_ok'])} ({r['reported_version'] or '-'})" if r["found"] else "-"
        print(f"| {r['component']} | {r['scope']} | {r['inclusion']} | {mark(r['found'])} "
              f"| {ver} | {r['license']} |")
    n = len(rows)
    found = sum(r["found"] for r in rows)
    ver = sum(r["version_ok"] for r in rows)
    lic = sum(r["license"] == "ok" for r in rows)
    yn = sum(r["license"] == "yocto-name" for r in rows)
    print(f"\nfound {found}/{n} · correct version {ver}/{n} · correct license {lic}/{n}"
          + (f" (+{yn} correct but not mapped to SPDX)" if yn else ""))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("sbom", help="CycloneDX/SPDX JSON file, or directory of SPDX JSON files")
    ap.add_argument("--scope", choices=["sensor-node", "ble-controller", "yocto"],
                    help="only score components from this part of the product")
    ap.add_argument("--format", choices=["markdown", "json"], default="markdown")
    args = ap.parse_args()

    truth = yaml.safe_load(GROUND_TRUTH.read_text())["components"]
    if args.scope:
        truth = [c for c in truth if c["scope"] == args.scope]
    reported = load_reported(args.sbom)
    if not reported:
        sys.exit(f"no CycloneDX or SPDX components found in {args.sbom}")

    rows = score(truth, reported)
    if args.format == "json":
        json.dump(rows, sys.stdout, indent=2)
        print()
    else:
        print_markdown(rows)


if __name__ == "__main__":
    main()
