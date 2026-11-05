# Shared event count source regression

Source-only, NOT RUN. The existing native_message_tests.cpp now checks queue aggregate construction, copy/assignment value snapshots and subsequent mutation of the single selected shared count. These snapshots are metadata values and do not become independent queue owners or source authority.

An actual typed033d70 encrypted event case throws a controlled Failure83 from the existing unlock callback. It requires the shared count and head reset to be already committed, and document cleanup to remain absent. The source algorithm is unchanged; this guards against deferring pop publication to successful unlock or guessing rollback after an unknown outcome.

All prior normal/retry/event/control/retained-prefix cases stay registered. The wrapper requires the selected uint64 atomic implementation to be always lock-free; no test execution or compiler result is claimed. Structural queue/payload access still requires the real queue mutex, and arbitrary unsynchronized writers are outside this projection contract.
