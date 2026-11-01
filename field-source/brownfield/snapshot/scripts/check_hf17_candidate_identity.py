#!/usr/bin/env python3
"""HF17 HOTFIX_RELEASE exact-target identity guard; PENDING acceptance is not publication."""
from pathlib import Path
import json,sys
r=Path(__file__).resolve().parents[1]
def j(n):return json.loads((r/n).read_text(encoding="utf-8"))
s=j("RELEASE_STATE.json");a=j("metadata/RELEASE_AUTHORIZATION.json")
p=j("metadata/PUBLICATION_TRANSACTION.json");q=j("metadata/RELEASE_ACCEPTANCE.json")
g=j("metadata/GOVERNANCE_BINDING.json");v=j("metadata/PROJECT_GOVERNANCE_INVARIANTS.json")
I="preloader_l 3.2.5 SQUALERIS HOTFIX 17";F="preloader_l_3.2.5_SQUALERIS_HOTFIX_17.zip"
P="9b380e4d953dffd03ace8b762e1199f11ca8ca4f373d86a0c9b3c074d647082c"
c={"canonical_release_type":s.get("release_type")=="HOTFIX_RELEASE" and q.get("release_type")=="HOTFIX_RELEASE" and a.get("release_type")=="HOTFIX_RELEASE",
"identity":all(x.get("official_release_identity")==I for x in (s,a,p,q,g,v)),
"ordinal_parent":s.get("hotfix_ordinal")==17 and s.get("immediate_hotfix_parent",{}).get("sha256")==P,
"names":s.get("official_package_filename")==F and p.get("official_package_filename")==F and a.get("requested_ordinal")==17,
"separate_authorizations":a.get("source_development_authorized") is True and a.get("publication_authorized") is True and p.get("publication_authorized") is True,
"pointer_stays_hf16":p.get("canonical_before",{}).get("sha256")==P and s.get("canonical_artifact_pointer_commit") is False and p.get("canonical_pointer_committed") is False,
"g0_master_pin":g.get("master_revision")=="2026-10-17.2" and s.get("governance_master_revision")=="2026-10-17.2",
"freshness":all(q.get(x,{}).get("status")=="NOT_RUN_HF17_CHANGED_BUILD_INPUTS" for x in ("windows_native","windows_production_golden","static_crt")),
"games":all(q.get(x,{}).get("status")=="NOT_RUN_HF17" for x in ("collegefb27_game_network","madden27_game_network")),
"eleven_detours":j("metadata/RELEASE_ARCHITECTURE.json").get("minhook",{}).get("detours")==11,
"opaque_b_not_shipped":s.get("hf17_implementation",{}).get("b_ownership_prototype")=="EXCLUDED"}
for k,val in c.items():
 if not val:print("FAIL",k)
print("HF17_HOTFIX_RELEASE_TARGET_IDENTITY: %d/%d %s"%(sum(c.values()),len(c),"PASS" if all(c.values()) else "FAIL"))
sys.exit(0 if all(c.values()) else 1)
