#!/usr/bin/env python3
"""Qualify cross-module private includes and switch to a single src/ -I root."""
from __future__ import annotations

import re
import subprocess
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

SKIP_PARTS = {".git", "third_party", "build", "build-inc", "build-reorg"}


def rel_from_src(path: Path) -> str:
    return path.relative_to(SRC).as_posix()


def main() -> None:
    # Index private headers: basename -> list of paths relative to src/
    by_base: dict[str, list[str]] = defaultdict(list)
    for p in SRC.rglob("*"):
        if p.suffix in {".h", ".hpp", ".inl"} and p.is_file():
            by_base[p.name].append(rel_from_src(p))

    # Also index by unique relative path for existence checks
    all_rels = {rel for rels in by_base.values() for rel in rels}

    # --- SelectionController: already public under include/ui/dom/ ---
    sc_priv = SRC / "font" / "SelectionController.h"
    if sc_priv.is_file():
        priv = sc_priv.read_text(encoding="utf-8")
        pub = priv.replace('#include "SelectionTypes.h"', "#include <ui/dom/SelectionTypes.h>")
        if "UI_CORE_API" not in pub and "class SelectionController" in pub:
            pub = pub.replace("class SelectionController", "class UI_CORE_API SelectionController")
        if "#include <ui/base/Header.h>" not in pub:
            pub = pub.replace("#pragma once", "#pragma once\n\n#include <ui/base/Header.h>", 1)
        (ROOT / "include" / "ui" / "dom" / "SelectionController.h").write_text(pub, encoding="utf-8")
        sc_priv.write_text("#pragma once\n\n#include <ui/dom/SelectionController.h>\n", encoding="utf-8")
        print("promoted SelectionController to public header")

    include_re = re.compile(r'^(\s*#include\s*)"([^"]+)"(.*)$')

    def qualify_file(path: Path) -> bool:
        text = path.read_text(encoding="utf-8")
        file_dir = path.parent
        try:
            file_rel_dir = file_dir.relative_to(SRC).as_posix()
        except ValueError:
            return False

        changed = False
        out_lines: list[str] = []
        for line in text.splitlines():
            m = include_re.match(line)
            if not m:
                out_lines.append(line)
                continue
            prefix, inc, rest = m.groups()
            # Already qualified with a slash — if it's a path under src, keep; if old Layout/ style, map
            if "/" in inc:
                # Normalize legacy prefixes
                legacy = {
                    "Layout/": "layout/",
                    "Elements/": "widgets/",
                    "FontEngineDefault/": "font/default/",
                    "FontEngineHarfBuzz/": "font/harfbuzz/",
                }
                new_inc = inc
                for old, new in legacy.items():
                    if new_inc.startswith(old):
                        new_inc = new + new_inc[len(old) :]
                # ../default/FreeTypeInterface.h from harfbuzz
                if new_inc.startswith("../"):
                    resolved = (file_dir / new_inc).resolve()
                    try:
                        new_inc = resolved.relative_to(SRC.resolve()).as_posix()
                    except ValueError:
                        pass
                if new_inc != inc:
                    changed = True
                out_lines.append(f'{prefix}"{new_inc}"{rest}')
                continue

            # Bare "Foo.h" — same-directory keep; else qualify
            same_dir = file_dir / inc
            if same_dir.is_file():
                out_lines.append(line)
                continue

            candidates = by_base.get(inc, [])
            if not candidates:
                # might be missing or system - leave
                out_lines.append(line)
                continue
            if len(candidates) == 1:
                new_inc = candidates[0]
            else:
                # Prefer sibling under same top module, else require unique under file's tree
                top = file_rel_dir.split("/", 1)[0]
                same_top = [c for c in candidates if c.startswith(top + "/")]
                # Prefer deeper path sharing longest prefix with file
                def score(c: str) -> int:
                    fp = file_rel_dir.split("/")
                    cp = c.split("/")[:-1]
                    n = 0
                    for a, b in zip(fp, cp):
                        if a == b:
                            n += 1
                        else:
                            break
                    return n

                pool = same_top or candidates
                pool = sorted(pool, key=score, reverse=True)
                # If still tied and file is in one of the candidate dirs' parents uniquely
                best = pool[0]
                if len(pool) > 1 and score(pool[0]) == score(pool[1]):
                    # For known collisions, prefer explicit rules
                    if inc.startswith("TextureLayout"):
                        if file_rel_dir.startswith("font/harfbuzz"):
                            best = next(c for c in candidates if c.startswith("font/harfbuzz/"))
                        else:
                            best = next(c for c in candidates if c.startswith("paint/"))
                    elif file_rel_dir.startswith("font/harfbuzz"):
                        best = next((c for c in candidates if c.startswith("font/harfbuzz/")), pool[0])
                    elif file_rel_dir.startswith("font/default"):
                        best = next((c for c in candidates if c.startswith("font/default/")), pool[0])
                new_inc = best

            changed = True
            out_lines.append(f'{prefix}"{new_inc}"{rest}')

        if changed:
            path.write_text("\n".join(out_lines) + "\n", encoding="utf-8")
        return changed

    n = 0
    for path in SRC.rglob("*"):
        if not path.is_file() or path.suffix not in {".cpp", ".h", ".hpp", ".inl"}:
            continue
        if qualify_file(path):
            n += 1
            print("qualified", path.relative_to(ROOT))
    print(f"updated {n} source files")

    # CMake: single src/ root
    core_cmake = ROOT / "src" / "core" / "CMakeLists.txt"
    t = core_cmake.read_text(encoding="utf-8")
    old = """# Private module roots so #include \"Foo.h\" resolves across the engine.
target_include_directories(ui_core PRIVATE
	\"${CMAKE_SOURCE_DIR}/src/base\"
	\"${CMAKE_SOURCE_DIR}/src/style\"
	\"${CMAKE_SOURCE_DIR}/src/layout\"
	\"${CMAKE_SOURCE_DIR}/src/dom\"
	\"${CMAKE_SOURCE_DIR}/src/font\"
	\"${CMAKE_SOURCE_DIR}/src/xml\"
	\"${CMAKE_SOURCE_DIR}/src/data\"
	\"${CMAKE_SOURCE_DIR}/src/paint\"
	\"${CMAKE_SOURCE_DIR}/src/widgets\"
	\"${CMAKE_SOURCE_DIR}/src/core\"
	\"${CMAKE_SOURCE_DIR}/src/svg\"
)
"""
    new = """# Single private root: use #include \"module/Header.h\" for cross-module private headers.
# Same-folder #include \"Header.h\" still works via the including file's directory.
target_include_directories(ui_core PRIVATE
	\"${CMAKE_SOURCE_DIR}/src\"
)
"""
    if old not in t:
        # flexible replace of the block
        t2, count = re.subn(
            r"# Private module roots.*?target_include_directories\(ui_core PRIVATE\n(?:\t\"\$\{CMAKE_SOURCE_DIR\}/src/[^\"]+\"\n)+\)\n",
            new,
            t,
            count=1,
            flags=re.S,
        )
        if count != 1:
            raise SystemExit("could not patch src/core/CMakeLists.txt include dirs")
        t = t2
    else:
        t = t.replace(old, new)
    core_cmake.write_text(t, encoding="utf-8")
    print("patched src/core/CMakeLists.txt")

    # debugger target also needs src/ for private includes if any
    dbg = ROOT / "src" / "debugger" / "CMakeLists.txt"
    dt = dbg.read_text(encoding="utf-8")
    if "${CMAKE_SOURCE_DIR}/src\"" not in dt and "CMAKE_SOURCE_DIR}/src)" not in dt:
        dt = dt.replace(
            'target_include_directories(ui_debugger PRIVATE "${PP_UI_INCLUDE_DIR}")',
            'target_include_directories(ui_debugger PRIVATE "${PP_UI_INCLUDE_DIR}" "${CMAKE_SOURCE_DIR}/src")',
        )
        dbg.write_text(dt, encoding="utf-8")
        print("patched debugger includes")

    # tests unit includes
    for cmake in [
        ROOT / "tests" / "engine" / "Source" / "UnitTests" / "CMakeLists.txt",
    ]:
        if not cmake.exists():
            continue
        ct = cmake.read_text(encoding="utf-8")
        ct2, count = re.subn(
            r"target_include_directories\(\$\{TARGET_NAME\} PRIVATE\n(?:\t\"\$\{CMAKE_SOURCE_DIR\}/src/[^\"]+\"\n)+\)",
            'target_include_directories(${TARGET_NAME} PRIVATE\n\t"${CMAKE_SOURCE_DIR}/src"\n)',
            ct,
            count=1,
        )
        if count:
            cmake.write_text(ct2, encoding="utf-8")
            print("patched", cmake.relative_to(ROOT))
        else:
            print("WARN: no test include block match", cmake)

    # Remove harfbuzz per-dir includes if present
    hb = ROOT / "src" / "text" / "harfbuzz" / "CMakeLists.txt"
    ht = hb.read_text(encoding="utf-8")
    ht2 = re.sub(
        r"\ntarget_include_directories\(ui_core PRIVATE\n(?:\t\"[^\"]+\"\n)+\)\n",
        "\n",
        ht,
    )
    if ht2 != ht:
        hb.write_text(ht2, encoding="utf-8")
        print("removed harfbuzz extra include dirs")

    # Docs blurb
    layout = ROOT / "docs" / "SRC_LAYOUT.md"
    lt = layout.read_text(encoding="utf-8")
    note = """
## Private includes

Cross-module engine headers use a single `-I src` root and qualified paths:

```cpp
#include "dom/SelectionController.h"
#include "debugger/Geometry.h"
#include "layout/LayoutEngine.h"
```

Same-folder includes (`#include "LayoutEngine.h"` from `layout/LayoutEngine.cpp`) stay unqualified.
Public headers always use `<ui/module/Name.h>`.
"""
    if "## Private includes" not in lt:
        layout.write_text(lt.rstrip() + "\n" + note, encoding="utf-8")
        print("docs updated")


if __name__ == "__main__":
    main()
