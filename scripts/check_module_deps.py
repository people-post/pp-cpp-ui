#!/usr/bin/env python3
"""Check pp-cpp-ui module include edges against ADR 002.

Exit 0 when every forbidden edge is in DEBT_ALLOWLIST (or there are none).
Exit 1 when a new forbidden edge appears, or an allowlisted edge is gone
(use --relax-allowlist to only fail on new edges).

Usage:
  python3 scripts/check_module_deps.py
  python3 scripts/check_module_deps.py --relax-allowlist
  python3 scripts/check_module_deps.py --list-debt
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MODULES = (
    "config",
    "base",
    "paint",
    "style",
    "layout",
    "text",
    "dom",
    "xml",
    "data",
    "widgets",
    "core",
    "svg",
    "debugger",
    "platform",
    "render",
)

# Higher number may depend on lower. Never reverse unless allowlisted or bridged.
LAYER = {
    "config": 0,
    "base": 1,
    "paint": 2,
    "style": 3,
    "layout": 4,
    "text": 5,
    "dom": 6,
    "xml": 7,
    "data": 8,
    "widgets": 9,
    "core": 10,
    "svg": 11,
    "debugger": 11,
    "platform": 12,
    "render": 12,
}

# Explicit upward edges permitted by ADR 002.
ALLOWED_BRIDGES = {
    ("layout", "dom"),
    ("text", "dom"),
    # Composition root may register plugins.
    ("core", "svg"),
    ("core", "debugger"),
}

# Modules allowed to include core.
CORE_CONSUMERS = {"core", "svg", "debugger", "platform", "render"}

# Tracked debt from the first ADR 002 baseline (module pair → reason).
# Shrink this set; do not grow it without updating ADR 002.
DEBT_ALLOWLIST: dict[tuple[str, str], str] = {
    ("base", "core"): "Clock/Log use Core::GetSystemInterface",
    ("paint", "layout"): "Mesh/geometry helpers take Box",
    ("paint", "text"): "MeshUtilities uses FontEngineInterface",
    ("paint", "dom"): "GeometryBoxShadow uses Element",
    ("paint", "core"): "RenderManager uses Core/SystemInterface",
    ("style", "dom"): "StyleSheet/transforms touch Element",
    ("style", "text"): "font-effect / DecoratorText",
    ("style", "xml"): "stylesheet parsers use Stream*",
    ("layout", "text"): "inline layout uses ElementText / fonts",
    ("layout", "core"): "formatting contexts use SystemInterface/Core",
    ("text", "widgets"): "ElementText/SelectionController ↔ ElementSelectableText",
    ("text", "core"): "ElementText/SelectionController use Core",
    ("dom", "xml"): "Document/Context load via Stream*",
    ("dom", "data"): "Context/Element hold DataModel",
    ("dom", "widgets"): "Factory/scroll concrete widgets",
    ("dom", "core"): "widespread Core::Get*()",
    ("xml", "core"): "handlers/StreamFile use Core",
    ("data", "core"): "DataViewDefault uses Core",
    ("widgets", "core"): "inputs/textarea use Core",
}

INC_RE = re.compile(r'#\s*include\s+[<"]([^>"]+)[>"]')


def module_of(path: Path) -> str | None:
    text = path.as_posix()
    for prefix in ("/include/ui/", "/src/"):
        if prefix in text:
            rest = text.split(prefix, 1)[1]
            top = rest.split("/", 1)[0]
            if top in MODULES:
                return top
    return None


def target_module(include: str) -> str | None:
    m = re.match(r"ui/([A-Za-z0-9_]+)/", include)
    if m:
        name = m.group(1).lower()
        return name if name in MODULES else None
    m = re.match(r"([a-z]+)/", include)
    if m and m.group(1) in MODULES and "/" in include:
        return m.group(1)
    return None


def collect_edges() -> dict[tuple[str, str], list[tuple[str, str]]]:
    edges: dict[tuple[str, str], list[tuple[str, str]]] = defaultdict(list)
    for root_name in ("src", "include/ui"):
        root = ROOT / root_name
        if not root.exists():
            continue
        for dirpath, _, files in os.walk(root):
            for name in files:
                if not name.endswith((".h", ".hpp", ".cpp", ".inl")):
                    continue
                path = Path(dirpath) / name
                src = module_of(path.resolve())
                if not src:
                    continue
                try:
                    text = path.read_text(encoding="utf-8", errors="ignore")
                except OSError:
                    continue
                rel = path.relative_to(ROOT).as_posix()
                for match in INC_RE.finditer(text):
                    inc = match.group(1)
                    tgt = target_module(inc)
                    if not tgt or tgt == src:
                        continue
                    edges[(src, tgt)].append((rel, inc))
    return edges


def classify(src: str, tgt: str) -> str | None:
    """Return None if allowed; otherwise a short reason string."""
    if (src, tgt) in ALLOWED_BRIDGES:
        return None
    if tgt == "core" and src not in CORE_CONSUMERS:
        return "non-plugin/backend must not include core"
    if LAYER[src] == LAYER[tgt] and src != tgt:
        return "same-layer peer include"
    if LAYER[src] < LAYER[tgt]:
        return f"upward L{LAYER[src]}→L{LAYER[tgt]}"
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--relax-allowlist",
        action="store_true",
        help="Do not fail when an allowlisted debt edge disappears",
    )
    parser.add_argument(
        "--list-debt",
        action="store_true",
        help="Print allowlisted debt hits and exit 0",
    )
    args = parser.parse_args()

    edges = collect_edges()
    forbidden: dict[tuple[str, str], str] = {}
    for (src, tgt), samples in sorted(edges.items()):
        reason = classify(src, tgt)
        if reason:
            forbidden[(src, tgt)] = reason

    debt_hits = {k: v for k, v in forbidden.items() if k in DEBT_ALLOWLIST}
    new_violations = {k: v for k, v in forbidden.items() if k not in DEBT_ALLOWLIST}
    stale_allow = {k: v for k, v in DEBT_ALLOWLIST.items() if k not in forbidden}

    if args.list_debt:
        print("Allowlisted debt edges present in tree:")
        for (src, tgt), reason in sorted(debt_hits.items()):
            n = len(edges[(src, tgt)])
            print(f"  {src:10} → {tgt:10}  ({n:3} includes)  {DEBT_ALLOWLIST[(src, tgt)]}")
        print(f"\n{len(debt_hits)} / {len(DEBT_ALLOWLIST)} allowlisted pairs still present")
        if stale_allow:
            print("Cleared (still in allowlist — remove entries):")
            for (src, tgt), note in sorted(stale_allow.items()):
                print(f"  {src} → {tgt}  ({note})")
        return 0

    print("pp-cpp-ui module dependency check (ADR 002)")
    print(f"  scanned edges:     {sum(len(v) for v in edges.values())}")
    print(f"  forbidden pairs:   {len(forbidden)}")
    print(f"  allowlisted debt:  {len(debt_hits)}")
    print(f"  new violations:    {len(new_violations)}")
    print(f"  stale allowlist:   {len(stale_allow)}")

    if debt_hits:
        print("\nTracked debt:")
        for (src, tgt), reason in sorted(debt_hits.items()):
            n = len(edges[(src, tgt)])
            print(f"  DEBT  {src} → {tgt}  ({n})  [{reason}] {DEBT_ALLOWLIST[(src, tgt)]}")

    status = 0
    if new_violations:
        status = 1
        print("\nNEW forbidden edges (not in allowlist):")
        for (src, tgt), reason in sorted(new_violations.items()):
            print(f"  FAIL  {src} → {tgt}  [{reason}]")
            for rel, inc in edges[(src, tgt)][:5]:
                print(f"        {rel}: {inc}")
            if len(edges[(src, tgt)]) > 5:
                print(f"        … +{len(edges[(src, tgt)]) - 5} more")

    if stale_allow and not args.relax_allowlist:
        status = 1
        print("\nAllowlist entries with no remaining includes (remove from DEBT_ALLOWLIST):")
        for (src, tgt), note in sorted(stale_allow.items()):
            print(f"  STALE {src} → {tgt}  ({note})")

    if status == 0:
        print("\nOK — no new forbidden edges")
    return status


if __name__ == "__main__":
    sys.exit(main())
