# SPDX-License-Identifier: GPL-3.0-only
"""Local scoped Golden operator;shared implementation owns policy andbehavior."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent/"build_orchestration"))
from golden import cli

if __name__ == "__main__":
    raise SystemExit(cli(2))
