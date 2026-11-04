# SPDX-License-Identifier: GPL-3.0-only

"""Tamper, failure-propagation and source/package identity regressions."""
from pathlib import Path
import contextlib
import io
import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import check_falcion_golden_cleaning as falcion
import check_metadata_ownership as ownership
import check_human_launcher_contract as launchers
import project_files as inventory
import package_source as packaging
import extract_source as extraction
import run_golden_pipeline as golden
import ursynex_golden_finalizer as finalizer
import verify_mustavex_golden_metadata as mustavex
import generate_avatar_data as avatar_data
import clear_tool_python_cache as tool_cache
import check_repository_contract as repository
import verify_sources as source_verification
import run_source_qa as source_qa
import ci_validation_readiness as ci_readiness
from golden_policy import load_target, PolicyPending


class ValidationReadinessTests(unittest.TestCase):
    head = 'a' * 40
    base = 'b' * 40
    repository = 'owner/project'

    def setUp(self):
        self.event = {'action': 'synchronize', 'number': 23,
                      'before': 'e' * 40, 'after': self.head,
                      'repository': {'default_branch': 'main'},
                      'pull_request': {'head': {'sha': self.head},
                                       'updated_at': '2026-11-04T12:00:00Z',
                                       'base': {'sha': 'c' * 40}}}
        self.current = {'state': 'open', 'created_at': '2026-11-04T10:00:00Z',
                        'head': {'sha': self.head, 'ref': 'feature/sync',
                                 'repo': {'full_name': self.repository}},
                        'base': {'ref': 'main'}}
        self.main = self.base
        self.selected = self.head
        self.comparison = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.previous = {'status': 'diverged', 'merge_base_commit': {'sha': 'c' * 40}}
        self.associated = []
        self.calls = []
        self.sync_runs = {1: []}
        self.sync_jobs = {1: []}

    def api(self, endpoint):
        self.calls.append(endpoint)
        prefix = 'repos/' + self.repository
        if endpoint == prefix + '/pulls/23':
            return self.current
        if endpoint == prefix + '/pulls?state=open&base=main&head=owner%3Afeature%2Fsync&per_page=100':
            return self.associated
        if endpoint == prefix + '/git/ref/heads/main':
            return {'object': {'sha': self.main}}
        if endpoint == prefix + '/git/ref/heads/feature%2Fsync':
            return {'object': {'sha': self.selected}}
        if endpoint == prefix + '/compare/' + self.main + '...' + self.head:
            return self.comparison
        if endpoint == prefix + '/compare/' + self.main + '...' + self.event.get('before', ''):
            return self.previous
        match = re.fullmatch(re.escape(prefix + '/actions/workflows/msvc-reference-graph.yml/runs?'
                                       'event=workflow_dispatch&head_sha=' + self.head +
                                       '&per_page=100&page=') + r'([0-9]+)', endpoint)
        if match:
            return {'workflow_runs': self.sync_runs.get(int(match[1]), [])}
        match = re.fullmatch(re.escape(prefix + '/actions/runs/101/jobs?filter=latest&per_page=100&page=') +
                             r'([0-9]+)', endpoint)
        if match:
            return {'jobs': self.sync_jobs.get(int(match[1]), [])}
        self.fail('Unexpected GitHub endpoint: ' + endpoint)

    def decision(self, name='pull_request', ref='refs/heads/feature/sync'):
        if name == 'workflow_dispatch':
            self.event.setdefault('inputs', {'operation': 'diagnostic'})
        return ci_readiness.decide(name, self.event, self.repository, self.head, ref, self.api)

    def test_open_reopen_and_metadata_events_do_not_start_full_tests(self):
        for action in ('opened', 'reopened', 'edited', 'labeled', 'ready_for_review'):
            self.event['action'] = action
            self.assertFalse(self.decision().run_tests)
        self.assertEqual([], self.calls)

    def test_sync_checks_live_main_instead_of_the_event_base_snapshot(self):
        decision = self.decision()
        self.assertTrue(decision.run_tests)
        self.assertEqual(self.base, decision.base_sha)
        self.assertNotEqual(self.event['pull_request']['base']['sha'], decision.base_sha)
        self.assertTrue(any(self.base + '...' + self.head in call for call in self.calls))

    def request_pr_validation(self, verified=True):
        self.event['action'] = 'labeled'
        self.event['label'] = {'name': 'run-full-validation'}
        self.current['labels'] = [{'name': 'run-full-validation'}]
        # A real labeled webhook has neither synchronization identity field.
        del self.event['before']
        del self.event['after']
        if verified:
            self.sync_runs[1] = [{'id': 101, 'event': 'workflow_dispatch',
                                  'path': ci_readiness.VALIDATION_WORKFLOW,
                                  'head_sha': self.head, 'head_branch': 'feature/sync',
                                  'status': 'completed', 'conclusion': 'success',
                                  'created_at': '2026-11-04T11:00:00Z',
                                  'updated_at': '2026-11-04T11:01:00Z'}]
            self.sync_jobs[1] = [{'name': ci_readiness.sync_job_name(23, self.base, self.head),
                                  'status': 'completed', 'conclusion': 'success'}]

    def test_explicit_label_requests_tests_in_pr_context_without_a_head_push(self):
        self.request_pr_validation()
        decision = self.decision()
        self.assertTrue(decision.run_tests)
        self.assertEqual(self.head, decision.head_sha)
        self.assertEqual(self.base, decision.base_sha)
        self.assertIn('repos/owner/project/pulls/23', self.calls)

    def test_initial_label_on_a_current_branch_cannot_start_full_tests(self):
        self.request_pr_validation(verified=False)
        decision = self.decision()
        self.assertFalse(decision.run_tests)
        self.assertIn('subsequent PR synchronization request', decision.reason)

    def test_repeated_initial_labels_cannot_create_their_own_sync_proof(self):
        self.request_pr_validation(verified=False)
        for _ in range(3):
            self.assertFalse(self.decision().run_tests)

    def test_sync_proof_is_bound_to_pr_number_head_and_current_main(self):
        self.request_pr_validation()
        for number, base, head in ((24, self.base, self.head),
                                   (23, 'd' * 40, self.head),
                                   (23, self.base, 'd' * 40)):
            self.sync_jobs[1][0]['name'] = ci_readiness.sync_job_name(number, base, head)
            self.assertFalse(self.decision().run_tests)

    def test_only_a_successful_preparation_job_from_the_selected_workflow_is_proof(self):
        self.request_pr_validation()
        original = dict(self.sync_runs[1][0])
        for key, value in (('event', 'pull_request'), ('path', '.github/workflows/other.yml'),
                           ('head_sha', 'd' * 40), ('head_branch', 'other'),
                           ('status', 'in_progress'), ('conclusion', 'failure')):
            self.sync_runs[1] = [dict(original, **{key: value})]
            self.assertFalse(self.decision().run_tests)
        self.sync_runs[1] = [original]
        for conclusion in ('skipped', 'failure', 'cancelled'):
            self.sync_jobs[1][0]['conclusion'] = conclusion
            self.assertFalse(self.decision().run_tests)

    def test_premature_label_cannot_adopt_a_later_sync_request(self):
        self.request_pr_validation()
        self.event['pull_request']['updated_at'] = '2026-11-04T10:30:00Z'
        self.assertFalse(self.decision().run_tests)
        self.event['pull_request']['updated_at'] = '2026-11-04T12:00:00Z'
        self.assertTrue(self.decision().run_tests)

    def test_preparation_before_opening_or_after_labeling_is_not_proof(self):
        self.request_pr_validation()
        original = dict(self.sync_runs[1][0])
        for changes in ({'created_at': '2026-11-04T09:00:00Z'},
                        {'updated_at': '2026-11-04T13:00:00Z'},
                        {'created_at': '2026-11-04T11:02:00Z'}):
            self.sync_runs[1] = [dict(original, **changes)]
            self.assertFalse(self.decision().run_tests)

    def test_sync_proof_search_paginates_runs_and_jobs(self):
        self.request_pr_validation()
        self.sync_runs[2] = self.sync_runs[1]
        self.sync_runs[1] = [{'event': 'push'}] * 100
        self.sync_jobs[2] = self.sync_jobs[1]
        self.sync_jobs[1] = [{'name': 'Standalone diagnostic'}] * 100
        self.assertTrue(self.decision().run_tests)
        self.assertTrue(any('runs?' in c and c.endswith('page=2') for c in self.calls))
        self.assertTrue(any('jobs?' in c and c.endswith('page=2') for c in self.calls))

    def prepare_sync_request(self):
        self.event['inputs'] = {'operation': 'synchronize-pr', 'pr_number': '23',
                                'sync_base': self.base}

    def record_sync(self, ref='refs/heads/feature/sync'):
        return ci_readiness.record_sync_request('workflow_dispatch', self.event,
                                                self.repository, self.head, ref, self.api)

    def test_later_noop_sync_verification_succeeds_without_starting_tests(self):
        self.prepare_sync_request()
        decision = self.record_sync()
        self.assertFalse(decision.run_tests)
        self.assertEqual(self.head, decision.head_sha)
        self.assertEqual(self.base, decision.base_sha)
        self.assertIn('verified', decision.reason)

    def test_preparation_mode_cannot_fall_through_to_full_diagnostics(self):
        self.prepare_sync_request()
        self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertEqual([], self.calls)

    def test_missing_and_unknown_manual_operations_cannot_start_full_tests(self):
        for inputs in ({}, {'operation': ''}, {'operation': 'sync'}, {'operation': 'DIAGNOSTIC'}):
            self.event['inputs'] = inputs
            self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertEqual([], self.calls)

    def test_sync_verification_requires_the_exact_open_owned_branch(self):
        self.prepare_sync_request()
        for ref in ('refs/heads/main', 'refs/tags/v1'):
            with self.assertRaises(ValueError):
                self.record_sync(ref)
        original = json.loads(json.dumps(self.current))
        for change in ('closed', 'head', 'fork', 'base'):
            self.current = json.loads(json.dumps(original))
            if change == 'closed': self.current['state'] = 'closed'
            if change == 'head': self.current['head']['sha'] = 'd' * 40
            if change == 'fork': self.current['head']['repo']['full_name'] = 'other/project'
            if change == 'base': self.current['base']['ref'] = 'other'
            with self.assertRaises(ValueError):
                self.record_sync()

    def test_sync_verification_rejects_stale_main_and_an_unremoved_initial_label(self):
        self.prepare_sync_request()
        self.current['labels'] = [{'name': 'run-full-validation'}]
        with self.assertRaises(ValueError): self.record_sync()
        self.current['labels'] = []
        self.event['inputs']['sync_base'] = 'd' * 40
        with self.assertRaises(ValueError): self.record_sync()
        self.event['inputs']['sync_base'] = self.base
        self.comparison = {'status': 'diverged'}
        with self.assertRaises(ValueError): self.record_sync()

    def test_preparation_validates_operation_pr_number_and_commit_inputs(self):
        self.prepare_sync_request()
        for number in ('', '0', '-1', '23/other', '023', 23):
            self.event['inputs']['pr_number'] = number
            with self.assertRaises(ValueError): self.record_sync()
        self.event['inputs']['pr_number'] = '23'
        self.event['inputs']['operation'] = 'diagnostic'
        with self.assertRaises(ValueError): self.record_sync()

    def test_bad_sync_receipt_identity_and_timestamp_fail_closed(self):
        self.request_pr_validation()
        self.sync_runs[1][0]['id'] = '101/jobs'
        with self.assertRaises(ValueError): self.decision()
        self.sync_runs[1][0]['id'] = 101
        self.sync_runs[1][0]['updated_at'] = '2026-11-04T11:01:00'
        with self.assertRaises(ValueError): self.decision()

    def run_cli(self, directory, event_name, record=False):
        root = Path(directory)
        event = root / 'event.json'
        event.write_text(json.dumps(self.event))
        output = root / 'output'
        argv = ['ci_validation_readiness.py', '--event-name', event_name,
                '--event-path', str(event), '--repository', self.repository,
                '--sha', self.head, '--ref', 'refs/heads/feature/sync',
                '--output', str(output), '--summary', str(root / 'summary')]
        if record: argv.append('--record-sync')
        with patch.object(sys, 'argv', argv), \
                patch.object(ci_readiness, 'github_json', side_effect=self.api), \
                contextlib.redirect_stdout(io.StringIO()):
            result = ci_readiness.main()
        return result, output.read_text()

    def test_cli_preparation_records_current_identity_without_full_test_permission(self):
        self.prepare_sync_request()
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'workflow_dispatch', record=True)
        self.assertEqual(0, status)
        self.assertIn('run_tests=false\n', output)
        self.assertIn('head_sha=' + self.head + '\n', output)
        self.assertIn('base_sha=' + self.base + '\n', output)
        self.assertIn('verified', output)

    def test_cli_premature_label_defers_without_fabricating_a_test_failure(self):
        self.request_pr_validation(verified=False)
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'pull_request')
        self.assertEqual(0, status)
        self.assertIn('run_tests=false\n', output)
        self.assertNotIn('run_tests=true', output)

    def test_cli_failed_preparation_cannot_create_a_successful_receipt(self):
        self.prepare_sync_request()
        self.event['inputs']['sync_base'] = 'd' * 40
        with tempfile.TemporaryDirectory() as directory:
            status, output = self.run_cli(directory, 'workflow_dispatch', record=True)
        self.assertEqual(1, status)
        self.assertIn('run_tests=false\n', output)
        self.assertNotIn('head_sha=' + self.head, output)

    def test_removed_request_label_cannot_start_full_tests(self):
        self.request_pr_validation()
        self.current['labels'] = []
        self.assertFalse(self.decision().run_tests)

    def test_explicit_label_still_requires_current_main_and_current_open_head(self):
        self.request_pr_validation()
        self.comparison = {'status': 'diverged'}
        self.assertFalse(self.decision().run_tests)
        self.comparison = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.current['head']['sha'] = 'd' * 40
        self.assertFalse(self.decision().run_tests)
        self.current['head']['sha'] = self.head
        self.current['state'] = 'closed'
        self.assertFalse(self.decision().run_tests)

    def test_retained_label_does_not_turn_ordinary_edits_into_full_test_requests(self):
        self.current['labels'] = [{'name': 'run-full-validation'}]
        self.previous = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)

    def test_other_label_actions_do_not_request_full_tests(self):
        self.event['label'] = {'name': 'changelog-exempt'}
        self.event['action'] = 'labeled'
        self.assertFalse(self.decision().run_tests)
        self.event['label'] = {'name': 'run-full-validation'}
        self.event['action'] = 'unlabeled'
        self.assertFalse(self.decision().run_tests)
        self.assertEqual([], self.calls)

    def test_regular_commit_on_an_already_synced_branch_waits_for_manual_request(self):
        self.previous = {'status': 'ahead', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)
        self.assertTrue(self.decision('workflow_dispatch').run_tests)

    def test_missing_previous_head_cannot_claim_a_main_integration(self):
        del self.event['before']
        with self.assertRaises(KeyError):
            self.decision()

    def test_inconsistent_previous_comparison_cannot_start_tests(self):
        for previous in ({}, {'status': 'ahead', 'merge_base_commit': {'sha': 'c' * 40}},
                         {'status': 'diverged', 'merge_base_commit': {'sha': self.base}}):
            self.previous = previous
            self.assertFalse(self.decision().run_tests)

    def test_update_after_sha_must_match_the_actual_pr_head(self):
        self.event['after'] = 'd' * 40
        self.assertFalse(self.decision().run_tests)

    def test_stale_diverged_and_malformed_comparisons_are_not_ready(self):
        for comparison in ({'status': 'behind'}, {'status': 'diverged'}, {},
                           {'status': 'ahead', 'merge_base_commit': {'sha': 'c' * 40}}):
            self.comparison = comparison
            self.assertFalse(self.decision().run_tests)

    def test_identical_manual_main_is_allowed(self):
        self.head = self.base
        self.comparison = {'status': 'identical', 'merge_base_commit': {'sha': self.base}}
        self.assertTrue(self.decision('workflow_dispatch', 'refs/heads/main').run_tests)

    def test_closed_changed_or_non_main_pr_is_not_ready(self):
        for current in ({'state': 'closed', 'head': {'sha': self.head}, 'base': {'ref': 'main'}},
                        {'state': 'open', 'head': {'sha': 'd' * 40}, 'base': {'ref': 'main'}},
                        {'state': 'open', 'head': {'sha': self.head}, 'base': {'ref': 'research'}}):
            self.current = current
            self.assertFalse(self.decision().run_tests)

    def test_manual_dispatch_requires_the_selected_current_branch_head(self):
        self.assertTrue(self.decision('workflow_dispatch').run_tests)
        self.selected = 'd' * 40
        self.assertFalse(self.decision('workflow_dispatch').run_tests)
        self.assertFalse(self.decision('workflow_dispatch', 'refs/tags/v1').run_tests)

    def test_manual_stale_branch_does_not_allocate_full_tests(self):
        self.comparison = {'status': 'diverged'}
        self.assertFalse(self.decision('workflow_dispatch').run_tests)

    def test_changelog_exemption_uses_live_pr_labels(self):
        self.assertFalse(self.decision().changelog_exempt)
        self.current['labels'] = [{'name': 'changelog-exempt'}]
        self.assertTrue(self.decision().changelog_exempt)

    def test_manual_branch_keeps_changelog_requirement_unless_approved(self):
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)
        self.associated = [self.current]
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)
        self.current['labels'] = [{'name': 'changelog-exempt'}]
        self.assertTrue(self.decision('workflow_dispatch').changelog_exempt)

    def test_manual_request_cannot_borrow_exemption_from_another_head(self):
        self.associated = [{'state': 'open', 'base': {'ref': 'main'},
                            'head': {'sha': 'd' * 40},
                            'labels': [{'name': 'changelog-exempt'}]}]
        self.assertFalse(self.decision('workflow_dispatch').changelog_exempt)

    def test_bad_commit_and_repository_identities_are_rejected(self):
        self.event['inputs'] = {'operation': 'diagnostic'}
        with self.assertRaises(ValueError):
            ci_readiness.decide('workflow_dispatch', self.event, '../project', self.head,
                                'refs/heads/main', self.api)
        self.event['pull_request']['head']['sha'] = self.head + '\nrun_tests=true'
        with self.assertRaises(ValueError):
            self.decision()

    def test_cli_api_failure_writes_false_and_returns_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            event = root / 'event.json'
            event.write_text(json.dumps(self.event))
            output, summary = root / 'output', root / 'summary'
            argv = ['ci_validation_readiness.py', '--event-name', 'pull_request',
                    '--event-path', str(event), '--repository', self.repository,
                    '--output', str(output), '--summary', str(summary)]
            with patch.object(sys, 'argv', argv), \
                    patch.object(ci_readiness, 'github_json', side_effect=RuntimeError('API failure')), \
                    contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(1, ci_readiness.main())
            self.assertIn('run_tests=false\n', output.read_text())
            self.assertNotIn('run_tests=true', output.read_text())
            self.assertIn('could not be verified', summary.read_text())

    def test_new_main_after_a_prior_success_invalidates_eligibility(self):
        self.assertTrue(self.decision().run_tests)
        self.main = 'd' * 40
        self.comparison = {'status': 'diverged', 'merge_base_commit': {'sha': self.base}}
        self.assertFalse(self.decision().run_tests)

    def test_deferred_jobs_are_gated_before_matrices_and_runner_allocation(self):
        workflow = (ROOT / '.github/workflows/msvc-reference-graph.yml').read_text()
        trigger = workflow.split('\npermissions:', 1)[0]
        self.assertIn('types: [synchronize, labeled]', trigger)
        self.assertIn("github.event.label.name != 'run-full-validation' && 'unrequested-label'", workflow)
        self.assertNotIn('  push:', trigger)
        self.assertNotIn('pull_request_target', workflow)
        blocks = dict(re.findall(r'^  ([a-z-]+):\n(.*?)(?=^  [a-z-]+:|\Z)',
                                 workflow.split('\njobs:\n', 1)[1], re.M | re.S))
        required = {'source-qa', 'windows', 'windows-portable', 'reference-graph'}
        self.assertEqual(required | {'readiness', 'sync-request'}, set(blocks))
        self.assertIn("if: github.event_name != 'workflow_dispatch' || inputs.operation == 'diagnostic'", blocks['readiness'])
        self.assertIn("if: github.event_name == 'workflow_dispatch' && inputs.operation == 'synchronize-pr'", blocks['sync-request'])
        self.assertIn('scripts/ci_validation_readiness.py --record-sync', blocks['sync-request'])
        self.assertIn("inputs.operation == 'synchronize-pr' && 'sync-request'", workflow)
        self.assertIn("name: 'PR sync #${{ inputs.pr_number }}: ${{ inputs.sync_base }} / ${{ github.sha }}'", blocks['sync-request'])
        for name in required:
            block = blocks[name]
            self.assertIn("\n    if: ${{ needs.readiness.result == 'success' && needs.readiness.outputs.run_tests == 'true' }}", block)
            self.assertIn('needs: readiness', block)
            self.assertNotIn('Require verified main synchronization', block)
            self.assertNotIn('if: ${{ always() }}', block)
            self.assertIn('ref: ${{ needs.readiness.outputs.head_sha }}', block)
            if name != 'source-qa':
                self.assertIn('runs-on: windows-2022', block)
                self.assertNotIn('ubuntu-24.04', block)
                self.assertIn('    strategy:\n', block)
        profiles = set(re.findall(r'- profile: ([a-z-]+)', blocks['windows']))
        contexts = {'Source QA and portable tests'} | {'Windows ' + p for p in profiles}
        contexts |= {'Windows portable ' + c for c in ('Debug', 'Release')}
        contexts |= {'Windows x64 reference objects ' + c for c in ('Debug', 'Release')}
        self.assertEqual(11, len(contexts))
        self.assertEqual({'native', 'native-debug', 'production', 'crt', 'debug', 'compile'}, profiles)
        self.assertIn("name: ${{ needs.readiness.outputs.run_tests == 'true' && 'Source QA and portable tests' || 'Source QA deferred' }}", blocks['source-qa'])
        self.assertIn("needs.readiness.outputs.changelog_exempt != 'true'", blocks['source-qa'])
        self.assertIn('needs.readiness.outputs.head_sha != needs.readiness.outputs.base_sha', blocks['source-qa'])
        self.assertIn('name: Windows ${{ matrix.profile }}', blocks['windows'])
        self.assertIn('name: Windows portable ${{ matrix.configuration }}', blocks['windows-portable'])
        self.assertIn('name: Windows x64 reference objects ${{ matrix.configuration }}', blocks['reference-graph'])

    def test_opening_ci_is_integrity_only_and_diagnostics_are_manual(self):
        fast = (ROOT / '.github/workflows/ci.yml').read_text()
        self.assertIn('scripts/verify_sources.py', fast)
        self.assertIn('scripts/check_manifest.py', fast)
        self.assertNotIn('run_source_qa.py', fast)
        self.assertNotIn('cmake --', fast)
        for path in (ROOT / '.github/workflows').glob('*.yml'):
            if path.name in ('ci.yml', 'msvc-reference-graph.yml', 'weekly-release.yml'):
                continue
            trigger = path.read_text().split('\non:\n', 1)[1].split('\npermissions:', 1)[0]
            self.assertIn('  workflow_dispatch:', trigger, path.name)
            self.assertNotIn('pull_request:', trigger, path.name)
            self.assertNotIn('  push:', trigger, path.name)


class IdentityBoundaryTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        (self.root/'docs').mkdir()
        (self.root/'src').mkdir()
        (self.root/'metadata').mkdir()
        (self.root/'docs/IDENTIFIER_POLICY.md').write_bytes((ROOT/'docs/IDENTIFIER_POLICY.md').read_bytes())
        self.label=repository.load_active_name_policy(self.root)[0].upper()

    def test_documents_are_allowed_but_active_comments_and_schemas_are_rejected(self):
        (self.root/'docs/history.json').write_text(json.dumps({'origin':self.label}))
        (self.root/'LICENSE').write_text('Source attribution: '+self.label)
        self.assertEqual([],repository.check_active_identifiers(self.root))
        source=self.root/'src/module.cpp'
        source.write_text('// '+self.label+'\n')
        self.assertTrue(repository.check_active_identifiers(self.root))
        source.unlink()
        (self.root/'metadata/current.json').write_text(json.dumps({'schema':self.label.lower()+'.test.v1'}))
        self.assertTrue(repository.check_active_identifiers(self.root))

    def test_active_paths_are_checked_and_pinned_vendor_content_is_excluded(self):
        (self.root/'third_party').mkdir()
        (self.root/'third_party/vendor.txt').write_text(self.label)
        self.assertEqual([],repository.check_active_identifiers(self.root))
        (self.root/'src'/(self.label+'.hpp')).write_text('#pragma once\n')
        self.assertTrue(repository.check_active_identifiers(self.root))

    def test_namespace_boundary_uses_the_owned_root(self):
        for folder in ['include','tests','third_party']:(self.root/folder).mkdir()
        path=self.root/'src/module.cpp'
        for namespace,accepted in [('preloader_l::module',True),('upstream::module',False)]:
            payload=('namespace '+namespace+' {}\n').encode('utf-8');path.write_bytes(payload)
            (self.root/'metadata/SOURCE_FILES.json').write_text(json.dumps({'files':{'src/module.cpp':{'bytes':len(payload),'sha256':inventory.digest(payload)}}}))
            self.assertEqual(accepted,source_verification.verify(self.root)['status']=='PASS')

    def test_source_sizes_are_required_integer_byte_counts(self):
        for folder in ['include','tests','third_party']:(self.root/folder).mkdir()
        # UTF-8 and CRLF distinguish bytes from characters and normalized text.
        payload=b'// \xc3\xa9\r\nnamespace preloader_l::fixture {}\r\n'
        (self.root/'src/module.cpp').write_bytes(payload)
        manifest=self.root/'metadata/SOURCE_FILES.json'
        entry={'bytes':len(payload),'sha256':inventory.digest(payload)}
        def verify(record):
            manifest.write_text(json.dumps({'files':{'src/module.cpp':record}}))
            return source_verification.verify(self.root)
        self.assertEqual('PASS',verify(entry)['status'])
        for size in [len(payload)-1,len(payload)+1,len(payload.decode('utf-8')),
                     -1,True,float(len(payload)),str(len(payload)),None]:
            with self.subTest(size=size):
                result=verify(dict(entry,bytes=size))
                self.assertEqual('FAIL',result['status'])
                self.assertTrue(any('source size' in error.lower() for error in result['errors']))
                self.assertFalse(any('Source differs:' in error for error in result['errors']))
        self.assertEqual('FAIL',verify({'sha256':entry['sha256']})['status'])
        self.assertEqual('PASS',verify(entry)['status'])


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / 'source'
        (self.root / 'metadata').mkdir(parents=True)
        (self.root / 'src').mkdir()
        (self.root / 'src/value.cpp').write_text('int value() { return 7; }\n')
        (self.root / 'CMakeLists.txt').write_text('fixture only\n')
        (self.root / 'metadata/REPOSITORY_CONTRACT.json').write_text(json.dumps({
            'required_files': ['CMakeLists.txt'], 'source_directories': ['src', 'metadata']}))
        inventory.write_manifest(self.root)
        self.zip = self.base / 'source.zip'

    def test_flat_roundtrip_and_determinism(self):
        first = packaging.package(self.root, self.zip)
        second = packaging.package(self.root, self.base / 'second.zip')
        self.assertEqual(first['sha256'], second['sha256'])
        self.assertEqual(first['sha256'] + '  source.zip\n', Path(first['sha256_file']).read_text())
        self.assertEqual(second['sha256'] + '  second.zip\n', Path(second['sha256_file']).read_text())
        self.assertEqual([], falcion.check(self.root, self.zip))
        with zipfile.ZipFile(self.zip) as archive:
            self.assertIn('CMakeLists.txt', archive.namelist())
            self.assertEqual({'CMakeLists.txt','src','metadata'}, {Path(name).parts[0] for name in archive.namelist()})


    def test_checksum_commit_failure_retains_previous_complete_package(self):
        previous = packaging.package(self.root, self.zip)
        sidecar = Path(previous['sha256_file'])
        original_zip, original_sidecar = self.zip.read_bytes(), sidecar.read_bytes()
        (self.root / 'src/value.cpp').write_text('int value() { return 9; }\n')
        inventory.write_manifest(self.root)

        replace = packaging.os.replace
        def reject_checksum(source, destination):
            if Path(destination) == sidecar:
                raise OSError('injected checksum publication error')
            return replace(source, destination)

        with patch.object(packaging.os, 'replace', side_effect=reject_checksum):
            with self.assertRaisesRegex(OSError, 'injected checksum publication error'):
                packaging.package(self.root, self.zip)
        self.assertEqual(original_zip, self.zip.read_bytes())
        self.assertEqual(original_sidecar, sidecar.read_bytes())
        self.assertFalse(list(self.base.glob('.*.partial')))
        self.assertFalse(list(self.base.glob('.*.rollback')))

        replacement = packaging.package(self.root, self.zip)
        self.assertNotEqual(previous['sha256'], replacement['sha256'])
        self.assertEqual(replacement['sha256'] + '  source.zip\n', sidecar.read_text())

    def test_checksum_commit_failure_without_old_pair_removes_new_zip(self):
        sidecar = Path(str(self.zip) + '.sha256')
        replace = packaging.os.replace
        def reject_checksum(source, destination):
            if Path(destination) == sidecar:
                raise OSError('injected checksum publication error')
            return replace(source, destination)
        with patch.object(packaging.os, 'replace', side_effect=reject_checksum):
            with self.assertRaisesRegex(OSError, 'injected checksum publication error'):
                packaging.package(self.root, self.zip)
        self.assertFalse(self.zip.exists())
        self.assertFalse(sidecar.exists())
        self.assertFalse(list(self.base.glob('.*.partial')))

    def test_incomplete_existing_pair_is_not_overwritten(self):
        self.zip.write_bytes(b'old ZIP without receipt')
        with self.assertRaisesRegex(ValueError, 'incomplete source ZIP/checksum pair'):
            packaging.package(self.root, self.zip)
        self.assertEqual(b'old ZIP without receipt', self.zip.read_bytes())
        self.zip.unlink()
        sidecar = Path(str(self.zip) + '.sha256')
        sidecar.write_text('orphaned receipt\n')
        with self.assertRaisesRegex(ValueError, 'incomplete source ZIP/checksum pair'):
            packaging.package(self.root, self.zip)
        self.assertEqual('orphaned receipt\n', sidecar.read_text())

    def test_preexisting_package_partials_cannot_redirect_writes(self):
        victim = self.base / 'unrelated-fixture.txt'
        victim.write_bytes(b'unmodified audit fixture')
        for name in ('source.zip.partial', 'source.zip.sha256.partial'):
            with self.subTest(partial=name):
                unsafe = self.base / name
                try:
                    unsafe.symlink_to(victim)
                except (OSError, NotImplementedError):
                    self.skipTest('Symlink fixture unavailable on this host')
                try:
                    with self.assertRaisesRegex(ValueError, 'Unsafe existing source package partial path'):
                        packaging.package(self.root, self.zip)
                    self.assertEqual(b'unmodified audit fixture', victim.read_bytes())
                    self.assertFalse(self.zip.exists())
                    self.assertFalse(list(self.base.glob('.source.zip.*.partial')))
                finally:
                    unsafe.unlink()

    def test_symlinked_package_output_is_rejected_without_modifying_target(self):
        victim = self.base / 'unrelated-fixture.txt'
        victim.write_bytes(b'unchanged output target')
        try:
            self.zip.symlink_to(victim)
        except (OSError, NotImplementedError):
            self.skipTest('Symlink fixture unavailable on this host')
        with self.assertRaisesRegex(ValueError, 'symlinked source ZIP'):
            packaging.package(self.root, self.zip)
        self.assertEqual(b'unchanged output target', victim.read_bytes())
        self.assertTrue(self.zip.is_symlink())

    def test_source_drift_is_not_automatically_legitimized(self):
        (self.root / 'src/value.cpp').write_text('changed')
        with self.assertRaises(ValueError):
            packaging.package(self.root, self.zip)
        self.assertFalse(self.zip.exists())


    def test_source_manifest_schema_and_release_classification_fail_closed(self):
        manifest_path = self.root / inventory.MANIFEST
        original = manifest_path.read_bytes()
        content = json.loads(original)
        self.assertEqual(inventory.MANIFEST_SCHEMA, content['schema'])
        self.assertNotIn('classification', content)
        self.assertEqual([], inventory.verify_manifest(self.root))

        for mutate, expected in (
                ({'schema': 'unexpected.audit.schema'}, 'Unrecognized active source manifest schema'),
                ({'classification': 'RELEASE'}, 'Release classification does not belong')):
            with self.subTest(mutate=mutate):
                modified = dict(content)
                modified.update(mutate)
                manifest_path.write_text(json.dumps(modified) + '\n', encoding='utf-8')
                self.assertTrue(any(expected in message
                                    for message in inventory.verify_manifest(self.root)))
                manifest_path.write_bytes(original)
        self.assertEqual([], inventory.verify_manifest(self.root))

    def test_unrecorded_source_is_rejected(self):
        (self.root / 'src/extra.cpp').write_text('unexpected')
        self.assertTrue(inventory.verify_manifest(self.root))

    def test_embedded_archives_and_binaries_are_rejected(self):
        for name in ['input.zip', 'input.tar.gz', 'old.dll', 'old.pdb']:
            path = self.root / 'src' / name
            path.write_bytes(b'payload')
            with self.assertRaises(ValueError):
                inventory.source_files(self.root)
            path.unlink()

    def test_package_tamper_and_private_manifest_are_rejected(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            members = {n: archive.read(n) for n in archive.namelist()}
        members['src/value.cpp'] = b'tampered'
        manifest = json.loads(members[inventory.MANIFEST])
        manifest['files']['src/value.cpp'] = inventory.digest(members['src/value.cpp'])
        members[inventory.MANIFEST] = json.dumps(manifest).encode()
        with zipfile.ZipFile(self.zip, 'w') as archive:
            for name, data in members.items():
                archive.writestr(name, data)
        errors = falcion.check(self.root, self.zip)
        self.assertIn('Package/source manifest identity mismatch', errors)
        self.assertTrue(any('src/value.cpp' in e for e in errors))

    def test_unsafe_archive_members(self):
        for name in ['../escape', '/absolute', 'C:/drive', 'dir\\ambiguous', 'a//b', './a', 'a/../b']:
            with self.subTest(name=name), self.assertRaises(ValueError):
                inventory.safe_member(name)


    def test_windows_unsafe_component_names(self):
        invalid = [
            'dir/file.', 'dir/file ', 'dir/file:stream', 'dir/CON.txt',
            'dir/con', 'dir/NUL.cfg', 'dir/COM9.log', 'dir/LPT3',
            'dir/COM\u00b9.log', 'dir/LPT\u00b3.tmp', 'dir/CONIN$',
            'dir/file<bad>.txt', 'dir/wild?.txt', 'dir/wild*.txt',
            'dir/double"quote', 'dir/pipe|name',
        ]
        for name in invalid:
            with self.subTest(member=name), self.assertRaisesRegex(ValueError, 'Unsafe Windows package path'):
                inventory.safe_member(name)
        valid = [
            'docs/readme.md', 'src/value.cpp', 'dir/.gitignore',
            'dir/COM10.txt', 'dir/auxiliary.txt', 'dir/file..name',
            'dir/normal name.txt', 'dir/non-ascii-\u00e7.txt',
        ]
        for name in valid:
            with self.subTest(member=name):
                self.assertEqual(name, inventory.safe_member(name))

    def test_manifest_consistent_windows_invalid_zip_is_not_published(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip) as archive:
            original = {name: archive.read(name) for name in archive.namelist()}
        for index, unsafe in enumerate([
                'docs/audit_portability/file:stream', 'docs/CON.txt', 'docs/trailing.']):
            members = dict(original)
            payload = b'cross-platform filename regression'
            manifest = json.loads(members[inventory.MANIFEST])
            manifest['files'][unsafe] = inventory.digest(payload)
            members[unsafe] = payload
            members[inventory.MANIFEST] = (json.dumps(manifest, sort_keys=True) + '\n').encode('utf-8')
            mutant = self.base / f'windows-name-mutant-{index}.zip'
            with zipfile.ZipFile(mutant, 'w') as archive:
                for name, data in sorted(members.items()):
                    info = zipfile.ZipInfo(name)
                    info.create_system = 3
                    info.external_attr = 0o100644 << 16
                    info.compress_type = zipfile.ZIP_DEFLATED
                    archive.writestr(info, data)
            output = self.base / f'extracted-{index}'
            with self.subTest(member=unsafe), self.assertRaisesRegex(ValueError, 'Unsafe Windows package path'):
                extraction.extract(mutant, output)
            self.assertFalse(output.exists())
        self.assertFalse(any(p.name.startswith('.surge-extract-') for p in self.base.iterdir()))

    def test_casefold_collision(self):
        (self.root / 'src/VALUE.cpp').write_text('collision')
        if len(list((self.root / 'src').glob('*'))) < 2:
            self.skipTest('Host filesystem already rejects case-fold coexistence')
        with self.assertRaises(ValueError):
            inventory.source_files(self.root)

    def test_source_symlink(self):
        link = self.root / 'src/alias.cpp'
        try:
            link.symlink_to(self.root / 'src/value.cpp')
        except (OSError, NotImplementedError):
            self.skipTest('Host cannot create test symlinks')
        with self.assertRaises(ValueError):
            inventory.source_files(self.root)

    def test_no_product_zip_inside_source(self):
        with self.assertRaises(ValueError):
            packaging.package(self.root, self.root / 'bundle.zip')

    def test_duplicate_package_member(self):
        packaging.package(self.root, self.zip)
        with zipfile.ZipFile(self.zip, 'a') as archive:
            import warnings
            with warnings.catch_warnings():
                warnings.simplefilter('ignore', UserWarning)
                archive.writestr('src/value.cpp', 'duplicate')
        self.assertTrue(any('Duplicate' in e for e in falcion.check(self.root, self.zip)))


class SourceQaReportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        # Windows temp paths may use an 8.3 alias; match the resolved ROOT
        # used by the real QA entrypoint before comparing output ancestry.
        self.root = Path(self.temp.name).resolve()
        (self.root / 'src').mkdir()
        (self.root / 'build').mkdir()
        self.source = self.root / 'src/sentinel.cpp'
        self.source.write_text('owned source sentinel\n')
        self.marker = self.root / 'checks-ran'
        # One actual disposable child process; never recursively run project QA.
        (self.root / 'check.py').write_text(
            'from pathlib import Path\n'
            'Path(__file__).with_name("checks-ran").write_text("ran\\n")\n')

    def run_fixture(self, report):
        with patch.object(source_qa, 'ROOT', self.root), \
             patch.object(source_qa, 'CHECKS', [('fixture check', ['check.py'])]), \
             patch.object(sys, 'argv', ['run_source_qa.py', '--report', str(report)]), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return source_qa.main()

    def test_hardlinked_report_preserves_source_inode(self):
        report = self.root / 'build/report.json'
        os.link(self.source, report)
        before = self.source.read_bytes()
        self.assertEqual(0, self.run_fixture(report))
        self.assertEqual(before, self.source.read_bytes())
        self.assertFalse(os.path.samefile(self.source, report))
        self.assertEqual('PASS_SOURCE_QA', json.loads(report.read_text())['result'])
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_invalid_report_is_rejected_before_checks(self):
        directory = self.root / 'build/directory'
        directory.mkdir()
        blocker = self.root / 'build/not-a-directory'
        blocker.write_text('preserved parent file\n')
        for report in (self.source, directory, blocker / 'report.json'):
            with self.subTest(report=report), self.assertRaises(SystemExit) as error:
                self.run_fixture(report)
            self.assertEqual(2, error.exception.code)
            self.assertFalse(self.marker.exists())
            self.assertEqual('owned source sentinel\n', self.source.read_text())
        self.assertEqual([], list(directory.iterdir()))
        self.assertEqual('preserved parent file\n', blocker.read_text())

    def test_missing_parents_receive_complete_report(self):
        report = self.root / 'build/qa/nested/report.json'
        self.assertEqual(0, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('surge.source-qa.v1', data['schema'])
        self.assertEqual('NOT_RUN', data['native_game_acceptance'])
        self.assertEqual(0, data['checks'][0]['exit_code'])
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_replace_failure_preserves_previous_report(self):
        report = self.root / 'build/report.json'
        report.write_bytes(b'previous complete report\n')
        with patch.object(os, 'replace', side_effect=PermissionError('fixture replace failure')):
            with self.assertRaises(PermissionError):
                self.run_fixture(report)
        self.assertEqual(b'previous complete report\n', report.read_bytes())
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_partial_write_failure_preserves_previous_report(self):
        report = self.root / 'build/report.json'
        report.write_bytes(b'previous complete report\n')
        def fail_write(value, output, **kwargs):
            output.write('{')
            raise OSError('fixture partial write failure')
        with patch.object(source_qa.json, 'dump', side_effect=fail_write):
            with self.assertRaises(OSError):
                self.run_fixture(report)
        self.assertEqual(b'previous complete report\n', report.read_bytes())
        self.assertTrue(self.marker.exists())
        self.assertEqual([report], list(report.parent.iterdir()))

    def test_failed_check_retains_nonzero_result_and_report(self):
        with (self.root / 'check.py').open('a') as output:
            output.write('raise SystemExit(7)\n')
        report = self.root / 'build/report.json'
        self.assertEqual(1, self.run_fixture(report))
        data = json.loads(report.read_text())
        self.assertEqual('FAIL', data['result'])
        self.assertEqual(7, data['checks'][0]['exit_code'])
        self.assertEqual([report], list(report.parent.iterdir()))


class BranchPushGateTests(unittest.TestCase):
    def test_real_pushes_preserve_gates_for_temporary_audits_and_research(self):
        # All refs, including main, belong to this disposable local fixture.
        # The QA stub records invocation; it does not impersonate project QA.
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            checkout, remote = base / 'checkout', base / 'remote.git'
            checkout.mkdir()
            # A real pre-push hook exports repository-local Git variables.
            # Do not let them redirect fixture commands into the source repo.
            environment = os.environ.copy()
            local_names = subprocess.run(['git', 'rev-parse', '--local-env-vars'],
                                         cwd=ROOT, capture_output=True, text=True,
                                         check=True, timeout=30).stdout.splitlines()
            for name in local_names:
                environment.pop(name, None)

            def git(*args, accept=True, cwd=checkout):
                result = subprocess.run(['git', '-C', str(cwd), *args],
                                        capture_output=True, text=True, timeout=30,
                                        env=environment)
                if accept:
                    self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                return result

            git('init', '--initial-branch=fix/fixture')
            git('config', 'user.name', 'Push gate fixture')
            git('config', 'user.email', 'push-gate@example.invalid')
            git('config', 'core.autocrlf', 'false')
            git('config', 'commit.gpgsign', 'false')
            self.assertEqual(checkout.resolve(), Path(git('rev-parse', '--show-toplevel').stdout.strip()).resolve())
            hooks = checkout / '.github/hooks'
            hooks.mkdir(parents=True)
            hook = hooks / 'pre-push'
            hook.write_bytes((ROOT / '.github/hooks/pre-push').read_bytes())
            hook.chmod(hook.stat().st_mode | 0o111)
            (checkout / '.gitignore').write_text('build/\n')
            (checkout / 'scripts').mkdir()
            (checkout / 'scripts/run_source_qa.py').write_text(
                'from pathlib import Path\n'
                'p = Path("build/qa-ran")\n'
                'p.parent.mkdir(parents=True, exist_ok=True)\n'
                'p.write_text("fixture QA invoked")\n')
            git('add', '.')
            git('update-index', '--chmod=+x', '.github/hooks/pre-push')
            git('commit', '-m', 'Seed disposable gate fixture')
            seed = git('rev-parse', 'HEAD').stdout.strip()
            git('init', '--bare', str(remote))
            git('remote', 'add', 'origin', str(remote))
            git('push', 'origin', 'HEAD:refs/heads/seed')
            for name in ['main', 'bugfix', 'fix/done']:
                git('update-ref', 'refs/heads/' + name, seed, cwd=remote)
            git('config', 'core.hooksPath', hooks.as_posix())
            (checkout / 'change.txt').write_text('reviewed fixture change\n')
            git('add', 'change.txt')
            git('commit', '-m', 'Prepare next fixture commit')
            head = git('rev-parse', 'HEAD').stdout.strip()
            marker = checkout / 'build/qa-ran'

            for name in ['bugfix', 'fix/done']:
                with self.subTest(deletion=name):
                    git('push', 'origin', '--delete', name)
                    self.assertNotEqual(0, git('show-ref', '--verify', 'refs/heads/' + name,
                                              cwd=remote, accept=False).returncode)
                    self.assertFalse(marker.exists(), 'pure deletion must not run QA')

            for refspec, message in [
                    ('HEAD:refs/heads/bugfix', 'shared bugfix branch is retired'),
                    ('HEAD:refs/heads/main', 'main is updated only by owner-reviewed PR merges'),
                    (':refs/heads/main', 'main is updated only by owner-reviewed PR merges'),
                    (seed + ':refs/heads/fix/old-tip', 'check out the commit being pushed')]:
                with self.subTest(rejection=refspec):
                    rejected = git('push', 'origin', refspec, accept=False)
                    self.assertNotEqual(0, rejected.returncode)
                    self.assertIn(message, rejected.stderr)
                    self.assertFalse(marker.exists(), 'rejection must happen before QA')
            self.assertEqual(seed, git('rev-parse', 'refs/heads/main', cwd=remote).stdout.strip())

            git('push', 'origin', 'HEAD:refs/heads/fix/new-task')
            self.assertTrue(marker.exists(), 'fresh PR branch must run the QA gate')
            self.assertEqual(head, git('rev-parse', 'refs/heads/fix/new-task', cwd=remote).stdout.strip())
            # The push hook checks source integrity, not the branch's retention
            # designation: a routine audit is still a temporary PR head.
            for name in ('audit/one-shot-finding', 'audit/owner-approved-campaign',
                         'research/reconstruction-package'):
                with self.subTest(project_branch=name):
                    marker.unlink()
                    git('push', 'origin', 'HEAD:refs/heads/' + name)
                    self.assertTrue(marker.exists(), 'every PR head must pass the same QA gate')
                    self.assertEqual(head, git('rev-parse', 'refs/heads/' + name,
                                               cwd=remote).stdout.strip())
            marker.unlink()
            git('push', 'origin', '--delete', 'audit/one-shot-finding')
            self.assertNotEqual(0, git('show-ref', '--verify',
                                      'refs/heads/audit/one-shot-finding',
                                      cwd=remote, accept=False).returncode)
            for retained in ('audit/owner-approved-campaign',
                             'research/reconstruction-package'):
                self.assertEqual(head, git('rev-parse', 'refs/heads/' + retained,
                                           cwd=remote).stdout.strip())
            self.assertFalse(marker.exists(), 'deleting a temporary audit head must not run QA')
            (checkout / 'change.txt').write_text('uncommitted fixture change\n')
            rejected = git('push', 'origin', 'HEAD:refs/heads/fix/dirty', accept=False)
            self.assertNotEqual(0, rejected.returncode)
            self.assertIn('commit the reviewed changes', rejected.stderr)
            self.assertFalse(marker.exists())
            self.assertNotEqual(0, git('show-ref', '--verify', 'refs/heads/fix/dirty',
                                      cwd=remote, accept=False).returncode)


class GitFixtureIsolationTests(unittest.TestCase):
    def test_root_fixtures_preserve_an_inherited_callers_repository(self):
        import shutil
        if not shutil.which('git'):
            self.skipTest('Git executable not available')
        with tempfile.TemporaryDirectory() as tmp:
            observer = Path(tmp) / 'observer'
            observer.mkdir()
            # Independent setup: every poisoned selector below points only to
            # this disposable observer, never to the real source checkout.
            environment = {name: value for name, value in os.environ.items()
                           if not name.startswith('GIT_')}
            self.assertFalse(any(name.startswith('GIT_') for name in environment))
            def git(*args):
                result = subprocess.run(['git', '-C', str(observer), *args],
                                        env=environment, capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                return result.stdout
            git('init', '--initial-branch=fixture-owner')
            self.assertEqual(observer.resolve(), Path(git('rev-parse', '--show-toplevel').strip()).resolve())
            git('config', 'user.name', 'Observer fixture')
            git('config', 'user.email', 'observer@example.invalid')
            git('config', 'commit.gpgsign', 'false')
            sentinel = observer / 'observer.txt'
            sentinel.write_bytes(b'owned observer sentinel\n')
            git('add', 'observer.txt')
            git('commit', '-m', 'Seed disposable observer')
            metadata = observer / '.git'
            def snapshot():
                return {
                    'head': git('rev-parse', 'HEAD'),
                    'refs': git('show-ref'),
                    'worktrees': git('worktree', 'list', '--porcelain'),
                    'config': (metadata / 'config').read_bytes(),
                    'index': (metadata / 'index').read_bytes(),
                    'objects': {p.relative_to(metadata).as_posix(): p.read_bytes()
                                for p in (metadata / 'objects').rglob('*') if p.is_file()},
                }
            before = snapshot()
            poisoned = dict(environment, GIT_DIR=str(metadata),
                            GIT_WORK_TREE=str(observer), GIT_COMMON_DIR=str(metadata),
                            GIT_INDEX_FILE=str(metadata / 'index'),
                            GIT_OBJECT_DIRECTORY=str(metadata / 'objects'))
            result = subprocess.run(
                [sys.executable, '-B', str(ROOT / 'tests/infrastructure/test_root_ownership.py'),
                 'RepositoryRootOwnership.test_real_git_worktree_gitfile_is_allowed',
                 'RepositoryRootOwnership.test_borrowed_valid_worktree_redirect_does_not_claim_another_root'],
                cwd=ROOT, env=poisoned, capture_output=True, text=True, timeout=45)
            # Check preservation before checking the child result: even a
            # rejected fixture run must never mutate the caller's repository.
            self.assertEqual(before, snapshot())
            self.assertEqual(b'owned observer sentinel\n', sentinel.read_bytes())
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertIn('Ran 2 tests', result.stderr)
            self.assertNotIn('skipped', result.stderr)


class GovernanceTests(unittest.TestCase):
    def test_resource_assets_require_exact_hash_without_dropping_versioninfo(self):
        import hashlib
        policy=json.loads((ROOT/'tests/support/golden/metadata/MUSTAVEX_GOLDEN_METADATA.json').read_text())['required']
        payload=b'owner asset fixture'
        policy['additional_resource_assets']=[{'path':[10,101,1033],'sha256':hashlib.sha256(payload).hexdigest()}]
        # Invalid VERSIONINFO remains an error; a new asset never bypasses it.
        leaves=[([16,1,1033],b'',0),([10,101,1033],payload,0)]
        errors=mustavex.validate_resource_leaves(leaves,policy)
        self.assertIn('invalid VERSIONINFO root',errors)
        self.assertFalse(any('allowlist' in e or 'byte pin' in e for e in errors))
        bad=[leaves[0],([10,101,1033],b'tampered',0)]
        self.assertTrue(any('byte pin' in e for e in mustavex.validate_resource_leaves(bad,policy)))
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves[1:],policy)))
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves+[leaves[1]],policy)))
        # MSVC's automatic DLL manifest is not an approved third product asset.
        manifest=([24,2,1033],b'<assembly/>',0)
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(leaves+[manifest],policy)))

    def test_pending_product_target_cannot_mutate_binary(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'original.dll'
            path.write_bytes(b'unchanged input; not a PE fixture')
            before = path.read_bytes()
            cp = subprocess.run([sys.executable, '-B', str(ROOT / 'scripts/ursynex_golden_finalizer.py'), str(path)], capture_output=True)
            self.assertNotEqual(0, cp.returncode)
            self.assertEqual(before, path.read_bytes())
            pending_root = Path(tmp) / 'pending-source'
            (pending_root / 'metadata').mkdir(parents=True)
            for name in golden.POLICIES:
                (pending_root / 'metadata' / name).write_text(json.dumps({'activation':'PENDING_PRODUCT_CONTRACT'}))
            report = golden.pipeline(pending_root, path, Path(tmp), pending_root / 'build/test-golden-blocked')
            self.assertEqual('BLOCKED_TARGET_SEMANTICS', report['result'])
            self.assertEqual([], report['steps'])
            self.assertEqual(before, path.read_bytes())

    def test_activation_cannot_hide_missing_product_fields(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            for name in golden.POLICIES:
                (root / 'metadata' / name).write_text(json.dumps({'activation':'READY_TARGET'}))
            self.assertEqual(4, len(golden.pending_targets(root)))

    def test_product_targets_are_configuration_ready_not_native_acceptance(self):
        self.assertEqual([], golden.pending_targets(ROOT))
        corvexon = load_target(ROOT, 'CORVEXON_GOLDEN_TRAIL.json')
        abi = json.loads((ROOT / 'metadata/PUBLIC_DLL_ABI.json').read_text())['exports']
        self.assertEqual({n:v['ordinal'] for n,v in abi.items()}, {v['name']:v['ordinal'] for v in corvexon['shipping_exports']})
        self.assertNotIn('OLE32.DLL', corvexon['import_policy']['allowed_modules'])
        self.assertIn('CRYPT32.DLL', corvexon['import_policy']['required_modules'])
        self.assertEqual(['CryptQueryObject'],corvexon['import_policy']['restricted_named_imports']['CRYPT32.DLL'])
        resource = load_target(ROOT, 'MUSTAVEX_GOLDEN_METADATA.json')
        self.assertEqual([], resource['required']['additional_resource_assets'])
        png_leaves=[([16,1,1033],b'',0),([10,101,1033],b'png',0)]
        self.assertTrue(any('allowlist' in e for e in mustavex.validate_resource_leaves(png_leaves,resource['required'])))

    def test_avatar_generator_refuses_tampered_input_before_output(self):
        import shutil
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            contract = json.loads((ROOT / 'metadata/AVATAR_DATA_CONTRACT.json').read_text())
            (root / 'metadata/AVATAR_DATA_CONTRACT.json').write_text(json.dumps(contract))
            for spec in contract['assets'].values():
                target = root / spec['source']
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / spec['source'], target)
            output=root / 'generated/avatar_data.hpp'
            avatar_data.generate(root,output)
            before=output.read_bytes()
            profile=root / contract['assets']['profile']['source']
            profile.write_bytes(profile.read_bytes()+b'tampered')
            with self.assertRaises(ValueError):
                avatar_data.generate(root,output)
            self.assertEqual(before,output.read_bytes())

    def test_fixture_is_never_a_product_target(self):
        with self.assertRaises(PolicyPending):
            load_target(ROOT / 'tests/support/golden', 'URSYNEX_GOLDEN_FINALIZER.json')

    def test_pipeline_order_and_failure_exit(self):
        for failed_stage in range(6):
            seen = []
            def run(command):
                index = len(seen)
                seen.append(command)
                return {'exit_code': 37 if index == failed_stage else 0, 'output': ''}
            result = golden.execute_steps([(name, i) for i, name in enumerate(golden.ORDER)], run)
            self.assertEqual(list(range(failed_stage+1)), seen)
            self.assertEqual(37, result[-1]['exit_code'])

    def test_finalizer_failed_write_keeps_original_and_cleans_temp(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'original.dll'
            path.write_bytes(b'original bytes')
            with patch.object(finalizer, '_write_file_fully', side_effect=OSError('injected disk write failure')):
                with self.assertRaises(OSError):
                    finalizer.atomic_replace_bytes(path, b'changed bytes')
            self.assertEqual(b'original bytes', path.read_bytes())
            self.assertEqual([path], list(Path(tmp).iterdir()))

    def test_unowned_metadata_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'metadata/METADATA_OWNERSHIP.json').write_text(json.dumps({'files': {
                'METADATA_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'registry', 'role': 'CURRENT_CONTRACT'},
                'SCRIPT_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'script registry', 'role': 'CURRENT_CONTRACT'}},
                'schema': ownership.METADATA_SCHEMA}))
            (root / 'metadata/SCRIPT_OWNERSHIP.json').write_text(json.dumps({'schema': ownership.SCRIPT_SCHEMA, 'files': {}}))
            self.assertEqual([], ownership.check(root))
            (root / 'metadata/unowned.json').write_text('{}')
            self.assertTrue(any('Unowned' in e for e in ownership.check(root)))


    def test_ownership_schemas_and_allowed_roles_fail_closed(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'scripts').mkdir()
            (root / 'scripts/audit_tool.py').write_text('pass\n')
            metadata = {
                'schema': ownership.METADATA_SCHEMA,
                'files': {
                    'METADATA_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'metadata registry',
                                                'role': 'CURRENT_CONTRACT'},
                    'SCRIPT_OWNERSHIP.json': {'owner': 'qa', 'purpose': 'script registry',
                                              'role': 'CURRENT_CONTRACT'},
                },
            }
            scripts = {
                'schema': ownership.SCRIPT_SCHEMA,
                'files': {
                    'scripts/audit_tool.py': {'owner': 'qa', 'purpose': 'fixture script',
                                              'role': 'PYTHON_TOOL_OR_MODULE'},
                },
            }
            metadata_path = root / 'metadata/METADATA_OWNERSHIP.json'
            script_path = root / 'metadata/SCRIPT_OWNERSHIP.json'
            def write_records():
                metadata_path.write_text(json.dumps(metadata))
                script_path.write_text(json.dumps(scripts))
            write_records()
            self.assertEqual([], ownership.check(root))
            for ledger, key in ((metadata, 'metadata'), (scripts, 'script')):
                ledger['schema'] = 'unexpected.audit.schema'
                write_records()
                self.assertTrue(any('Unrecognized ' + key + ' ownership schema' in error
                                    for error in ownership.check(root)))
                ledger['schema'] = (ownership.METADATA_SCHEMA if key == 'metadata'
                                    else ownership.SCRIPT_SCHEMA)
                write_records()
            for ledger, name in ((metadata, 'METADATA_OWNERSHIP.json'),
                                 (scripts, 'scripts/audit_tool.py')):
                record = ledger['files'][name]
                role = record.pop('role')
                write_records()
                self.assertTrue(any('invalid' in error and name in error
                                    for error in ownership.check(root)))
                record['role'] = 'UNKNOWN_ROLE'
                write_records()
                self.assertTrue(any('invalid' in error and name in error
                                    for error in ownership.check(root)))
                record['role'] = role
                write_records()
                self.assertEqual([], ownership.check(root))

    def test_launcher_result_and_argument_mutations(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'metadata').mkdir()
            (root / 'scripts/build_orchestration').mkdir(parents=True)
            registry = json.loads((ROOT / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').read_text())
            registry['entrypoints'] = [registry['entrypoints'][0]]
            (root / 'metadata/HUMAN_ENTRYPOINT_REGISTRY.json').write_text(json.dumps(registry))
            entry = registry['entrypoints'][0]
            original = (ROOT / entry['launcher']).read_text()
            (root / entry['automation']).write_text((ROOT / entry['automation']).read_text())
            path = root / entry['launcher']
            path.write_text(original)
            self.assertEqual([], launchers.check(root))
            for token in ['%*', 'exit /b %RC%', 'ForegroundColor Red', 'Exit code: %RC%', 'pause >nul']:
                path.write_text(original.replace(token, 'REMOVED'))
                self.assertTrue(launchers.check(root), token)
            path.write_text(original)
            automation=root/entry['automation']
            automation.write_text(automation.read_text().replace('exit $LASTEXITCODE','exit 0'))
            self.assertTrue(any('masks child exit' in e for e in launchers.check(root)))


class BuildWordingTests(unittest.TestCase):
    def test_active_operator_tools_and_contracts_reject_retired_labels(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            for name in ('scripts/build_orchestration/build.ps1','tools/inspect.py',
                         'cmake/Policy.cmake','.github/workflows/ci.yml',
                         'metadata/POLICY.json','src/platform/module.cpp',
                         'include/preloader_l/platform/module.hpp','CMakeLists.txt'):
                path=root/name
                path.parent.mkdir(parents=True,exist_ok=True)
                for label in ('NOT_RELEASE','LOCAL_GOLDEN_PASS','LOCAL_BUILD_RECEIPT.json',
                              'LOCAL_DIST_CONTRACT.json','Local Golden','local build',
                              'golden-local','READY_LOCAL_TARGET'):
                    path.write_text(label,encoding='utf-8')
                    self.assertEqual(['Obsolete build wording: '+name+':1'],
                                     launchers.check_build_wording(root))
                path.write_text('GOLDEN_PASS\nBUILD_RECEIPT.json\nDIST_RECEIPT.json\n',encoding='utf-8')
                self.assertEqual([],launchers.check_build_wording(root))

    def test_historical_evidence_and_vendor_bytes_are_not_rewritten(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            for name in ('metadata/evidence/OLD.json','docs/evidence/OLD.log',
                         'third_party/vendor/tool.py','tests/fixtures/old.txt'):
                path=root/name
                path.parent.mkdir(parents=True,exist_ok=True)
                path.write_text('NOT_RELEASE; LOCAL_GOLDEN_PASS',encoding='utf-8')
            before={path:path.read_bytes() for path in root.rglob('*') if path.is_file()}
            self.assertEqual([],launchers.check_build_wording(root))
            self.assertEqual(before,{path:path.read_bytes() for path in before})


class DistTests(unittest.TestCase):
    """Synthetic process results test orchestration only; never stand in for MSVC evidence."""
    def setUp(self):
        import shutil
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)/'source'
        (self.root/'metadata').mkdir(parents=True)
        (self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json').write_text('{"fixture_only":true,"files":{}}')
        shutil.copyfile(ROOT/'metadata/DIST_CONTRACT.json',self.root/'metadata/DIST_CONTRACT.json')
        self.build=self.root/'build/golden'
        self.build.mkdir(parents=True)
        self.raw=self.build/'preloader.unsigned.dll'
        self.raw.write_bytes(b'synthetic orchestration fixture, not a PE or MSVC result')
        self.output=self.build/'golden'

    def run_pipeline(self,failed_stage=None,runtime_failure=False,corrupt_runtime=False):
        calls=[]
        def run(command,**kwargs):
            self.assertEqual('1',kwargs['env']['PYTHONDONTWRITEBYTECODE'])
            index=len(calls);calls.append(command)
            if index==6:
                self.assertTrue(command[0].endswith('preloader_l_runtime_host.exe'))
                self.assertEqual('preloader_l.dll',Path(command[-2]).name)
                self.assertEqual('--xml-contracts',command[-1])
                if corrupt_runtime:Path(command[-2]).write_bytes(b'changed during runtime')
            return subprocess.CompletedProcess(command,37 if index==failed_stage or (index==6 and runtime_failure) else 0,'synthetic process result\n','')
        with patch.object(golden,'pending_targets',return_value=[]),patch.object(golden.subprocess,'run',side_effect=run):
            report=golden.pipeline(self.root,self.raw,self.build,self.output)
        return report,calls

    def dist_bytes(self):
        return {p.name:p.read_bytes() for p in (self.root/'dist').iterdir() if p.is_file()}

    def test_success_stages_exact_final_name_hash_receipt_and_original_bytes(self):
        before=self.raw.read_bytes()
        report,calls=self.run_pipeline()
        self.assertEqual('GOLDEN_PASS',report['result'])
        self.assertEqual(7,len(calls))
        target=self.root/'dist/preloader_l.dll'
        self.assertEqual(before,target.read_bytes())
        self.assertEqual(before,self.raw.read_bytes())
        self.assertEqual(golden.sha(target)+'  preloader_l.dll\n',Path(report['dist_sha256_file']).read_text())
        receipt=json.loads(Path(report['dist_receipt']).read_text())
        self.assertEqual(report['sha256'],receipt['sha256'])
        self.assertEqual(golden.sha(self.root/'metadata/ACTIVE_SOURCE_MANIFEST.json'),receipt['source_manifest_sha256'])
        self.assertEqual('surge.dist-receipt.v1',receipt['schema'])
        self.assertEqual('surge.golden.v1',report['schema'])
        for value in (receipt,report):
            self.assertNotIn('classification',value)
            self.assertNotIn('official_release',value)
            self.assertNotIn('library_publication',value)


    def test_golden_output_symlink_cannot_overwrite_external_file(self):
        self.output.mkdir(parents=True, exist_ok=True)
        victim = self.root.parent / 'outside-golden-victim.bin'
        victim.write_bytes(b'original external fixture bytes')
        candidate_hash = golden.sha(self.raw)[:16]
        output_name = 'preloader_l.' + candidate_hash + '.dll'
        artifact = self.output / output_name
        sidecar = Path(str(artifact) + '.sha256')
        for unsafe in (artifact, sidecar):
            with self.subTest(unsafe=unsafe.name):
                try:
                    unsafe.symlink_to(victim)
                except (OSError, NotImplementedError):
                    self.skipTest('Symlink fixtures are unavailable')
                try:
                    with self.assertRaisesRegex(ValueError, 'Unsafe existing Golden output member'):
                        self.run_pipeline()
                    self.assertEqual(b'original external fixture bytes', victim.read_bytes())
                    self.assertTrue(unsafe.is_symlink())
                    self.assertFalse((self.root / 'dist').exists())
                    self.assertFalse(list(self.output.glob('.golden-*.partial')))
                finally:
                    unsafe.unlink()

    def test_golden_output_staging_leaves_no_partial_files(self):
        report, calls = self.run_pipeline()
        self.assertEqual('GOLDEN_PASS', report['result'])
        self.assertEqual(7, len(calls))
        artifact = Path(report['artifact'])
        self.assertEqual(self.raw.read_bytes(), artifact.read_bytes())
        self.assertEqual(golden.sha(artifact) + '  ' + artifact.name + '\n',
                         Path(report['sha256_file']).read_text())
        self.assertFalse(list(self.output.glob('.golden-*.partial')))

    def test_every_golden_failure_preserves_previous_dist_and_skips_runtime(self):
        self.run_pipeline()
        before=self.dist_bytes()
        for stage in range(6):
            with self.subTest(stage=stage):
                report,calls=self.run_pipeline(failed_stage=stage)
                self.assertEqual('FAIL',report['result'])
                self.assertEqual(stage+1,len(calls))
                self.assertNotIn('runtime_check',report)
                self.assertEqual(before,self.dist_bytes())

    def test_runtime_failure_or_changed_candidate_cannot_create_dist(self):
        for corrupt in (False,True):
            with self.subTest(corrupt=corrupt):
                report,_=self.run_pipeline(runtime_failure=not corrupt,corrupt_runtime=corrupt)
                self.assertEqual('FAIL',report['result'])
                self.assertNotEqual(0,report['runtime_check']['exit_code'])
                self.assertFalse((self.root/'dist').exists())

    def test_incomplete_result_cannot_stage_dist(self):
        with self.assertRaises(ValueError):
            golden.stage_dist(self.root,self.raw,{'result':'GOLDEN_PASS','steps':[]})
        self.assertFalse((self.root/'dist').exists())

    def test_replace_failure_restores_previous_bundle(self):
        report,_=self.run_pipeline()
        before=self.dist_bytes()
        artifact=Path(report['artifact']);artifact.write_bytes(b'new synthetic candidate')
        report['sha256']=golden.sha(artifact)
        report['runtime_check'].update(sha256_before=report['sha256'],sha256_after=report['sha256'])
        replace=golden.os.replace
        def fail_sidecar(source,target):
            if Path(source).name=='preloader_l.dll.sha256' and Path(source).parent.name.startswith('.candidate-'):
                raise OSError('injected sidecar commit failure')
            return replace(source,target)
        with patch.object(golden.os,'replace',side_effect=fail_sidecar):
            with self.assertRaises(OSError):golden.stage_dist(self.root,artifact,report)
        self.assertEqual(before,self.dist_bytes())
        self.assertFalse((self.root/'dist/.golden.lock').exists())
        self.assertFalse(list((self.root/'dist').glob('.candidate-*')))

    def test_final_filename_matches_unchanged_hook_filter(self):
        contract=json.loads((ROOT/'metadata/DIST_CONTRACT.json').read_text())
        header=(ROOT/'include/preloader_l/platform/crypt_query.hpp').read_text()
        source=(ROOT/'src/platform/crypt_query.cpp').read_text()
        declaration=re.search(r'\btarget_suffix\s*=\s*u"([^"\n]+)"\s*;',header)
        self.assertIsNotNone(declaration)
        self.assertEqual(declaration.group(1),contract['runtime_dll_name'])
        self.assertIn('target_suffix[j]',source)


class ToolCacheTests(unittest.TestCase):
    def cleanup_fixture(self, base):
        root = base / 'source'
        (root / 'scripts').mkdir(parents=True)
        (root / 'metadata').mkdir()
        (root / 'build').mkdir()
        (root / 'scripts/helper.py').write_text('value=1\n')
        victim = root / 'metadata/keep.json'
        victim.write_bytes(b'owned source sentinel')
        inventory.write_manifest(root)
        payload = root / 'scripts/__pycache__/helper.cpython-313.pyc'
        payload.parent.mkdir()
        payload.write_bytes(b'owned cache')
        command = [sys.executable, '-B', str(ROOT / 'scripts/clear_tool_python_cache.py'),
                   '--root', str(root), '--quiet', '--report']
        return root, victim, payload, command

    def test_cleanup_report_invalid_targets_preserve_sources_and_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            root, victim, payload, command = self.cleanup_fixture(Path(tmp))
            directory = root / 'build/directory'
            directory.mkdir()
            protected = {p: p.read_bytes() for p in
                         [victim, root / inventory.MANIFEST, root / 'scripts/helper.py']}
            for target in [victim, root / inventory.MANIFEST, root / 'scripts/helper.py',
                           root / 'build', directory, root / 'build/missing/report.json',
                           root.parent / 'outside.json', root / 'build/../metadata/keep.json',
                           Path('metadata/keep.json')]:
                with self.subTest(target=str(target)):
                    result = subprocess.run(command + [str(target)], cwd=root,
                                            capture_output=True, text=True)
                    self.assertNotEqual(0, result.returncode)
                    self.assertEqual('', result.stdout)
                    self.assertIn('TOOL_CACHE_CLEANUP: FAIL:', result.stderr)
                    self.assertTrue(payload.exists())
                    for path, before in protected.items():
                        self.assertEqual(before, path.read_bytes())
                    self.assertFalse(list((root / 'build').rglob('*.partial')))
            self.assertFalse((root.parent / 'outside.json').exists())

    def test_cleanup_report_linked_directories_cannot_reach_outside(self):
        import os
        for link_root in (True, False):
            with self.subTest(link_root=link_root), tempfile.TemporaryDirectory() as tmp:
                base = Path(tmp)
                root, victim, payload, command = self.cleanup_fixture(base)
                outside = base / 'outside'
                outside.mkdir()
                sentinel = outside / 'report.json'
                sentinel.write_bytes(b'outside sentinel')
                link = root / 'build' if link_root else root / 'build/redirect'
                if link_root:
                    link.rmdir()
                if os.name == 'nt':
                    made = subprocess.run(['cmd.exe', '/d', '/c', 'mklink', '/J',
                                           str(link), str(outside)], capture_output=True, text=True)
                    self.assertEqual(0, made.returncode, made.stdout + made.stderr)
                else:
                    link.symlink_to(outside, target_is_directory=True)
                try:
                    result = subprocess.run(command + [str(link / 'report.json')],
                                            capture_output=True, text=True)
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn('TOOL_CACHE_CLEANUP: FAIL:', result.stderr)
                    self.assertTrue(payload.exists())
                    self.assertEqual(b'owned source sentinel', victim.read_bytes())
                    self.assertEqual(b'outside sentinel', sentinel.read_bytes())
                    self.assertEqual(['report.json'], sorted(p.name for p in outside.iterdir()))
                finally:
                    if os.name == 'nt':
                        os.rmdir(link)  # Remove the owned junction entry, never its target.
                    else:
                        link.unlink()
                self.assertEqual(b'outside sentinel', sentinel.read_bytes())

    def test_cleanup_report_atomic_replace_preserves_hardlinked_source(self):
        import os
        with tempfile.TemporaryDirectory() as tmp:
            root, victim, payload, command = self.cleanup_fixture(Path(tmp))
            report = root / 'build/report.json'
            os.link(victim, report)
            self.assertTrue(os.path.samefile(victim, report))
            result = subprocess.run(command + [str(report)], capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual('', result.stdout)
            self.assertEqual(b'owned source sentinel', victim.read_bytes())
            self.assertFalse(os.path.samefile(victim, report))
            self.assertFalse(payload.exists())
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],
                             json.loads(report.read_text())['removed'])
            self.assertFalse(list(report.parent.glob('.*.partial')))

    def test_cleanup_report_replace_failure_preserves_previous_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            report = Path(tmp) / 'report.json'
            report.write_bytes(b'previous complete report')
            output = '{"removed": []}\n'
            def fail_replace(source, target):
                self.assertEqual(report, target)
                self.assertEqual(b'previous complete report', report.read_bytes())
                self.assertEqual(output, Path(source).read_text(encoding='utf-8'))
                raise OSError('injected report publication failure')
            with patch.object(tool_cache.os, 'replace', side_effect=fail_replace):
                with self.assertRaisesRegex(OSError, 'injected report publication failure'):
                    tool_cache.write_report(report, output)
            self.assertEqual(b'previous complete report', report.read_bytes())
            self.assertEqual(['report.json'], sorted(p.name for p in report.parent.iterdir()))
            tool_cache.write_report(report, output)
            self.assertEqual(output, report.read_text())
            self.assertEqual(['report.json'], sorted(p.name for p in report.parent.iterdir()))

    def test_quiet_cleanup_keeps_report_and_propagates_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'scripts').mkdir()
            (root/'metadata').mkdir()
            (root/'scripts/helper.py').write_text('value=1\n')
            inventory.write_manifest(root)
            cache=root/'scripts/__pycache__';cache.mkdir()
            (cache/'helper.cpython-313.pyc').write_bytes(b'fixture')
            report=root/'build/PYTHON_CACHE_CLEANUP.json';report.parent.mkdir()
            command=[sys.executable,'-B',str(ROOT/'scripts/clear_tool_python_cache.py'),
                     '--root',str(root),'--quiet','--report',str(report)]
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertEqual(0,cp.returncode,cp.stderr)
            self.assertEqual('',cp.stdout)
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],json.loads(report.read_text())['removed'])
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertEqual(0,cp.returncode,cp.stderr)
            self.assertEqual('',cp.stdout)
            self.assertEqual([],json.loads(report.read_text())['removed'])
            (root/inventory.MANIFEST).write_text('invalid json')
            cp=subprocess.run(command,capture_output=True,text=True)
            self.assertNotEqual(0,cp.returncode)
            self.assertIn('TOOL_CACHE_CLEANUP: FAIL:',cp.stderr)


    def test_cleanup_report_rejects_source_destinations_before_mutation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'scripts').mkdir()
            (root / 'metadata').mkdir()
            (root / 'scripts/helper.py').write_text('value=1\n')
            victim = root / 'metadata/METADATA_OWNERSHIP.json'
            victim.write_bytes(b'original active record')
            inventory.write_manifest(root)
            baseline = victim.read_bytes()
            cache = root / 'scripts/__pycache__'
            cache.mkdir()
            payload = cache / 'helper.cpython-313.pyc'
            payload.write_bytes(b'fixture')

            command = [sys.executable, '-B', str(ROOT / 'scripts/clear_tool_python_cache.py'),
                       '--root', str(root), '--quiet', '--report', str(victim)]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(0, result.returncode)
            self.assertIn('beneath source build/', result.stderr)
            self.assertEqual(baseline, victim.read_bytes())
            self.assertTrue(payload.exists())

            build = root / 'build'
            build.mkdir()
            linked = build / 'unsafe.json'
            try:
                linked.symlink_to(victim)
            except (OSError, NotImplementedError):
                pass
            else:
                command[-1] = str(linked)
                result = subprocess.run(command, capture_output=True, text=True)
                self.assertNotEqual(0, result.returncode)
                self.assertEqual(baseline, victim.read_bytes())
                self.assertTrue(payload.exists())

            report = build / 'PYTHON_CACHE_CLEANUP.json'
            command[-1] = str(report)
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(['scripts/__pycache__/helper.cpython-313.pyc'],
                             json.loads(report.read_text())['removed'])
            self.assertEqual(baseline, victim.read_bytes())
            self.assertFalse(list(build.glob('.*.partial')))

    def test_cleanup_removes_only_owned_generated_caches_without_refresh(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/'metadata').mkdir()
            (root/'scripts').mkdir()
            (root/'scripts/helper.py').write_text('value=1\n')
            inventory.write_manifest(root)
            manifest=(root/inventory.MANIFEST).read_bytes()
            cache=root/'scripts/__pycache__';cache.mkdir()
            for name in ['helper.cpython-313.pyc','helper.cpython-312.opt-1.pyc','unowned.cpython-313.pyc','notes.txt']:
                (cache/name).write_bytes(b'fixture')
            removed=tool_cache.clear(root)
            self.assertEqual(['scripts/__pycache__/helper.cpython-312.opt-1.pyc','scripts/__pycache__/helper.cpython-313.pyc'],removed)
            self.assertEqual({'unowned.cpython-313.pyc','notes.txt'},{p.name for p in cache.iterdir()})
            self.assertEqual(manifest,(root/inventory.MANIFEST).read_bytes())
            with self.assertRaises(ValueError):inventory.verify_manifest(root)

    def test_corvexon_nested_children_reproduce_old_cache_and_keep_new_tree_clean(self):
        import os,shutil
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);scripts=root/'scripts';scripts.mkdir();(root/'metadata').mkdir()
            shutil.copyfile(ROOT/'scripts/golden_policy.py',scripts/'golden_policy.py')
            shutil.copyfile(ROOT/'metadata/CORVEXON_GOLDEN_TRAIL.json',root/'metadata/CORVEXON_GOLDEN_TRAIL.json')
            children=['verify_aryra_golden_layout.py','verify_mustavex_golden_metadata.py','check_msvc_shipping_link_command.py']
            for name in children:
                (scripts/name).write_text('import golden_policy\nfrom pathlib import Path\nPath(__file__).with_suffix(".ran").write_text("ran")\n')
            parent=scripts/'verify_corvexon_golden_trail.py'
            current=(ROOT/'scripts/verify_corvexon_golden_trail.py').read_text()
            self.assertEqual(3,current.count("[sys.executable,'-B',"))
            legacy=current.replace("[sys.executable,'-B',","[sys.executable,")
            dll=root/'invalid-synthetic.dll';dll.write_bytes(b'not a PE; failure after the three child processes is expected')
            env=dict(os.environ)
            env.pop('PYTHONDONTWRITEBYTECODE',None);env.pop('PYTHONPYCACHEPREFIX',None)
            for source,expect_cache in [(legacy,True),(current,False)]:
                parent.write_text(source)
                cache=scripts/'__pycache__'
                if cache.exists():shutil.rmtree(cache)
                cp=subprocess.run([sys.executable,'-B',str(parent),str(dll),'--build-dir',str(root/'build')],env=env,capture_output=True,text=True)
                self.assertEqual(1,cp.returncode,cp.stdout+cp.stderr)
                for name in children:self.assertTrue((scripts/name).with_suffix('.ran').is_file())
                self.assertEqual(expect_cache,cache.exists())
                for name in children:(scripts/name).with_suffix('.ran').unlink()


if __name__ == '__main__':
    unittest.main(verbosity=2)
