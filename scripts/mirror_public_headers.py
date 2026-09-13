#!/usr/bin/env python3
"""Move include/ui/Core → include/ui/<module> and rewrite includes. No shims."""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INC = ROOT / "include" / "ui"
CORE = INC / "Core"

PUB: dict[str, str] = {
    # base
    "Colour.h": "base",
    "Colour.inl": "base",
    "Debug.h": "base",
    "Dictionary.h": "base",
    "Header.h": "base",
    "ID.h": "base",
    "Log.h": "base",
    "Math.h": "base",
    "Matrix4.h": "base",
    "Matrix4.inl": "base",
    "NumericValue.h": "base",
    "ObserverPtr.h": "base",
    "Platform.h": "base",
    "Profiling.h": "base",
    "Rectangle.h": "base",
    "Span.h": "base",
    "StableVector.h": "base",
    "StringUtilities.h": "base",
    "Traits.h": "base",
    "Tween.h": "base",
    "TypeConverter.h": "base",
    "TypeConverter.inl": "base",
    "Types.h": "base",
    "URL.h": "base",
    "Utilities.h": "base",
    "Variant.h": "base",
    "Variant.inl": "base",
    "Vector2.h": "base",
    "Vector2.inl": "base",
    "Vector3.h": "base",
    "Vector3.inl": "base",
    "Vector4.h": "base",
    "Vector4.inl": "base",
    # style
    "Animation.h": "style",
    "ComputedValues.h": "style",
    "ConvolutionFilter.h": "style",
    "DecorationTypes.h": "style",
    "Decorator.h": "style",
    "EffectSpecification.h": "style",
    "Filter.h": "style",
    "PropertiesIteratorView.h": "style",
    "Property.h": "style",
    "PropertyDefinition.h": "style",
    "PropertyDictionary.h": "style",
    "PropertyIdSet.h": "style",
    "PropertyParser.h": "style",
    "PropertySpecification.h": "style",
    "Spritesheet.h": "style",
    "StyleSheet.h": "style",
    "StyleSheetContainer.h": "style",
    "StyleSheetSpecification.h": "style",
    "StyleSheetTypes.h": "style",
    "StyleTypes.h": "style",
    "Transform.h": "style",
    "TransformPrimitive.h": "style",
    "Unit.h": "style",
    # layout
    "Box.h": "layout",
    "LayoutTextElement.h": "layout",
    "LayoutElement.h": "layout",
    # dom
    "Context.h": "dom",
    "ContextInstancer.h": "dom",
    "Element.h": "dom",
    "Element.inl": "dom",
    "ElementDocument.h": "dom",
    "ElementInstancer.h": "dom",
    "ElementScroll.h": "dom",
    "ElementUtilities.h": "dom",
    "Event.h": "dom",
    "EventInstancer.h": "dom",
    "EventListener.h": "dom",
    "EventListenerInstancer.h": "dom",
    "Factory.h": "dom",
    "Input.h": "base",
    "ScrollTypes.h": "dom",
    # text
    "ElementText.h": "text",
    "FontEffect.h": "text",
    "FontEffectInstancer.h": "text",
    "FontEngineInterface.h": "text",
    "FontGlyph.h": "text",
    "FontMetrics.h": "base",
    "SelectionController.h": "text",
    "SelectionTypes.h": "text",
    "TextInputContext.h": "text",
    "TextInputHandler.h": "text",
    "TextLoupe.h": "text",
    "TextShapingContext.h": "text",
    # xml
    "BaseXMLParser.h": "xml",
    "Stream.h": "base",
    "StreamMemory.h": "base",
    "XMLNodeHandler.h": "xml",
    "XMLParser.h": "xml",
    # data
    "DataModelHandle.h": "data",
    "DataStructHandle.h": "data",
    "DataTypeRegister.h": "data",
    "DataTypes.h": "data",
    "DataVariable.h": "data",
    # paint
    "CallbackTexture.h": "paint",
    "CompiledFilterShader.h": "paint",
    "Geometry.h": "paint",
    "Mesh.h": "paint",
    "MeshUtilities.h": "paint",
    "RenderBox.h": "paint",
    "RenderInterface.h": "paint",
    "RenderInterfaceCompatibility.h": "paint",
    "RenderManager.h": "paint",
    "Texture.h": "paint",
    "UniqueRenderResource.h": "paint",
    "Vertex.h": "paint",
    # core bootstrap
    "Core.h": "core",
    "FileInterface.h": "core",
    "Plugin.h": "dom",
    "ScriptInterface.h": "base",
    "SystemInterface.h": "core",
    # widgets (from Core/Elements/)
    "ElementForm.h": "widgets",
    "ElementFormControl.h": "widgets",
    "ElementFormControlInput.h": "widgets",
    "ElementFormControlSelect.h": "widgets",
    "ElementFormControlTextArea.h": "widgets",
    "ElementProgress.h": "widgets",
    "ElementTabSet.h": "widgets",
}

