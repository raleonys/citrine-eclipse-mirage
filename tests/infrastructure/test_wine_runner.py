"""Check real exit propagation and inherited-output lifetime independently of Wine."""
import subprocess
import sys
import time
import unittest
from pathlib import Path

RUNNER = Path(__file__).resolve().parents[2] / 'scripts/run_wine_test.py'


class WineRunner(unittest.TestCase):
    def test_nonzero_and_output_are_preserved(self):
        result = subprocess.run([sys.executable, '-B', str(RUNNER), sys.executable,
                                 '-c', "print('fixture output');raise SystemExit(7)"], capture_output=True)
        self.assertEqual(result.returncode, 7)
        self.assertIn(b'fixture output', result.stdout)
        self.assertIn(b'WINE_PROGRAM_EXIT=7', result.stdout)

    def test_descendant_pipe_does_not_hold_runner(self):
        # The descendant inherits stdout but outlives the program. A communicate
        # pipe would wait for it even though the actual program already exited.
        parent = "import subprocess,sys;subprocess.Popen([sys.executable,'-c','import time;time.sleep(3)']);print('parent done')"
        start = time.monotonic()
        result = subprocess.run([sys.executable, '-B', str(RUNNER), sys.executable, '-c', parent], capture_output=True, timeout=2)
        self.assertEqual(result.returncode, 0)
        self.assertLess(time.monotonic() - start, 2)
        self.assertIn(b'parent done', result.stdout)

    def test_hung_program_is_failure(self):
        result = subprocess.run([sys.executable, '-B', str(RUNNER), '--timeout', '0.1', sys.executable,
                                 '-c', 'import time;time.sleep(3)'], capture_output=True)
        self.assertEqual(result.returncode, 124)


if __name__ == '__main__':
    unittest.main()
