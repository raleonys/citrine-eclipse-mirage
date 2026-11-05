# Header ledger growth: test source

Changed only profile_tests.cpp and application-graph graph_tests.cpp. Exact before copies are retained here. Both new cases are registered in their existing normal mains, and all prior cases, including the isolated Graph fiber608 case, remain registered. No compiler, project test, link, native REA session, reconstructed DLL execution or package action occurred.

## Actual completion consumers against rebound views

`strings_registered_header_growth()` uses the existing Parent/Domain fixture and actual Strings. It registers live FrameReadContext::allocation_headers and WriteFinishContext::allocation_headers span fields, then holds260 aligned4095-byte strings simultaneously from initial header capacity256. It checks both registered spans follow the moved ledger.

The controlled error category transfers ownership of string259 into the local message used by actual finish_frame_read_025480, clearing its donor. The real error branch finds the aligned header through the registered FrameReadContext and frees it. Separately, actual finish_write_025a70 retires string258 as a successful queue head through the registered WriteFinishContext. Both headers are beyond the previous256-entry span, so merely retaining the initial span cannot satisfy either consumer. The injected free boundary checks exact raw aligned base and capacity+1+39 extent and calls Strings::free_sized, which delegates to the actual owned Parent free. Metadata liveness drops by exactly one for each consumed payload. Both freed entries are then reused, and all260 live payloads are explicitly released without a remaining Parent buffer.

The local FrameReadApi/category are narrow synthetic logging/free boundaries. The test runs the actual completion algorithms rather than replacing their allocation-header lookup with a bindings() snapshot. It does not install sockets or reconstructed DLL hooks.

## Actual Profile/Graph auto-binding pipeline

`large_event_header_ledger_growth()` selects actual Profile initial headers256 and Graph initial event map64. An encrypted session enqueues260 distinct events larger than5000 bytes before the first event drain, retaining those aligned allocations plus the initial DOWN event. Actual Native033d70 processes261 requests, drains the FIFO and writes261 encrypted response/event pairs. Independent client decryption checks every response and event payload. Existing normal Graph/Profile/Domain retirement must free all allocations and balance allocator/free counts.

This covers actual Profile constructor auto-registration and large successful WriteFinish processing after the header ledger grows. It does not inject a large FrameRead error inside that actual Profile pipeline; the FrameRead error consumer is exercised by the direct registered-context case above. Neither case was compiled or executed, so source coverage is not a passing runtime result.

## Final source hashes

- profile_tests.cpp: `007fd4e0fbb2e7bb9aac01d895778164263f991e6506489f2d8879ec1c1e23cf`
- graph_tests.cpp: `732042f790d54a16472d76398eb570d40e29bbefaddac4a09b8294376fcfe606`
