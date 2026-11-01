#!/usr/bin/env python3
"""Fail-closed date provenance: upstream tag release != pinned commit != local import."""
from pathlib import Path
import json,datetime,sys
r=Path(__file__).resolve().parents[1]
try:
 d=json.loads((r/'metadata/UPSTREAM_PUBLICATION_DATES.json').read_text(encoding='utf-8'))
 assert d['schema']=='dll-rd-football.upstream-publication-dates.v1'
 commits={'asio':'third_party/asio/UPSTREAM_COMMIT.txt', 'pegtl':'third_party/pegtl/UPSTREAM_COMMIT.txt', 'filesystem':'third_party/filesystem/UPSTREAM_COMMIT.txt', 'minhook':'third_party/minhook/UPSTREAM_COMMIT.txt'}
 assert len(d['dependencies'])==5
 for name,entry in d['dependencies'].items():
  assert len(entry['pinned_commit'])==40
  assert entry['upstream_tag'] and entry['release_evidence_url'].startswith('https://')
  assert entry['published_release_date'] is not None
  datetime.date.fromisoformat(entry['published_release_date'])
  for key in ('pinned_commit_date','local_integration_date'):
   if entry[key] is not None: datetime.date.fromisoformat(entry[key])
  if not entry['pin_matches_exact_tag_revision']:
   assert entry['qualification']
  if name in commits:
   assert (r/commits[name]).read_text(encoding='utf-8').strip()==entry['pinned_commit'],name
  else:
   assert entry['pinned_commit'] in (r/'third_party/tinyxml2/REFERENCE_PROVENANCE.md').read_text(),name
 print('UPSTREAM_PUBLICATION_DATES: PASS (5/5 pin and date contracts)')
except Exception as e:
 print('UPSTREAM_PUBLICATION_DATES: FAIL',repr(e));sys.exit(1)
