# SPDX-License-Identifier: GPL-3.0-only

"""Golden target activation; fixtures never activate a product contract."""
import json
from pathlib import Path


class PolicyPending(ValueError):
    pass


def validate_product_policy(name, policy):
    """Activation alone cannot turn a placeholder into an executable target."""
    fields = {
        'ARYRA_GOLDEN_LAYOUT.json': ('machine', 'pe_format_magic', 'section_count', 'sections_in_order', 'static_crt_fptable_profile'),
        'MUSTAVEX_GOLDEN_METADATA.json': ('required',),
        'URSYNEX_GOLDEN_FINALIZER.json': ('mutation_values', 'policy_bindings', 'integrity_finalization'),
        'CORVEXON_GOLDEN_TRAIL.json': ('required_core', 'shipping_exports', 'import_policy', 'integrity_policy'),
    }
    missing = [key for key in fields.get(name, ()) if key not in policy]
    if name == 'MUSTAVEX_GOLDEN_METADATA.json':
        missing += ['required.' + key for key in ('machine', 'pe_format_magic', 'subsystem', 'os_version',
                    'subsystem_version', 'timestamp', 'linker_version', 'rich_header', 'export_internal_name',
                    'versioninfo', 'additional_resource_assets') if key not in policy.get('required', {})]
    if name == 'URSYNEX_GOLDEN_FINALIZER.json':
        missing += ['mutation_values.' + key for key in ('timestamp', 'linker_version', 'rich_header')
                    if key not in policy.get('mutation_values', {})]
    if name == 'CORVEXON_GOLDEN_TRAIL.json':
        missing += ['import_policy.' + key for key in ('required_modules', 'allowed_modules', 'gs_runtime_policy', 'crt_runtime_policy')
                    if key not in policy.get('import_policy', {})]
        if not policy.get('shipping_exports'):
            missing.append('nonempty shipping_exports')
    if missing:
        raise PolicyPending(name + ': incomplete product target: ' + ', '.join(missing))


def load_target(root, name):
    policy = json.loads((Path(root) / 'metadata' / name).read_text(encoding='utf-8'))
    if policy.get('activation') != 'READY_LOCAL_TARGET':
        reasons = policy.get('unresolved', ['target values have not been established'])
        raise PolicyPending(name + ': ' + '; '.join(reasons))
    if policy.get('fixture_only'):
        raise PolicyPending('Synthetic fixtures cannot activate a product target')
    validate_product_policy(name, policy)
    return policy
