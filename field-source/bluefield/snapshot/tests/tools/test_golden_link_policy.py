# SPDX-License-Identifier: GPL-3.0-only
import importlib.util,sys
from pathlib import Path
sys.dont_write_bytecode=True
p=Path(__file__).resolve().parents[2]/'tools/golden/scripts/check_msvc_shipping_link_command.py'
spec=importlib.util.spec_from_file_location('shipping_link_policy',p)
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
sys.exit(module.self_test())
