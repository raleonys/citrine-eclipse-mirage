# SPDX-License-Identifier: GPL-3.0-only
"""debug operator entry;shared build implementation owns all behavior."""
from build_profiles import operator

if __name__ == "__main__":
    raise SystemExit(operator("debug"))
