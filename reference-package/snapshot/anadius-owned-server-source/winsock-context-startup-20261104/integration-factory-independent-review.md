# Independent integration/factory test-source review

No blocking source defect found against the two captured `before/` files. No test, child process, compiler, linker or DLL was executed. Primitive ownership tests belong to the parent's separate review.

The integration fixture preserves the existing Host IPv4/IPv6, diagnostic and sticky-prefix cases with WORD `0x0202`, two gates and cleanup after port/gates. New Original mode uses its own passively constructed shared state and exactly matching injected pair, retained through the existing dependency lifetime. Its rejected configurations run before provider acquisition. Its normal actual backend path requires three initialized/deleted gates, registry gate before cold WORD `0x0002` startup, socket retirement, last-owner cleanup before port/gate teardown, and Closed/drained resource assertions. The cleanup wrapper performs the real own WSACleanup but returns SOCKET_ERROR, making ignored Native cleanup status observable.

The cached-failure child makes an unchecked fixture claim before context construction, records 10091 without a physical WSA startup, then requires the checked backend's additional claim to raise native `winsock`/84500 before scheduler creation. Releasing the fixture claim plus an extra explicit nonlast claim proves the failed backend retains its own increment without retry. The separate registry-failure child requires native `mutex`/84428 before any startup/port attempt. Both terminate through ExitProcess after exact source-code/category/address/full-message and retained-prefix assertions, without unwinding the poisoned backend or manufacturing cleanup. Both are registered in default and focused modes.

Factory assertions observe Original/default-module selection and exact import identity after actual configure_native_worker. Occupied profile/state and a nondefault startup pair reject without committing IO/executor/Graph fields or calling the fake startup. Lower native SDK selection and OriginalApplication lifecycle selection alone retain Host. The existing running Resources fixture still uses its explicit Host configure path.

Limits: these files do not link/execute DLL entry.cpp, register its atexit callback, prove actual compiler import-pointer representation, or validate the real DLL's CRT timing. The additive retained native CRT proof and production source review cover those separate boundaries. Message comparisons use a separate native-category oracle, not an intercepted once-only formatting claim. Source counts/order are assertions awaiting an authorized rebuild/run.

| File | Final SHA256 |
|---|---|
| `outputs/anadius-owned-server-source/server_integration_tests.cpp` | `87bf17d709d9a9f508bb3f85489b6ef6ae4ec974f5d0f11db90dc58d0c908e8a` |
| `outputs/anadius-resources-factory-source/factory_tests.cpp` | `992842d88053a6bb70621a080aa4d51132ab39ecf73a1cca5d7bede53f36b093` |
