# SPDX-License-Identifier: GPL-3.0-only

"""Permit full CI only for an updated head that includes current main."""
import argparse
from dataclasses import dataclass
from datetime import datetime
import json
import os
from pathlib import Path
import re
import subprocess
from urllib.parse import quote


VALIDATION_LABEL = 'run-full-validation'
VALIDATION_WORKFLOW = '.github/workflows/msvc-reference-graph.yml'


def sync_job_name(number, base, head):
    return 'PR sync #' + str(number) + ': ' + base + ' / ' + head


def _time(value):
    if not isinstance(value, str):
        raise ValueError('GitHub returned an invalid timestamp')
    stamp = datetime.fromisoformat(value.replace('Z', '+00:00'))
    if stamp.tzinfo is None:
        raise ValueError('GitHub returned a timestamp without a timezone')
    return stamp


def _pages(api, endpoint, key):
    # GitHub limits filtered workflow-run searches to 1,000 results. If a
    # receipt ages out, request a new verification instead of guessing.
    for page in range(1, 11):
        items = api(endpoint + '&per_page=100&page=' + str(page))[key]
        if not isinstance(items, list):
            raise ValueError('GitHub returned an invalid result page')
        yield from items
        if len(items) < 100:
            return


def sync_request_verified(api, prefix, event, current, number, base, head):
    opened = _time(current['created_at'])
    requested = _time(event['pull_request']['updated_at'])
    endpoint = (prefix + '/actions/workflows/msvc-reference-graph.yml/runs?'
                'event=workflow_dispatch&head_sha=' + head)
    expected = sync_job_name(number, base, head)
    for run in _pages(api, endpoint, 'workflow_runs'):
        if (run.get('event') != 'workflow_dispatch' or
                run.get('path') != VALIDATION_WORKFLOW or
                run.get('head_sha') != head or
                run.get('head_branch') != current['head']['ref'] or
                run.get('status') != 'completed' or run.get('conclusion') != 'success'):
            continue
        # A premature label must not become eligible while its runner waits
        # and a later verification completes. Re-add it after verification.
        if not (opened < _time(run['created_at']) <= _time(run['updated_at']) <= requested):
            continue
        run_id = run['id']
        if type(run_id) is not int or run_id <= 0:
            raise ValueError('Invalid workflow run identity')
        jobs = _pages(api, prefix + '/actions/runs/' + str(run_id) + '/jobs?filter=latest', 'jobs')
        if any(job.get('name') == expected and job.get('status') == 'completed' and
               job.get('conclusion') == 'success' for job in jobs):
            return True
    return False


@dataclass(frozen=True)
class Decision:
    run_tests: bool
    reason: str
    head_sha: str = ''
    base_sha: str = ''
    changelog_exempt: bool = False


def _sha(value):
    if not isinstance(value, str) or not re.fullmatch(r'[0-9a-f]{40}', value):
        raise ValueError('GitHub returned an invalid commit identity')
    return value


def github_json(endpoint):
    result = subprocess.run(['gh', 'api', endpoint], capture_output=True,
                            text=True, timeout=60)
    if result.returncode:
        raise RuntimeError('GitHub readiness request failed')
    return json.loads(result.stdout)


def decide(event_name, event, repository, sha, ref, api=None):
    if api is None:
        api = github_json
    if event_name == 'workflow_dispatch' and event.get('inputs', {}).get('operation') == 'synchronize-pr':
        return Decision(False, 'Synchronization preparation does not request full tests')
    explicit_pr_request = (event_name == 'pull_request' and
                           event.get('action') == 'labeled' and
                           event.get('label', {}).get('name') == VALIDATION_LABEL)
    if (event_name == 'pull_request' and event.get('action') != 'synchronize' and
            not explicit_pr_request):
        return Decision(False, 'Await main synchronization or manual validation')
    if event_name not in ('pull_request', 'workflow_dispatch'):
        return Decision(False, 'This event does not request full validation')
    if (not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository) or
            any(part in ('.', '..') for part in repository.split('/'))):
        raise ValueError('Invalid repository identity')
    default = event['repository']['default_branch']
    prefix = 'repos/' + repository
    if event_name == 'pull_request':
        pr = event['pull_request']
        head = _sha(pr['head']['sha'])
        if not explicit_pr_request and _sha(event['after']) != head:
            return Decision(False, 'The update payload does not identify the current head')
        number = event['number']
        if type(number) is not int or number <= 0:
            raise ValueError('Invalid pull request identity')
        current = api(prefix + '/pulls/' + str(number))
        if current['state'] != 'open' or current['head']['sha'] != head:
            return Decision(False, 'The requested pull request head is no longer current')
        if current['base']['ref'] != default:
            return Decision(False, 'Full validation requires the default integration branch')
        if explicit_pr_request and not any(label.get('name') == VALIDATION_LABEL
                                           for label in current.get('labels', [])):
            return Decision(False, 'The explicit validation request is no longer present')
    else:
        if not ref.startswith('refs/heads/'):
            return Decision(False, 'Choose a branch for manual validation')
        head = _sha(sha)
        selected = api(prefix + '/git/ref/heads/' + quote(ref[len('refs/heads/'):], safe=''))
        if selected['object']['sha'] != head:
            return Decision(False, 'The selected branch changed after validation was requested')
    base = _sha(api(prefix + '/git/ref/heads/' + quote(default, safe=''))['object']['sha'])
    comparison = api(prefix + '/compare/' + base + '...' + head)
    included = (comparison.get('status') in ('ahead', 'identical') and
                comparison.get('merge_base_commit', {}).get('sha') == base)
    if not included:
        return Decision(False, 'Synchronize this branch with current main before full validation', head, base)
    if explicit_pr_request and not sync_request_verified(api, prefix, event, current,
                                                         number, base, head):
        return Decision(False, 'Verify a subsequent PR synchronization request before adding the validation label',
                        head, base)
    if event_name == 'pull_request' and not explicit_pr_request:
        before = _sha(event['before'])
        previous = api(prefix + '/compare/' + base + '...' + before)
        already_included = (previous.get('status') in ('ahead', 'identical') and
                            previous.get('merge_base_commit', {}).get('sha') == base)
        if already_included:
            return Decision(False, 'Main was already included; request manual validation when ready', head, base)
        previous_base = previous.get('merge_base_commit', {}).get('sha')
        if previous.get('status') not in ('behind', 'diverged') or _sha(previous_base) == base:
            return Decision(False, 'A new integration of current main could not be verified', head, base)
    if event_name == 'pull_request':
        relevant = [current]
    elif head != base:
        query = ('/pulls?state=open&base=' + quote(default, safe='') + '&head=' +
                 quote(repository.split('/')[0] + ':' + ref[len('refs/heads/'):], safe='') +
                 '&per_page=100')
        relevant = api(prefix + query)
    else:
        relevant = []
    exempt = any(pr['state'] == 'open' and pr['head']['sha'] == head and
                 pr['base']['ref'] == default and
                 any(label.get('name') == 'changelog-exempt' for label in pr.get('labels', []))
                 for pr in relevant)
    return Decision(True, 'The selected head includes current main', head, base, exempt)