WIDGETS = {
    "ElementForm.h",
    "ElementFormControl.h",
    "ElementFormControlInput.h",
    "ElementFormControlSelect.h",
    "ElementFormControlTextArea.h",
    "ElementProgress.h",
    "ElementTabSet.h",
}

SKIP_DIR_NAMES = {".git", "third_party", "build", "build-reorg", "build-tmp"}
REWRITE_SUFFIXES = {".h", ".hpp", ".inl", ".cpp", ".cc", ".c", ".md", ".txt", ".cmake", ".yml", ".yaml"}


def git_mv(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    subprocess.check_call(["git", "mv", str(src), str(dst)])


def main() -> None:
    flat = {p.name for p in CORE.iterdir() if p.is_file()}
    mapped_flat = {k for k, v in PUB.items() if k not in WIDGETS}
    missing = sorted(flat - mapped_flat)
    extra = sorted(mapped_flat - flat)
    if missing or extra:
        raise SystemExit(f"map mismatch missing={missing} extra={extra}")

    widgets_disk = {p.name for p in (CORE / "Elements").iterdir() if p.is_file()}
    if widgets_disk != WIDGETS:
        raise SystemExit(f"widgets mismatch disk={widgets_disk} map={WIDGETS}")

    print("Moving headers...")
    for name, mod in PUB.items():
        src = CORE / "Elements" / name if name in WIDGETS else CORE / name
        dst = INC / mod / name
        git_mv(src, dst)

    containers = CORE / "Containers"
    if containers.exists():
        git_mv(containers, INC / "base" / "Containers")

    for d in (CORE / "Elements", CORE):
        if d.exists():
            leftovers = list(d.rglob("*"))
            if leftovers:
                raise SystemExit(f"leftover in {d}: {leftovers}")
            d.rmdir()
            print("removed", d)

    basename_to_inc = {name: f"ui/{mod}/{name}" for name, mod in PUB.items()}

    def rewrite_text(text: str) -> str:
        out: list[str] = []
        for line in text.splitlines():
            m = re.match(r"^(\s*#include\s*)([<\"])([^>\"]+)([>\"])(.*)$", line)
            if not m:
                out.append(line + "\n")
                continue
            prefix, _q, path, _closer, rest = m.groups()
            new_path = None
            if path.startswith("ui/Core/Elements/"):
                new_path = basename_to_inc.get(path.rsplit("/", 1)[-1])
            elif path.startswith("ui/Core/Containers/"):
                new_path = "ui/base/containers/" + path[len("ui/Core/Containers/") :]
            elif path.startswith("ui/Core/"):
                base = path[len("ui/Core/") :]
                if "/" not in base:
                    new_path = basename_to_inc.get(base)
            elif path.startswith("Core/Elements/"):
                new_path = basename_to_inc.get(path.rsplit("/", 1)[-1])
            elif path.startswith("Core/Containers/"):
                new_path = "ui/base/containers/" + path[len("Core/Containers/") :]
            elif path.startswith("Core/"):
                base = path[len("Core/") :]
                if "/" not in base:
                    new_path = basename_to_inc.get(base)
            elif path.startswith("../Config/"):
                new_path = "ui/config/" + path[len("../Config/") :]
            elif path.startswith("../") and path.count("/") == 1:
                new_path = basename_to_inc.get(path[3:])
            elif "/" not in path and path in basename_to_inc:
                new_path = basename_to_inc[path]
            if new_path:
                out.append(f"{prefix}<{new_path}>{rest}\n")
            else:
                out.append(line + "\n")
        return "".join(out)

    changed = 0
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SKIP_DIR_NAMES or part.startswith("build") for part in path.parts):
            continue
        if path.suffix not in REWRITE_SUFFIXES and path.name != "CMakeLists.txt":
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue
        new = rewrite_text(text)
        if new != text:
            path.write_text(new, encoding="utf-8")
            changed += 1
    print(f"rewrote {changed} files")

    leftover: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SKIP_DIR_NAMES or part.startswith("build") for part in path.parts):
            continue
        if path.suffix not in REWRITE_SUFFIXES and path.name != "CMakeLists.txt":
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        if "ui/Core/" in text or '#include "Core/' in text:
            for i, line in enumerate(text.splitlines(), 1):
                if "ui/Core/" in line or '#include "Core/' in line:
                    leftover.append(f"{path}:{i}:{line.strip()}")
    print(f"leftover {len(leftover)}")
    for item in leftover[:40]:
        print(item)
    if leftover:
        raise SystemExit("leftover ui/Core includes remain")


if __name__ == "__main__":
    main()
