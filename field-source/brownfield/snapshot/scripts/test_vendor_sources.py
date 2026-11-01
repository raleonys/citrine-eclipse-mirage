#!/usr/bin/env python3
"""HF12 P2 standalone feasibility. Not a Windows-native acceptance substitute."""
from pathlib import Path
import hashlib
import json
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[1]

# Mandatory independent per-file/registry/usage contract; cheap --contract-only mode
# is called by repository QA (and thus by each applicable release QA shard).
def verify_dependency_contract():
    contract=json.loads((root/'metadata/DEPENDENCY_SOURCE_CONTRACT.json').read_text(encoding='utf-8'))
    assert contract['schema']=='dll-rd-football.hf12-third-party-dependency-contract.v1'
    assert contract['parent_source_sha256']=='f75897e8b474e2aa2f0e7cd4bbaf78bd28d27f48e3b5c4bb04b795abcb73b293'
    assert contract['release_authority']=='NONE; NOT_RELEASE'
    assert contract['policy']['shipped_deps']==['pegtl','filesystem']
    assert contract['policy']['staged_test_only']==['asio']
    assert all(contract['approval_policy'].values())
    assert contract['platform_build']['language']=='C++20'
    assert contract['platform_build']['windows_msvc_MT_MTd_GRminus_EHsc_native_verified'] is False
    assert contract['platform_build']['no_network_download_during_build'] is True
    cm=(root/'CMakeLists.txt').read_text(encoding='utf-8')
    assert 'target_compile_features(preloader_l PRIVATE cxx_std_20)' in cm
    assert 'target_compile_features(preloader_l_debug PRIVATE cxx_std_20)' in cm
    assert 'src/config/config2_pegtl.cpp' in cm
    assert 'third_party/pegtl/upstream/include' in cm
    assert 'third_party/filesystem/upstream/include' in cm
    # The historical HF12 P2 role above remains pinned, but HF13 source
    # integration is governed by an explicit separate *current* adoption gate.
    # Never let "compiled in shipping" be mislabeled as a live backend cutover.
    adoption=json.loads((root/'metadata/ASIO_ADOPTION_CONTRACT.json').read_text(encoding='utf-8'))
    assert adoption['schema']=='dll-rd-football.asio-production-adoption.v1'
    assert adoption['historical_contract']=='metadata/DEPENDENCY_SOURCE_CONTRACT.json'
    assert adoption['pinned_commit']==contract['dependencies']['asio']['pinned_git_commit']
    assert adoption['candidate_usage']=='SHIPPING_NATIVE_RELEASERVER_ASIO_ACTIVE'
    assert adoption['historical_usage_is_not_current_ownership'] is True
    assert adoption['windows_msvc_real_iocp_verified'] is True
    assert adoption['complete_production_acceptance'] is False
    assert adoption['production_shipping_session_integrated'] is True
    assert adoption['production_shipping_listener_integrated'] is True
    assert all(adoption['requirements'].values())
    assert cm.count('src/runtime/asio_session_owner.cpp')==2
    assert cm.count('src/runtime/asio_listener_owner.cpp')==2
    assert cm.count('src/runtime/asio_service_owner.cpp')==2
    assert cm.count('src/runtime/asio_release_bridge.cpp')==2
    assert (root/'src/runtime/asio_release_bridge.cpp').is_file()
    assert (root/'src/runtime/asio_service_owner.cpp').is_file()
    assert cm.count('third_party/asio/upstream/include')==2
    assert 'ASIO_DISABLE_IOCP' not in cm
    assert (root/'src/runtime/asio_session_owner.cpp').is_file()
    assert (root/'src/runtime/asio_listener_owner.cpp').is_file()
    assert 'AsioSessionOwner' not in (root/'src/runtime/release_server.cpp').read_text(encoding='utf-8')
    assert 'AsioListenerOwner' not in (root/'src/runtime/release_server.cpp').read_text(encoding='utf-8')
    assert 'asio::' not in (root/'src/net/socket_runtime.cpp').read_text(encoding='utf-8')
    assert 'module_relative_filesystem.hpp' in (root/'src/config/process_configuration.cpp').read_text(encoding='utf-8')
    assert 'module_relative_filesystem.hpp' in (root/'src/runtime/avatar_path.cpp').read_text(encoding='utf-8')
    reg=json.loads((root/'metadata/THIRD_PARTY_KNOWN_VULNERABILITIES.json').read_text(encoding='utf-8'))
    assert reg['governance_revision']==2
    assert reg['dependency_contract']=='metadata/DEPENDENCY_SOURCE_CONTRACT.json'
    recs={x['name']:x for x in reg['dependency_records']}
    assert len(recs)==len(reg['dependency_records'])==5
    assert {'TinyXML2','MinHook'}.issubset(recs)
    expect={
      'pegtl':('3b075432e94f6f906425350b43d03cd9cebfa4f1','SHIPPING_LINKED','BSL-1.0','LICENSE_1_0.txt','src/config/config2_pegtl.cpp'),
      'filesystem':('3812f8a6f31ed67d32272ac33fe2b85c6b2c947e','SHIPPING_PATH_ONLY','MIT','LICENSE','src/config/module_relative_filesystem.hpp'),
      'asio':('8806a6803cde7054c3049d3666d3ec36786568c5','VENDORED_TEST_ONLY','BSL-1.0','LICENSE_1_0.txt','tests/unit/test_asio_lifecycle.cpp')
    }
    total=0
    for key,(sha,usage,license_name,lic_file,anchor) in expect.items():
        d=contract['dependencies'][key]
        assert (d['pinned_git_commit'],d['usage'],d['license_spdx'],d['usage_anchor'])==(sha,usage,license_name,anchor)
        assert d['vendor_bytes_policy']=='PRISTINE_UPSTREAM_NO_PATCH_NO_FORK'
        assert len(sha)==40
        assert (root/'third_party'/key/'UPSTREAM_COMMIT.txt').read_text().strip()==sha
        assert (root/d['license_file']).is_file() and d['license_file'].endswith(lic_file)
        assert (root/anchor).is_file()
        entries=d['upstream_file_sha256']
        base=root/d['vendored_tree']
        actual={p.relative_to(base).as_posix() for p in base.rglob('*') if p.is_file()}
        assert actual==set(entries), f'{key}: vendor file-set mismatch: missing={len(set(entries)-actual)} extra={len(actual-set(entries))}'
        assert len(entries)==d['file_count']
        for relative,digest in entries.items():
            assert hashlib.sha256((base/relative).read_bytes()).hexdigest()==digest, f'{key}: upstream vendor byte drift: {relative}'
        rr=recs[d['name']]
        assert (rr['pinned_commit'],rr['usage'],rr['license_spdx'])==(sha,usage,license_name)
        assert rr['issues']==[]
        assert rr['security_review_status']=='PENDING_INDEPENDENT_UPSTREAM_ADVISORY_CVE_AND_AFFECTEDNESS_REVIEW'
        total+=len(entries)
    docs=(root/contract['contract_document']).read_text(encoding='utf-8')
    assert 'NOT_RELEASE' in docs or 'not a release' in docs
    assert 'prior' in docs and 'user' in docs
    riskdoc=(root/'docs/THIRD_PARTY_KNOWN_VULNERABILITIES.md').read_text(encoding='utf-8')
    for key in ('pegtl','filesystem','asio'):
        assert contract['dependencies'][key]['pinned_git_commit'] in docs
        assert contract['dependencies'][key]['pinned_git_commit'] in riskdoc
    assert len(contract['dependencies'])==3
    print(f'HF12_THIRD_PARTY_GOVERNANCE: PASS (3 pins, {total}/{total} pristine upstream bytes, 5/5 risk records, native pending)')

