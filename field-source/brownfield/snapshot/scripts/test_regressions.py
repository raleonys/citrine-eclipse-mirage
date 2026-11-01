#!/usr/bin/env python3
"""Canonical behavioral regression entrypoint for the current source line.

The retired custom-hook implementation stays absent while the inherited
application-behavior matrix is exercised through the MinHook-specific
8-shard suite. Arguments are forwarded unchanged.
"""
from pathlib import Path
import runpy
path=Path(__file__).with_name('test_minhook_regressions.py')
runpy.run_path(str(path),run_name='__main__')
