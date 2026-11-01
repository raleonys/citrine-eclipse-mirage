#!/usr/bin/env python3
"""Single version-independent externally pinned G0 entry point.

Development/build actions never invoke this script. Publication remains a G4 action.
"""
from check_external_g0 import main

if __name__ == '__main__':
    raise SystemExit(main())