verify_dependency_contract()
if '--contract-only' in sys.argv:
    sys.exit(0)
pins={
 'pegtl': ('3b075432e94f6f906425350b43d03cd9cebfa4f1','include/tao/pegtl.hpp','LICENSE_1_0.txt'),
 'filesystem': ('3812f8a6f31ed67d32272ac33fe2b85c6b2c947e','include/ghc/filesystem.hpp','LICENSE'),
 'asio': ('8806a6803cde7054c3049d3666d3ec36786568c5','include/asio.hpp','LICENSE_1_0.txt')
}
for name,(commit,header,license_name) in pins.items():
    folder=root/'third_party'/name
    assert (folder/'UPSTREAM_COMMIT.txt').read_text().strip()==commit,name+' pin'
    assert (folder/'upstream'/header).is_file(),name+' include'
    assert (folder/'upstream'/license_name).is_file(),name+' license'
    assert len(list((folder/'upstream'/'include').rglob('*.hpp')))>=1
print('HF12_THREE_VENDOR_PIN_SMOKE: PASS')
with tempfile.TemporaryDirectory(prefix='hf12_vendor_') as tmp:
    for cxx in ('g++','clang++'):
        for name,src,include,opts in [
            ('filesystem','tests/unit/test_filesystem_path.cpp',
                'third_party/filesystem/upstream/include',[]),
            ('asio','tests/unit/test_asio_lifecycle.cpp',
                'third_party/asio/upstream/include',['-pthread','-DASIO_STANDALONE=1','-DASIO_NO_TYPEID=1'])]:
            binary=str(Path(tmp)/('test_'+name+'_'+cxx.replace('+','p')))
            args=[cxx,'-std=c++20','-fno-rtti','-O2',*opts,'-I'+str(root/include),str(root/src),'-o',binary]
            subprocess.run(args,check=True,timeout=90,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            result=subprocess.run([binary],check=True,timeout=30,capture_output=True,text=True)
            assert 'PASS' in result.stdout
            print(cxx,name,result.stdout.strip())
print('HF12_VENDOR_PORTABLE_PROBE: PASS')
