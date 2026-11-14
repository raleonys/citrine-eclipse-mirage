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


def code_only(text):
    """Blank comments and literals while preserving offsets and line breaks.

    This is a bounded lexical prefilter, not a C++ parser or preprocessor.
    """
    output = list(text)

    def hide(start, stop):
        for at in range(start, stop):
            if text[at] not in '\r\n':
                output[at] = ' '

    i, n = 0, len(text)
    while i < n:
        if text.startswith('//', i):
            end = text.find('\n', i)
            end = n if end < 0 else end
            hide(i, end)
            i = end
            continue
        if text.startswith('/*', i):
            end = text.find('*/', i + 2)
            end = n if end < 0 else end + 2
            hide(i, end)
            i = end
            continue
        if text.startswith('R"', i):
            start = text.find('(', i + 2, min(n, i + 19))
            if start >= 0:
                delimiter = text[i + 2:start]
                if not any(c.isspace() or c in '\\()' for c in delimiter):
                    terminator = ')' + delimiter + '"'
                    end = text.find(terminator, start + 1)
                    if end >= 0:
                        end += len(terminator)
                        hide(i, end)
                        i = end
                        continue
        if text[i] in ('"', "'"):
            quote = text[i]
            start = i
            i += 1
            while i < n:
                if text[i] == '\\':
                    i += 2
                elif text[i] == quote:
                    i += 1
                    break
                else:
                    i += 1
            hide(start, min(i, n))
            continue
        i += 1
    return ''.join(output)


def inspect_text(text, filename="<fixture>"):
    """Inspect individual header definitions, not forward declarations."""
    seen = {}
    definitions = []
    errors = []
    for match in DECL.finditer(code_only(text)):
        name, suffix = match.group(1), match.group(2)
        line = text.count("\n", 0, match.start(1)) + 1
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
    headers = sorted((Path(root) / "include" / "laureondo").rglob("*.hpp"))
    counts = defaultdict(list)
    all_definitions = defaultdict(list)
    errors = []
    for path in headers:
        if path.is_symlink():
            errors.append(f"Symlinked active header is not accepted: {path}")
            continue
        data = path.read_text(encoding="utf-8")
        if "namespace laureondo::socket" not in code_only(data):
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