def record_sync_request(event_name, event, repository, sha, ref, api=None):
    """Verify a later synchronization request without scheduling full tests.

    The successful preparation job is the receipt, scoped by its exact name
    to PR number, current main and head. It is not native/test acceptance.
    """
    if api is None:
        api = github_json
    if event_name != 'workflow_dispatch' or event.get('inputs', {}).get('operation') != 'synchronize-pr':
        raise ValueError('Synchronization preparation requires an explicit manual request')
    if (not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository) or
            any(part in ('.', '..') for part in repository.split('/'))):
        raise ValueError('Invalid repository identity')
    inputs = event['inputs']
    number = inputs['pr_number']
    if not isinstance(number, str) or not re.fullmatch(r'[1-9][0-9]*', number):
        raise ValueError('Choose an open pull request number')
    head = _sha(sha)
    prefix = 'repos/' + repository
    current = api(prefix + '/pulls/' + number)
    default = event['repository']['default_branch']
    if (current['state'] != 'open' or current['head']['sha'] != head or
            current['head']['repo']['full_name'] != repository or
            ref != 'refs/heads/' + current['head']['ref'] or current['base']['ref'] != default):
        raise ValueError('Select the current source branch of an open integration PR')
    if any(label.get('name') == VALIDATION_LABEL for label in current.get('labels', [])):
        raise ValueError('Remove the validation label before preparing a new request')
    base = _sha(api(prefix + '/git/ref/heads/' + quote(default, safe=''))['object']['sha'])
    if _sha(inputs['sync_base']) != base:
        raise ValueError('The requested integration base is no longer current')
    comparison = api(prefix + '/compare/' + base + '...' + head)
    if (comparison.get('status') not in ('ahead', 'identical') or
            comparison.get('merge_base_commit', {}).get('sha') != base):
        raise ValueError('Synchronize with current main before recording the request')
    return Decision(False, 'Synchronization request verified; full tests remain deferred', head, base)


def write_decision(decision, output, summary):
    # Values written to Actions outputs are fixed messages and validated hashes.
    values = {'run_tests': str(decision.run_tests).lower(), 'reason': decision.reason,
              'head_sha': decision.head_sha, 'base_sha': decision.base_sha,
              'changelog_exempt': str(decision.changelog_exempt).lower()}
    if output:
        with Path(output).open('a', encoding='utf-8') as stream:
            for key, value in values.items():
                stream.write(key + '=' + value + '\n')
    if summary:
        with Path(summary).open('a', encoding='utf-8') as stream:
            stream.write('Full validation: ' + decision.reason + '\n\n')
            if decision.head_sha:
                stream.write('Head: `' + decision.head_sha + '`\n\n')
            if decision.base_sha:
                stream.write('Current main: `' + decision.base_sha + '`\n')
    print('CI_READINESS: ' + decision.reason)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--event-name', default=os.environ.get('GITHUB_EVENT_NAME', ''))
    parser.add_argument('--event-path', type=Path, default=os.environ.get('GITHUB_EVENT_PATH'))
    parser.add_argument('--repository', default=os.environ.get('GITHUB_REPOSITORY', ''))
    parser.add_argument('--sha', default=os.environ.get('GITHUB_SHA', ''))
    parser.add_argument('--ref', default=os.environ.get('GITHUB_REF', ''))
    parser.add_argument('--output', default=os.environ.get('GITHUB_OUTPUT'))
    parser.add_argument('--summary', default=os.environ.get('GITHUB_STEP_SUMMARY'))
    parser.add_argument('--record-sync', action='store_true')
    args = parser.parse_args()
    try:
        if args.event_path is None:
            raise ValueError('Missing GitHub event payload')
        event = json.loads(args.event_path.read_text(encoding='utf-8'))
        operation = record_sync_request if args.record_sync else decide
        decision = operation(args.event_name, event, args.repository, args.sha, args.ref)
    except (OSError, ValueError, KeyError, TypeError, RuntimeError, subprocess.TimeoutExpired):
        write_decision(Decision(False, 'Readiness could not be verified; no full test result is available'),
                       args.output, args.summary)
        return 1
    write_decision(decision, args.output, args.summary)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
