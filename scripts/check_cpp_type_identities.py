# SPDX-License-Identifier: GPL-3.0-only
"""Bounded C++ identity guard for renamed socket interface/adapter classes.

This check catches duplicate definitions, direct self-inheritance, and loss of
the eleven independently named interface/adapter pairs. It is not a C++ parser;
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
    "NativeMessageApi", "PrefixCipherBlocks", "PoolAllocationEffects",
    "PoolAppendEffects", "CancellationRecycler",
)


def inspect_text(text, filename="<fixture>"):
    """Inspect individual header definitions, not forward declarations."""
    seen = {}
    definitions = []
    errors = []
    for match in DECL.finditer(text):
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
    headers = sorted((Path(root) / "include" / "preloader_l").rglob("*.hpp"))
    counts = defaultdict(list)
    errors = []
    for path in headers:
        if path.is_symlink():
            errors.append(f"Symlinked active header is not accepted: {path}")
            continue
        data = path.read_text(encoding="utf-8")
        if "namespace preloader_l::socket" not in data:
            continue
        definitions, problems = inspect_text(data, path.relative_to(root).as_posix())
        errors.extend(problems)
        for name, line in definitions:
            if name in INTERFACES or (name.endswith("Adapter") and name[:-7] in INTERFACES):
                counts[name].append(f"{path.relative_to(root).as_posix()}:{line}")
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
