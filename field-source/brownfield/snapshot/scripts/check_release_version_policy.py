#!/usr/bin/env python3
"""Release-independent identity/version dispatch for active source-stage QA.

The exact release metadata, not a switch on HOTFIX ordinals, determines the
current identity and official filename. This is never a G0 or G4 approval.
Historical fixed-release checks are retained outside the active QA route.
"""
from pathlib import Path
import subprocess,sys
from check_active_release_contract import validate

errors = validate(Path(__file__).resolve().parents[1])
if errors:
    print('RELEASE_VERSION_POLICY: FAIL_SOURCE_STRUCTURE', ','.join(errors))
    sys.exit(1)
root=Path(__file__).resolve().parents[1]
regression=subprocess.run([sys.executable,'-B',str(root/'scripts/test_active_release_contract.py')],cwd=root)
if regression.returncode:
    print('RELEASE_VERSION_POLICY: FAIL_RELEASE_CONTRACT_REGRESSION')
    sys.exit(1)
print('RELEASE_VERSION_POLICY: PASS_SOURCE_STRUCTURE_ONLY; no G0/G2/G4 claim')
