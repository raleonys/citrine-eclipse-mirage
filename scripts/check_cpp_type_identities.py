# SPDX-License-Identifier: GPL-3.0-only
"""Bounded C++ identity guard for renamed socket interface/adapter classes.

This check catches duplicate definitions, direct self-inheritance, and loss of
the twelve independently named interface/adapter pairs. It is not a C++ parser;
the Windows compiler's full reference-object build remains authoritative.
"""
from collections import defaultdict
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
DECL = re.compile(r"(?m)^\s*(?:class|struct)\s+([A-Za-z_]\w*)\b([^;{}]*)\{")
BASE = re.compile(r"\b(?:public|private|protected)\s+([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)")
INTERFACES = (
    "WorkerStartupEffects", "ComposedWriteEffects", "AsyncReceiveEffects",
    "FrameReadApi", "ResponseDispatchApi", "ResponseQueueApi",
    "NativeMessageApi", "MessageDispatchApi", "PrefixCipherBlocks", "PoolAllocationEffects",
    "PoolAppendEffects", "CancellationRecycler",
)


def mask_cpp_literals_and_comments(text):
    """Hide comments and quoted literals without changing line positions.

    This is a bounded lexical prepass, not a C++ parser: preprocessor branches
    and compiler type resolution remain the authoritative build checks.
    """
    masked = list(text)
    size = len(text)

    def blank(start, stop):
        for index in range(start, stop):
            if text[index] not in "\r\n":
                masked[index] = " "

    index = 0
    while index < size:
        if text.startswith("//", index):
            end = text.find("\n", index)
            end = size if end < 0 else end
            blank(index, end)
            index = end
        elif text.startswith("/*", index):
            end = text.find("*/", index + 2)
            end = size if end < 0 else end + 2
            blank(index, end)
            index = end
        elif text.startswith('R"', index):
            # C++ raw-string delimiter ends at the first '(' and is <=16 chars.
            opening = text.find("(", index + 2, min(size, index + 19))
            if opening >= 0 and all(c not in "\r\n\t ()\\" for c in text[index+2:opening]):
                terminator = ")" + text[index+2:opening] + '"'
                closing = text.find(terminator, opening + 1)
                end = size if closing < 0 else closing + len(terminator)
                blank(index, end)
                index = end
            else:
                index += 1
        elif text[index] in ("\'", '"'):
            quote = text[index]
            end = index + 1
            while end < size:
                if text[end] == "\\":
                    end += 2
                elif text[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
            end = min(end, size)
            blank(index, end)
            index = end
        else:
            index += 1
    return "".join(masked)


def inspect_text(text, filename="<fixture>"):
    """Inspect individual header definitions, not forward declarations."""
    seen = {}
    definitions = []
    errors = []
    clean = mask_cpp_literals_and_comments(text)
    for match in DECL.finditer(clean):
        name, suffix = match.group(1), match.group(2)
        line = text.count("\n", 0, match.start()) + 1
        if name in seen:
            errors.append(f"{filename}:{line}: duplicate class/struct definition {name} (first at {seen[name]})")
        else:
            seen[name] = line
        for parent in BASE.findall(suffix):
            if parent == name:
                errors.append(f"{filename}:{line}: direct self-inheritance {name}")
        definitions.append((name, line))
    return definitions, errors


def inspect(root=ROOT):
    headers = sorted((Path(root) / "include" / "citrine_eclipse_mirage").rglob("*.hpp"))
    counts = defaultdict(list)
    all_definitions = defaultdict(list)
    errors = []
    for path in headers:
        if path.is_symlink():
            errors.append(f"Symlinked active header is not accepted: {path}")
            continue
        data = path.read_text(encoding="utf-8")
        if "namespace citrine_eclipse_mirage::socket" not in data:
            continue
        definitions, problems = inspect_text(data, path.relative_to(root).as_posix())
        errors.extend(problems)
        for name, line in definitions:
            all_definitions[name].append(f"{path.relative_to(root).as_posix()}:{line}")
            if name in INTERFACES or (name.endswith("Adapter") and name[:-7] in INTERFACES):
                counts[name].append(f"{path.relative_to(root).as_posix()}:{line}")
    for name, locations in sorted(all_definitions.items()):
        if len({site.rsplit(":", 1)[0] for site in locations}) > 1:
            errors.append(f"Duplicate definitions across socket headers for {name}: {locations}")
    for interface in INTERFACES:
        for name in (interface, interface + "Adapter"):
            if len(counts[name]) != 1:
                errors.append(f"Expected exactly one definition of {name}; found {counts[name]}")
    return len(headers), errors


if __name__ == "__main__":
    total, failures = inspect()
    for issue in failures:
        print(" -", issue)
    print(f"CPP_TYPE_IDENTITIES: {'PASS' if not failures else 'FAIL'} ({total} headers, {len(INTERFACES)} interface/adapter pairs)")
    raise SystemExit(bool(failures))
