$ErrorActionPreference = 'Stop'
$moduleRoot = $PSScriptRoot
$workspaceRoot = [IO.Path]::GetFullPath((Join-Path $moduleRoot '../..'))
$compiler = Join-Path $workspaceRoot 'work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64/bin/clang++.exe'
$buildRoot = Join-Path $moduleRoot 'build'
New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null
$flags = @('-std=c++20','-O2','-Wall','-Wextra','-Werror','-static')
$support = @((Join-Path $moduleRoot 'operations.cpp'),(Join-Path $moduleRoot '../anadius-server-source/server.cpp'),(Join-Path $moduleRoot '../anadius-execution-source/execution.cpp'))
$operationExe = Join-Path $buildRoot 'socket_operations_tests.exe'
$nativeExe = Join-Path $buildRoot 'socket_native_tests.exe'
$acceptorExe = Join-Path $buildRoot 'socket_acceptor_tests.exe'
$acceptorNativeExe = Join-Path $buildRoot 'socket_acceptor_native_tests.exe'
$lifecycleExe = Join-Path $buildRoot 'socket_lifecycle_tests.exe'
$lifecycleNativeExe = Join-Path $buildRoot 'socket_lifecycle_native_tests.exe'
$closeExe = Join-Path $buildRoot 'socket_close_tests.exe'
$closingExe = Join-Path $buildRoot 'socket_closing_tests.exe'
$cancellationExe = Join-Path $buildRoot 'socket_cancellation_tests.exe'
$cancellationNativeExe = Join-Path $buildRoot 'socket_cancellation_native_tests.exe'
$recycleExe = Join-Path $buildRoot 'socket_recycle_tests.exe'
$recycleStorageExe = Join-Path $buildRoot 'socket_recycle_storage_tests.exe'
$serviceRegistryExe = Join-Path $buildRoot 'socket_service_registry_tests.exe'
$serviceRegistryIntegrationExe = Join-Path $buildRoot 'socket_service_registry_integration_tests.exe'
& $compiler @flags @support (Join-Path $moduleRoot 'operations_tests.cpp') '-lws2_32' '-o' $operationExe
if ($LASTEXITCODE -ne 0) { throw 'Socket operation test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'native.cpp') (Join-Path $moduleRoot 'native_tests.cpp') '-lws2_32' '-o' $nativeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket native test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'acceptor.cpp') (Join-Path $moduleRoot 'acceptor_tests.cpp') '-lws2_32' '-o' $acceptorExe
if ($LASTEXITCODE -ne 0) { throw 'Socket acceptor test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'acceptor.cpp') (Join-Path $moduleRoot 'acceptor_native.cpp') (Join-Path $moduleRoot 'acceptor_native_tests.cpp') '-lws2_32' '-o' $acceptorNativeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket acceptor native test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'acceptor.cpp') (Join-Path $moduleRoot 'lifecycle.cpp') (Join-Path $moduleRoot 'lifecycle_tests.cpp') '-lws2_32' '-o' $lifecycleExe
if ($LASTEXITCODE -ne 0) { throw 'Socket lifecycle test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'acceptor.cpp') (Join-Path $moduleRoot 'lifecycle.cpp') (Join-Path $moduleRoot 'lifecycle_native.cpp') (Join-Path $moduleRoot 'lifecycle_native_tests.cpp') '-lws2_32' '-o' $lifecycleNativeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket lifecycle native test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'close.cpp') (Join-Path $moduleRoot 'close_tests.cpp') '-lws2_32' '-o' $closeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket close test compilation failed' }
& $compiler @flags @support (Join-Path $moduleRoot 'acceptor.cpp') (Join-Path $moduleRoot 'acceptor_native.cpp') (Join-Path $moduleRoot 'lifecycle.cpp') (Join-Path $moduleRoot 'close.cpp') (Join-Path $moduleRoot 'closing.cpp') (Join-Path $moduleRoot 'closing_tests.cpp') '-lws2_32' '-o' $closingExe
if ($LASTEXITCODE -ne 0) { throw 'Socket closing test compilation failed' }
$cancellationSupport = $support + @((Join-Path $moduleRoot 'acceptor.cpp'),(Join-Path $moduleRoot 'lifecycle.cpp'),(Join-Path $moduleRoot 'close.cpp'),(Join-Path $moduleRoot 'closing.cpp'),(Join-Path $moduleRoot 'cancellation.cpp'))
& $compiler @flags @cancellationSupport (Join-Path $moduleRoot 'cancellation_tests.cpp') '-lws2_32' '-o' $cancellationExe
if ($LASTEXITCODE -ne 0) { throw 'Socket cancellation test compilation failed' }
& $compiler @flags @cancellationSupport (Join-Path $moduleRoot 'native.cpp') (Join-Path $moduleRoot 'cancellation_native.cpp') (Join-Path $moduleRoot 'cancellation_native_tests.cpp') '-lws2_32' '-o' $cancellationNativeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket native cancellation test compilation failed' }
& $compiler @flags (Join-Path $moduleRoot 'recycle.cpp') (Join-Path $moduleRoot 'recycle_tests.cpp') '-lws2_32' '-o' $recycleExe
if ($LASTEXITCODE -ne 0) { throw 'Socket recycler test compilation failed' }
& $compiler @flags @cancellationSupport (Join-Path $moduleRoot 'recycle.cpp') (Join-Path $moduleRoot 'recycle_storage.cpp') (Join-Path $moduleRoot 'recycle_storage_tests.cpp') '-lws2_32' '-o' $recycleStorageExe
if ($LASTEXITCODE -ne 0) { throw 'Socket recycler storage test compilation failed' }
$registrySupport = $cancellationSupport + @((Join-Path $moduleRoot 'recycle.cpp'),(Join-Path $moduleRoot 'recycle_storage.cpp'),(Join-Path $moduleRoot 'service_registry.cpp'))
& $compiler @flags @registrySupport (Join-Path $moduleRoot 'service_registry_tests.cpp') '-lws2_32' '-o' $serviceRegistryExe
if ($LASTEXITCODE -ne 0) { throw 'Socket service registry test compilation failed' }
& $compiler @flags @registrySupport (Join-Path $moduleRoot 'service_registry_integration_tests.cpp') '-lws2_32' '-o' $serviceRegistryIntegrationExe
if ($LASTEXITCODE -ne 0) { throw 'Socket service registry integration test compilation failed' }
$serviceConstructorExe = Join-Path $buildRoot 'socket_service_constructor_tests.exe'
$serviceConstructorNativeExe = Join-Path $buildRoot 'socket_service_constructor_native_tests.exe'
$constructorSupport = $registrySupport + @((Join-Path $moduleRoot 'service_constructor.cpp'),(Join-Path $moduleRoot 'service_constructor_native.cpp'))
& $compiler @flags @constructorSupport (Join-Path $moduleRoot 'service_constructor_tests.cpp') '-lws2_32' '-o' $serviceConstructorExe
if ($LASTEXITCODE -ne 0) { throw 'Socket service constructor test compilation failed' }
& $compiler @flags @constructorSupport (Join-Path $moduleRoot 'service_constructor_native_tests.cpp') '-lws2_32' '-o' $serviceConstructorNativeExe
if ($LASTEXITCODE -ne 0) { throw 'Socket native service constructor test compilation failed' }
$serviceConstructorOutput = (& $serviceConstructorExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $serviceConstructorOutput }
$serviceConstructorNativeOutput = (& $serviceConstructorNativeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $serviceConstructorNativeOutput }
$serviceRegistryOutput = (& $serviceRegistryExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $serviceRegistryOutput }
$serviceRegistryIntegrationOutput = (& $serviceRegistryIntegrationExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $serviceRegistryIntegrationOutput }
$operationOutput = (& $operationExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $operationOutput }
$nativeOutput = (& $nativeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $nativeOutput }
$acceptorOutput = (& $acceptorExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $acceptorOutput }
$acceptorNativeOutput = (& $acceptorNativeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $acceptorNativeOutput }
$lifecycleOutput = (& $lifecycleExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $lifecycleOutput }
$lifecycleNativeOutput = (& $lifecycleNativeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $lifecycleNativeOutput }
$closeOutput = (& $closeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $closeOutput }
$closingOutput = (& $closingExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $closingOutput }
$cancellationOutput = (& $cancellationExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $cancellationOutput }
$cancellationNativeOutput = (& $cancellationNativeExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $cancellationNativeOutput }
$recycleOutput = (& $recycleExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $recycleOutput }
$recycleStorageOutput = (& $recycleStorageExe 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw $recycleStorageOutput }
$output = $operationOutput + "`n" + $nativeOutput
$output | Set-Content -LiteralPath (Join-Path $buildRoot 'test-output.txt') -Encoding utf8
$sourceHashes = [ordered]@{}
foreach ($name in @('operations.hpp','operations.cpp','native.hpp','native.cpp','operations_tests.cpp','native_tests.cpp','build.ps1','CMakeLists.txt')) {
    $sourceHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$operationChecks = [int]([regex]::Match($operationOutput,'PASS: (\d+)').Groups[1].Value)
$nativeChecks = [int]([regex]::Match($nativeOutput,'PASS: (\d+)').Groups[1].Value)
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; operation_checks=$operationChecks; native_checks=$nativeChecks; output=$output; source_hashes=$sourceHashes
    original_execution='none'; real_user_files_read=$false; native_scope='Injected SDK imports only; no WSAStartup, sockets, network or original DLL execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'validation.json') -Destination (Join-Path $moduleRoot 'validation.json')
$acceptorHashes = [ordered]@{}
foreach ($name in @('acceptor.hpp','acceptor.cpp','acceptor_native.hpp','acceptor_native.cpp','acceptor_tests.cpp','acceptor_native_tests.cpp','build.ps1','CMakeLists.txt')) {
    $acceptorHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$acceptorCombinedOutput = $acceptorOutput + "`n" + $acceptorNativeOutput
$acceptorCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'acceptor-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; acceptor_checks=[int]([regex]::Match($acceptorOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($acceptorNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$acceptorCombinedOutput; source_hashes=$acceptorHashes
    original_execution='none'; real_user_files_read=$false; native_scope='Injected SDK imports and controlled service/ownership/category/runtime providers only; no WSAStartup, sockets, network or original DLL execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'acceptor-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'acceptor-validation.json') -Destination (Join-Path $moduleRoot 'acceptor-validation.json')
$lifecycleHashes = [ordered]@{}
foreach ($name in @('lifecycle.hpp','lifecycle.cpp','lifecycle_native.hpp','lifecycle_native.cpp','lifecycle_tests.cpp','lifecycle_native_tests.cpp','build.ps1','CMakeLists.txt')) {
    $lifecycleHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$lifecycleCombinedOutput = $lifecycleOutput + "`n" + $lifecycleNativeOutput
$lifecycleCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'lifecycle-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; lifecycle_checks=[int]([regex]::Match($lifecycleOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($lifecycleNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$lifecycleCombinedOutput; source_hashes=$lifecycleHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='Typed logical service/list/owner fields with controlled resolver/close/cancel/executor effects; injected SDK gate imports and selected own-host noop control allocation'
    native_ASIO_IOCP_allocator_ABI_SEH='not established; no original DLL, WSAStartup, sockets, network or real critical-section APIs executed'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'lifecycle-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'lifecycle-validation.json') -Destination (Join-Path $moduleRoot 'lifecycle-validation.json')
$closeHashes = [ordered]@{}
foreach ($name in @('close.hpp','close.cpp','closing.hpp','closing.cpp','close_tests.cpp','closing_tests.cpp','build.ps1','CMakeLists.txt')) {
    $closeHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$closeCombinedOutput = $closeOutput + "`n" + $closingOutput
$closeCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'close-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; close_checks=[int]([regex]::Match($closeOutput,'PASS: (\d+)').Groups[1].Value)
    closing_checks=[int]([regex]::Match($closingOutput,'PASS: (\d+)').Groups[1].Value)
    output=$closeCombinedOutput; source_hashes=$closeHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='026e50/026fb0 normal paths, controlled cancellation engine/service/owner callbacks and typed injected socket APIs; actual acceptor/lifecycle/destructor source integration'
    native_ASIO_IOCP_ABI_SEH='not established;026aa0 remains required, no actual WSA/socket/network/critical-section or original target execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'close-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'close-validation.json') -Destination (Join-Path $moduleRoot 'close-validation.json')
$cancellationHashes = [ordered]@{}
foreach ($name in @('cancellation.hpp','cancellation.cpp','cancellation_native.hpp','cancellation_native.cpp','cancellation_tests.cpp','cancellation_native_tests.cpp','build.ps1','CMakeLists.txt')) {
    $cancellationHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$cancellationCombinedOutput = $cancellationOutput + "`n" + $cancellationNativeOutput
$cancellationCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'cancellation-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; cancellation_checks=[int]([regex]::Match($cancellationOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($cancellationNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$cancellationCombinedOutput; source_hashes=$cancellationHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='026aa0 four-map normal drain/post/fallback and028850 wake, typed injected SDK imports and actual acceptor/lifecycle/closing/destructor source integration'
    required_providers='0283c0 recycler, map initialization/lifetime/concurrency, completion consumer and context mapping remain explicit caller-owned boundaries'
    native_ASIO_IOCP_ABI_SEH='not established; no actual WSA/socket/network/IOCP/critical-section API or original target execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'cancellation-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'cancellation-validation.json') -Destination (Join-Path $moduleRoot 'cancellation-validation.json')
Write-Output $output
Write-Output $acceptorCombinedOutput
Write-Output $lifecycleCombinedOutput

Write-Output $closeCombinedOutput
Write-Output $cancellationCombinedOutput
$recycleHashes = [ordered]@{}
foreach ($name in @('recycle.hpp','recycle.cpp','recycle_storage.hpp','recycle_storage.cpp','recycle_tests.cpp','recycle_storage_tests.cpp','build.ps1','CMakeLists.txt')) {
    $recycleHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$recycleCombinedOutput = $recycleOutput + "`n" + $recycleStorageOutput
$recycleCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'recycle-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; recycle_checks=[int]([regex]::Match($recycleOutput,'PASS: (\d+)').Groups[1].Value)
    storage_checks=[int]([regex]::Match($recycleStorageOutput,'PASS: (\d+)').Groups[1].Value)
    output=$recycleCombinedOutput; source_hashes=$recycleHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='0283c0 normal bucket/list transfer, equality-limit partial mutation and unsigned counts; explicitly selected host owned storage/reuse, actual acceptor/lifecycle/closing/cancellation source chain with controlled imports'
    original_procedures_added=1
    native_constructor_insert_rehash_allocator_ABI_exception='not established; owned storage is a selected host backend, required047fc4 remains a nonreturning effect; no original, socket, network or IOCP execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'recycle-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'recycle-validation.json') -Destination (Join-Path $moduleRoot 'recycle-validation.json')
Write-Output $recycleCombinedOutput

$serviceRegistryHashes = [ordered]@{}
foreach ($name in @('service_registry.hpp','service_registry.cpp','service_registry_tests.cpp','service_registry_integration_tests.cpp','build.ps1','CMakeLists.txt')) {
    $serviceRegistryHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$serviceRegistryCombinedOutput = $serviceRegistryOutput + "`n" + $serviceRegistryIntegrationOutput
$serviceRegistryCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'service-registry-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; core_checks=[int]([regex]::Match($serviceRegistryOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($serviceRegistryIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$serviceRegistryCombinedOutput; source_hashes=$serviceRegistryHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='0239c0/022fa0 retained normal-flow key/match/two-pass publication/race deletion and live callback mutations; borrowed RegistryLifecycleEffects with actual acceptor/lifecycle/closing/cancellation/0283c0 source chain'
    original_procedures_added=2
    native_constructor_type_compare_ASIO_ABI_unwind='not established; 0228e0 and04d2d0 remain required controlled effects; host projection is not native layout or ownership, no original/network/socket/IOCP execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'service-registry-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'service-registry-validation.json') -Destination (Join-Path $moduleRoot 'service-registry-validation.json')
Write-Output $serviceRegistryCombinedOutput
$serviceConstructorHashes = [ordered]@{}
foreach ($name in @('service_constructor.hpp','service_constructor.cpp','service_constructor_native.hpp','service_constructor_native.cpp','service_constructor_tests.cpp','service_constructor_native_tests.cpp','build.ps1','CMakeLists.txt')) {
    $serviceConstructorHashes[$name] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$serviceConstructorCombinedOutput = $serviceConstructorOutput + "`n" + $serviceConstructorNativeOutput
$serviceConstructorCombinedOutput | Set-Content -LiteralPath (Join-Path $buildRoot 'service-constructor-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS'; compiler=$compiler; compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant()
    flags=$flags; core_checks=[int]([regex]::Match($serviceConstructorOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($serviceConstructorNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$serviceConstructorCombinedOutput; source_hashes=$serviceConstructorHashes
    original_execution='none'; real_user_files_read=$false
    execution_scope='0228e0/027be0 normal allocation/header/scheduler/gate/category/failure/finalization order; SDK typed injected InitializeCriticalSectionAndSpinCount/GetLastError; real registry/acceptor/lifecycle/closing/cancellation/0283c0 source chain'
    original_procedures_added=2
    native_scheduler_allocator_category_exception_unwind_destructor_ABI='not established; 023b30,04b0bc,028620,028240 and scalar deletion remain required providers; no original/socket/network/IOCP/real critical-section execution'
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $buildRoot 'service-constructor-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'service-constructor-validation.json') -Destination (Join-Path $moduleRoot 'service-constructor-validation.json')
Write-Output $serviceConstructorCombinedOutput

$schedulerRegistryExe=Join-Path $buildRoot 'socket_scheduler_registry_tests.exe'
$schedulerRegistryIntegrationExe=Join-Path $buildRoot 'socket_scheduler_registry_integration_tests.exe'
$schedulerSupport=$constructorSupport+@((Join-Path $moduleRoot 'scheduler_registry.cpp'))
& $compiler @flags @schedulerSupport (Join-Path $moduleRoot 'scheduler_registry_tests.cpp') '-lws2_32' '-o' $schedulerRegistryExe
if($LASTEXITCODE -ne 0){throw 'Scheduler registry core test compilation failed'}
& $compiler @flags @schedulerSupport (Join-Path $moduleRoot 'scheduler_registry_integration_tests.cpp') '-lws2_32' '-o' $schedulerRegistryIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Scheduler registry integration compilation failed'}
$schedulerRegistryOutput=(& $schedulerRegistryExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $schedulerRegistryOutput}
$schedulerRegistryIntegrationOutput=(& $schedulerRegistryIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $schedulerRegistryIntegrationOutput}
$schedulerHashes=[ordered]@{}
foreach($name in @('scheduler_registry.hpp','scheduler_registry.cpp','scheduler_registry_tests.cpp','scheduler_registry_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $schedulerHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$schedulerCombinedOutput=$schedulerRegistryOutput+"`n"+$schedulerRegistryIntegrationOutput
$schedulerCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'scheduler-registry-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($schedulerRegistryOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($schedulerRegistryIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$schedulerCombinedOutput;source_hashes=$schedulerHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=2
    execution_scope='022fd0/023b30 retained normal flow, cached owner before enter, constructor return identity, live two-pass scans and publication/deletion; shared socket/scheduler registry through actual0228e0/acceptor/lifecycle/closing/queue/recycler with controlled effects'
    required_providers='04b0bc native allocator,024990 scheduler constructor,04d2d0 RTTI comparator,scalar deletion remain controlled providers; normal flow only'
    native_ASIO_IOCP_allocator_ABI_SEH='not established; no original DLL/game or actual socket/network/IOCP/critical-section execution; no real cfg/token/achievement reads'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'scheduler-registry-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'scheduler-registry-validation.json') -Destination (Join-Path $moduleRoot 'scheduler-registry-validation.json')
Write-Output $schedulerCombinedOutput

$iocpConstructorExe=Join-Path $buildRoot 'socket_iocp_constructor_tests.exe'
$iocpConstructorNativeExe=Join-Path $buildRoot 'socket_iocp_constructor_native_tests.exe'
$iocpSupport=$schedulerSupport+@((Join-Path $moduleRoot 'iocp_constructor.cpp'),(Join-Path $moduleRoot 'iocp_constructor_native.cpp'))
& $compiler @flags @iocpSupport (Join-Path $moduleRoot 'iocp_constructor_tests.cpp') '-lws2_32' '-o' $iocpConstructorExe
if($LASTEXITCODE -ne 0){throw 'IOCP constructor core test compilation failed'}
& $compiler @flags @iocpSupport (Join-Path $moduleRoot 'iocp_constructor_native_tests.cpp') '-lws2_32' '-o' $iocpConstructorNativeExe
if($LASTEXITCODE -ne 0){throw 'IOCP constructor native integration compilation failed'}
$iocpConstructorOutput=(& $iocpConstructorExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $iocpConstructorOutput}
$iocpConstructorNativeOutput=(& $iocpConstructorNativeExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $iocpConstructorNativeOutput}
$iocpHashes=[ordered]@{}
foreach($name in @('iocp_constructor.hpp','iocp_constructor.cpp','iocp_constructor_native.hpp','iocp_constructor_native.cpp','iocp_constructor_tests.cpp','iocp_constructor_native_tests.cpp','build.ps1','CMakeLists.txt')){
    $iocpHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$iocpCombinedOutput=$iocpConstructorOutput+"`n"+$iocpConstructorNativeOutput
$iocpCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'iocp-constructor-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($iocpConstructorOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($iocpConstructorNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$iocpCombinedOutput;source_hashes=$iocpHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=1
    execution_scope='024990 retained normal construction, version/gate/port/error/worker callback ordering, exact partial writes, unsigned low32 work wrap; five SDK-typed injected imports and actual shared registry/0228e0/acceptor/lifecycle/closing/026aa0/0283c0 chain using IocpSchedulerEntry.completion itself'
    required_providers='04b0bc/04b0f8 allocator/free,02a000 worker-event-thread start,028240 exception,04d2d0 CRT comparison and scalar deletion remain required controlled effects; socket-service completion_port remains an explicit caller-selected projection'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; null function tests observe only required-call boundary and do not claim native successful null execution; no original/game/real socket/network/IOCP/worker/thread/critical-section or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'iocp-constructor-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'iocp-constructor-validation.json') -Destination (Join-Path $moduleRoot 'iocp-constructor-validation.json')
Write-Output $iocpCombinedOutput

$workerStartupExe=Join-Path $buildRoot 'socket_worker_startup_tests.exe'
$workerStartupNativeExe=Join-Path $buildRoot 'socket_worker_startup_native_tests.exe'
$workerSupport=$iocpSupport+@((Join-Path $moduleRoot 'worker_startup.cpp'),(Join-Path $moduleRoot 'worker_startup_native.cpp'),(Join-Path $moduleRoot '../anadius-thread-source/threads.cpp'))
& $compiler @flags @workerSupport (Join-Path $moduleRoot 'worker_startup_tests.cpp') '-lws2_32' '-o' $workerStartupExe
if($LASTEXITCODE -ne 0){throw 'Worker startup core test compilation failed'}
& $compiler @flags @workerSupport (Join-Path $moduleRoot 'worker_startup_native_tests.cpp') '-lws2_32' '-o' $workerStartupNativeExe
if($LASTEXITCODE -ne 0){throw 'Worker startup native integration compilation failed'}
$workerStartupOutput=(& $workerStartupExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $workerStartupOutput}
$workerStartupNativeOutput=(& $workerStartupNativeExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $workerStartupNativeOutput}
$workerHashes=[ordered]@{}
foreach($name in @('worker_startup.hpp','worker_startup.cpp','worker_startup_native.hpp','worker_startup_native.cpp','worker_startup_tests.cpp','worker_startup_native_tests.cpp','build.ps1','CMakeLists.txt')){
    $workerHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$workerCombinedOutput=$workerStartupOutput+"`n"+$workerStartupNativeOutput
$workerCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'worker-startup-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($workerStartupOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($workerStartupNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$workerCombinedOutput;source_hashes=$workerHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=2
    execution_scope='02a000/02a4a0 retained normal event/thread handshake, cached status/entry, live exit reload, zeroerror repeated deletion/close with live projections; six SDK injected wrappers; unchanged05e8e0 source bridge and actual024990/shared registry/acceptor/closing/cancellation/0283c0 controlled integration'
    required_providers='04b0bc/04b0f8 allocation and free, function slot08 scheduler body and slot00 deletion,028240 exception,04d2d0 CRT comparison remain required providers; original entry VA is opaque data'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; no original/game/real socket/network/IOCP/events/threads/waits/sleep/critical-section or user cfg/token/achievement execution/read; source fixtures do not establish native concurrency or binary equivalence'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'worker-startup-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'worker-startup-validation.json') -Destination (Join-Path $moduleRoot 'worker-startup-validation.json')
Write-Output $workerCombinedOutput

$workerRunExe=Join-Path $buildRoot 'socket_worker_run_tests.exe'
$workerRunNativeExe=Join-Path $buildRoot 'socket_worker_run_native_tests.exe'
$workerRunSupport=$workerSupport+@((Join-Path $moduleRoot 'worker_run.cpp'),(Join-Path $moduleRoot 'worker_run_native.cpp'))
& $compiler @flags @workerRunSupport (Join-Path $moduleRoot 'worker_run_tests.cpp') '-lws2_32' '-o' $workerRunExe
if($LASTEXITCODE -ne 0){throw 'Worker run core test compilation failed'}
& $compiler @flags @workerRunSupport (Join-Path $moduleRoot 'worker_run_native_tests.cpp') '-lws2_32' '-o' $workerRunNativeExe
if($LASTEXITCODE -ne 0){throw 'Worker run native integration compilation failed'}
$workerRunOutput=(& $workerRunExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $workerRunOutput}
$workerRunNativeOutput=(& $workerRunNativeExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $workerRunNativeOutput}
$workerRunHashes=[ordered]@{}
foreach($name in @('worker_run.hpp','worker_run.cpp','worker_run_native.hpp','worker_run_native.cpp','worker_run_tests.cpp','worker_run_native_tests.cpp','build.ps1','CMakeLists.txt')){
    $workerRunHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$workerRunCombinedOutput=$workerRunOutput+"`n"+$workerRunNativeOutput
$workerRunCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'worker-run-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($workerRunOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($workerRunNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$workerRunCombinedOutput;source_hashes=$workerRunHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=4
    execution_scope='026320/029890/0298b0/02a190 retained normal bodies, logical optional status, low32 field effects, live TLS indices/previous/cache reads, dispatch count saturation and cache cleanup; four SDK injected wrappers; actual024990/02a000/unchanged05e8e0/02a4a0/029890/0298b0/02a190/026320 controlled source composition with actual safe heap deletion'
    required_providers='027c10 dispatcher,04b0f8 native allocation lifetime,04b788 cache free,04a240/04a2cc CRT pair and028240 exception remain required providers; CRT production unchanged and paused; image addresses are opaque data'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; serialized typed low32 fixture writes do not prove native atomics/concurrency; no original/game/real TLS/port/post/event/thread/wait/sleep/network or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'worker-run-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'worker-run-validation.json') -Destination (Join-Path $moduleRoot 'worker-run-validation.json')
Write-Output $workerRunCombinedOutput

$dispatcherExe=Join-Path $buildRoot 'socket_completion_dispatcher_tests.exe'
$dispatcherNativeExe=Join-Path $buildRoot 'socket_completion_dispatcher_native_tests.exe'
$dispatcherSupport=$workerRunSupport+@((Join-Path $moduleRoot 'completion_dispatcher.cpp'),(Join-Path $moduleRoot 'completion_dispatcher_native.cpp'))
& $compiler @flags @dispatcherSupport (Join-Path $moduleRoot 'completion_dispatcher_tests.cpp') '-lws2_32' '-o' $dispatcherExe
if($LASTEXITCODE -ne 0){throw 'Completion dispatcher core test compilation failed'}
& $compiler @flags @dispatcherSupport (Join-Path $moduleRoot 'completion_dispatcher_native_tests.cpp') '-lws2_32' '-o' $dispatcherNativeExe
if($LASTEXITCODE -ne 0){throw 'Completion dispatcher native integration compilation failed'}
$dispatcherOutput=(& $dispatcherExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $dispatcherOutput}
$dispatcherNativeOutput=(& $dispatcherNativeExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $dispatcherNativeOutput}
$dispatcherHashes=[ordered]@{}
foreach($name in @('completion_dispatcher.hpp','completion_dispatcher.cpp','completion_dispatcher_native.hpp','completion_dispatcher_native.cpp','completion_dispatcher_tests.cpp','completion_dispatcher_native_tests.cpp','build.ps1','CMakeLists.txt')){
    $dispatcherHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$dispatcherCombinedOutput=$dispatcherOutput+"`n"+$dispatcherNativeOutput
$dispatcherCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'completion-dispatcher-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($dispatcherOutput,'PASS: (\d+)').Groups[1].Value)
    native_checks=[int]([regex]::Match($dispatcherNativeOutput,'PASS: (\d+)').Groups[1].Value)
    output=$dispatcherCombinedOutput;source_hashes=$dispatcherHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=1
    execution_scope='027c10 complete retained normal body: recursive dispatch publication/retry, live service/timer queues, NEG32 due arithmetic, mandatory wait/error ordering, TLS signed guard, packet metadata/queued CAS, exception terminal, work decrement/stop relay; five injected SDK imports; actual026aa0 fallback plus full024990/02a000/unchanged05e8e0/02a4a0/029890/0298b0/027c10/02a190/026320 source composition and heap lifetime end'
    required_providers='Native GS TLS slot/epoch storage,04b838/04b770/04b7cc guard CRT,024100 exception copy/029880 terminal rethrow,04b0f8 allocation lifetime,04b788 cache free,04a240/04a2cc CRT and028240 exception remain required effects; CRT production unchanged'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; typed serialized low32 effects do not prove native atomic concurrency; borrowed OVERLAPPED identity cached only while live; no original/game/real SDK TLS/IOCP/timer/event/thread/wait/sleep/network or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'completion-dispatcher-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'completion-dispatcher-validation.json') -Destination (Join-Path $moduleRoot 'completion-dispatcher-validation.json')
Write-Output $dispatcherCombinedOutput

$postedExe=Join-Path $buildRoot 'socket_posted_operation_tests.exe'
$postedIntegrationExe=Join-Path $buildRoot 'socket_posted_operation_integration_tests.exe'
$postedSupport=$dispatcherSupport+@((Join-Path $moduleRoot 'posted_operation.cpp'),(Join-Path $moduleRoot 'posted_operation_dispatch.cpp'))
& $compiler @flags @postedSupport (Join-Path $moduleRoot 'posted_operation_tests.cpp') '-lws2_32' '-o' $postedExe
if($LASTEXITCODE -ne 0){throw 'Posted operation core test compilation failed'}
& $compiler @flags @postedSupport (Join-Path $moduleRoot 'posted_operation_integration_tests.cpp') '-lws2_32' '-o' $postedIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Posted operation integration compilation failed'}
$postedOutput=(& $postedExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $postedOutput}
$postedIntegrationOutput=(& $postedIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $postedIntegrationOutput}
$postedHashes=[ordered]@{}
foreach($name in @('posted_operation.hpp','posted_operation.cpp','posted_operation_dispatch.hpp','posted_operation_dispatch.cpp','posted_operation_tests.cpp','posted_operation_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $postedHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$postedCombinedOutput=$postedOutput+"`n"+$postedIntegrationOutput
$postedCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'posted-operation-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($postedOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($postedIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$postedCombinedOutput;source_hashes=$postedHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=6
    execution_scope='021820/021ac0/022e40/027120/028ca0/028fd0 retained normal bodies: bounded two-slot allocator/cache marker, source transfer, inline TLS first-match and fresh queued TLS, live scheduler, work32/post/fallback gate, callback free-before-invoke and live cleanup mutations; actual022e40/028ca0/0298b0/027c10/027120/028fd0/021ac0 integration with existing injected NativeWorkerRunApi/NativeDispatcherApi and cache reuse for reentrant posted job; actual operation/storage/functor heap lifetime ends'
    required_providers='04b0bc native allocator,04b788 free,functor slot00 callable/deletion,GS TLS/guard CRT/exception storage,SDK imports and ABI ownership remain mandatory effects; CRT production unchanged'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; bounded raw byte spans and SDK OVERLAPPED are separate projections, not native0x48 overlay; fabricated storage identities and deterministic allocator/free fixtures do not establish native heap/SEH/concurrency or binary equivalence; no original/game/real SDK TLS/IOCP/event/thread/wait/network or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'posted-operation-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'posted-operation-validation.json') -Destination (Join-Path $moduleRoot 'posted-operation-validation.json')
Write-Output $postedCombinedOutput

$ioExe=Join-Path $buildRoot 'socket_io_completion_tests.exe'
$ioIntegrationExe=Join-Path $buildRoot 'socket_io_completion_integration_tests.exe'
$ioSupport=$postedSupport+@((Join-Path $moduleRoot 'io_completion.cpp'),(Join-Path $moduleRoot 'io_completion_dispatch.cpp'))
& $compiler @flags @ioSupport (Join-Path $moduleRoot 'io_completion_tests.cpp') '-lws2_32' '-o' $ioExe
if($LASTEXITCODE -ne 0){throw 'IO completion core test compilation failed'}
& $compiler @flags @ioSupport (Join-Path $moduleRoot 'io_completion_integration_tests.cpp') '-lws2_32' '-o' $ioIntegrationExe
if($LASTEXITCODE -ne 0){throw 'IO completion integration compilation failed'}
$ioOutput=(& $ioExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $ioOutput}
$ioIntegrationOutput=(& $ioIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $ioIntegrationOutput}
$ioHashes=[ordered]@{}
foreach($name in @('io_completion.hpp','io_completion.cpp','io_completion_dispatch.hpp','io_completion_dispatch.cpp','io_completion_tests.cpp','io_completion_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $ioHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$ioCombinedOutput=$ioOutput+"`n"+$ioIntegrationOutput
$ioCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'io-completion-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($ioOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($ioIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$ioCombinedOutput;source_hashes=$ioHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=8
    execution_scope='027710/027a20/0293f0/0294c0/023cb0/021a40/021a80/026a00 retained normal bodies: receive/send defined-field error mapping and EOF branch exclusion, live environment moves, cached factory function with actual executor deletion during query, temporary live guard and unconditional nested destroy, early cleanup, unsigned32 strong/weak decrement and live pointer reloads, TLS-first bounded two-slot marker cache; actual028ca0/0298b0/027c10/IO routing plus reentrant022e40/027120 mixed callback integration, reused IO storage for posted job, existing injected NativeWorkerRunApi/NativeDispatcherApi, post-failure fallback and actual operation/storage/control heap lifetime ends'
    required_providers='04b788 free/04b0bc allocation, native polymorphic move/query/factory/destroy implementations,025c40/0261d0/0229d0/022c60 bodies, reference dispose/delete,GS TLS/guard CRT/exception storage and native ABI ownership remain required effects; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; Error represents defined code/category only, send remapped padding and dummy prefix unspecified, native factory scratch contents opaque; bounded raw storage and separate typed SDK OVERLAPPED do not prove native layout/heap/SEH/concurrency or binary equivalence; no original/game/real SDK TLS/IOCP/socket/network/event/thread/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'io-completion-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'io-completion-validation.json') -Destination (Join-Path $moduleRoot 'io-completion-validation.json')
Write-Output $ioCombinedOutput

$handlerExe=Join-Path $buildRoot 'socket_handler_dispatch_tests.exe'
$handlerIntegrationExe=Join-Path $buildRoot 'socket_handler_dispatch_integration_tests.exe'
$handlerSupport=$ioSupport+@((Join-Path $moduleRoot 'handler_dispatch.cpp'),(Join-Path $moduleRoot 'handler_dispatch_bridge.cpp'))
& $compiler @flags @handlerSupport (Join-Path $moduleRoot 'handler_dispatch_tests.cpp') '-lws2_32' '-o' $handlerExe
if($LASTEXITCODE -ne 0){throw 'Handler dispatch core test compilation failed'}
& $compiler @flags @handlerSupport (Join-Path $moduleRoot 'handler_dispatch_integration_tests.cpp') '-lws2_32' '-o' $handlerIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Handler dispatch integration compilation failed'}
$handlerOutput=(& $handlerExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $handlerOutput}
$handlerIntegrationOutput=(& $handlerIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $handlerIntegrationOutput}
$handlerHashes=[ordered]@{}
foreach($name in @('handler_dispatch.hpp','handler_dispatch.cpp','handler_dispatch_bridge.hpp','handler_dispatch_bridge.cpp','handler_dispatch_tests.cpp','handler_dispatch_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $handlerHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$handlerCombinedOutput=$handlerOutput+"`n"+$handlerIntegrationOutput
$handlerCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'handler-dispatch-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($handlerOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($handlerIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$handlerCombinedOutput;source_hashes=$handlerHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=9
    execution_scope='021900/022640/022670/022780/0227b0/0229d0/022c60/0291c0/0292a0 retained normal bodies: handler-only pool4/5 allocation and marker cleanup, borrowed synchronous slot18 priority, owned mutable publication slot10 cached before TLS, actual nested send factory/move/live destroy, transfer-before-error-copy, early free-before-invoke and live final control release; actual028ca0/0298b0/027c10/IO routing/recovered handler scheduler/owned task callbacks plus reentrant022e40/027120 and separate pool0/1 and4/5 reuse, post-failure fallback; injected existing native SDK adapters and actual object/storage/control heap lifetime ends'
    required_providers='04b0bc allocation/04b788 free, native polymorphic query/factory/move/destroy/scheduler slot18 and10,025c40/0261d0 bodies, reference dispose/delete,GS TLS/guard CRT/exception storage and ABI ownership remain required effects; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; bounded byte storage and typed handlers are separate projections, not native0x70/78 ABI overlays; uninitialized allocator tag/scratch omitted, normal-flow ownership and deterministic synthetic lifetimes do not prove native concurrent heap/SEH or binary equivalence; no original/game/real SDK TLS/IOCP/socket/network/event/thread/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'handler-dispatch-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'handler-dispatch-validation.json') -Destination (Join-Path $moduleRoot 'handler-dispatch-validation.json')
Write-Output $handlerCombinedOutput

$composedExe=Join-Path $buildRoot 'socket_composed_write_tests.exe'
$composedIntegrationExe=Join-Path $buildRoot 'socket_composed_write_integration_tests.exe'
$composedSupport=$handlerSupport+@((Join-Path $moduleRoot 'composed_write.cpp'),(Join-Path $moduleRoot 'composed_write_native.cpp'))
& $compiler @flags @composedSupport (Join-Path $moduleRoot 'composed_write_tests.cpp') '-lws2_32' '-o' $composedExe
if($LASTEXITCODE -ne 0){throw 'Composed write core test compilation failed'}
& $compiler @flags @composedSupport (Join-Path $moduleRoot 'composed_write_integration_tests.cpp') '-lws2_32' '-o' $composedIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Composed write integration test compilation failed'}
$composedOutput=(& $composedExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $composedOutput}
$composedIntegrationOutput=(& $composedIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $composedIntegrationOutput}
$composedHashes=[ordered]@{}
foreach($name in @('composed_write.hpp','composed_write.cpp','composed_write_native.hpp','composed_write_native.cpp','composed_write_tests.cpp','composed_write_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $composedHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$composedCombinedOutput=$composedOutput+"`n"+$composedIntegrationOutput
$composedCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'composed-write-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($composedOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($composedIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$composedCombinedOutput;source_hashes=$composedHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=5
    execution_scope='021fd0/0261d0/0269b0/028b50/028c00 retained normal bodies: independent flag/progress/error/64KiB slice matrix and unsigned wrap, current TLS pool0/1 allocation, live buffer reload after constructor and weak release, weak low32 ownership, work32 increment, exact injected WSASend then unconditional immediate WSAGetLastError, status remap, queued CAS and same-gate fallback; actual021fd0/0298b0/027c10/IO routing/027a20/recovered handler scheduler/0261d0 three-chunk continuation, borrowed and owned paths, first2 and4/5 cache reuse, native early completion key2 preservation and key0 fallback metadata replacement, actual heap lifetime ends'
    required_providers='023dd0 environment constructor,025a70 application/deque/log finish,04b0bc allocator/04b788 free, native weak deletion and handler factory/scheduler/move/destroy,GS TLS/guard CRT/exception storage and ABI ownership remain required; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; bounded raw bytes and typed SDK OVERLAPPED/packet/handler projections are separate, not native0xd0 overlay; unspecified allocator tag omitted; deterministic injected SDK calls and synthetic lifetime/race handshake do not prove native concurrency/heap/SEH, binary or full DLL equivalence; no original/game/real SDK socket/network/TLS/IOCP/event/thread/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'composed-write-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'composed-write-validation.json') -Destination (Join-Path $moduleRoot 'composed-write-validation.json')
Write-Output $composedCombinedOutput
$environmentConstructorExe=Join-Path $buildRoot 'socket_environment_constructor_tests.exe'
$environmentConstructorIntegrationExe=Join-Path $buildRoot 'socket_environment_constructor_integration_tests.exe'
$environmentConstructorSupport=$composedSupport+@((Join-Path $moduleRoot 'environment_constructor.cpp'))
& $compiler @flags @environmentConstructorSupport (Join-Path $moduleRoot 'environment_constructor_tests.cpp') '-lws2_32' '-o' $environmentConstructorExe
if($LASTEXITCODE -ne 0){throw 'Environment constructor core test compilation failed'}
& $compiler @flags @environmentConstructorSupport (Join-Path $moduleRoot 'environment_constructor_integration_tests.cpp') '-lws2_32' '-o' $environmentConstructorIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Environment constructor integration test compilation failed'}
$environmentConstructorOutput=(& $environmentConstructorExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $environmentConstructorOutput}
$environmentConstructorIntegrationOutput=(& $environmentConstructorIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $environmentConstructorIntegrationOutput}
$environmentConstructorHashes=[ordered]@{}
foreach($name in @('environment_constructor.hpp','environment_constructor.cpp','environment_constructor_tests.cpp','environment_constructor_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $environmentConstructorHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$environmentConstructorCombinedOutput=$environmentConstructorOutput+"`n"+$environmentConstructorIntegrationOutput
$environmentConstructorCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'environment-constructor-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($environmentConstructorOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($environmentConstructorIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$environmentConstructorCombinedOutput;source_hashes=$environmentConstructorHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=1
    execution_scope='023dd0 retained normal body with existing023cb0: exact type-query/opaque type+8 comparator sequence, zero and signed nonzero branches, address wrap, default sentinels, cached slot58 factory before source query deleting native executor projection, two defined-field moves with live operations/state/schedule/executor order, conditional first and unconditional final live destroy, exact callback exception prefixes, bounded missing/ambiguous registries; actualRecoveredConstructedWriteEffects/023dd0/021fd0/0261d0/027c10/0298b0/027a20/recovered handler scheduler three-chunk composition, early key2 completion and key0 fallback metadata replacement, borrowed/owned paths, allocator reuse and synthetic lifetime ends'
    required_providers='04d2d0 CRT comparator remains an injected required provider, native type-query/factory58/move/destroy/prepare40/scheduler/weak deletion,025a70 application finish/025c40 receive,04b0bc allocation/04b788 free,GS TLS/guard CRT/exception storage and ABI ownership remain required; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; typed identities, temporary payload placeholders and defined-field callbacks do not reconstruct native vtable/ABI/heap/SEH or concurrency; exception tests assert normal sequence prefixes only and do not invent native unwind cleanup; no original/game/real SDK socket/network/TLS/IOCP/event/thread/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'environment-constructor-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'environment-constructor-validation.json') -Destination (Join-Path $moduleRoot 'environment-constructor-validation.json')
Write-Output $environmentConstructorCombinedOutput
$writeFinishExe=Join-Path $buildRoot 'socket_write_finish_tests.exe'
$writeFinishIntegrationExe=Join-Path $buildRoot 'socket_write_finish_integration_tests.exe'
$writeFinishSupport=$environmentConstructorSupport+@((Join-Path $moduleRoot 'write_finish.cpp'))
& $compiler @flags @writeFinishSupport (Join-Path $moduleRoot 'write_finish_tests.cpp') '-lws2_32' '-o' $writeFinishExe
if($LASTEXITCODE -ne 0){throw 'Write finish core test compilation failed'}
& $compiler @flags @writeFinishSupport (Join-Path $moduleRoot 'write_finish_integration_tests.cpp') '-lws2_32' '-o' $writeFinishIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Write finish integration test compilation failed'}
$writeFinishOutput=(& $writeFinishExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $writeFinishOutput}
$writeFinishIntegrationOutput=(& $writeFinishIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $writeFinishIntegrationOutput}
$writeFinishHashes=[ordered]@{}
foreach($name in @('write_finish.hpp','write_finish.cpp','write_finish_tests.cpp','write_finish_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $writeFinishHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$writeFinishCombinedOutput=$writeFinishOutput+"`n"+$writeFinishIntegrationOutput
$writeFinishCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'write-finish-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($writeFinishOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($writeFinishIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$writeFinishCombinedOutput;source_hashes=$writeFinishHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=1
    execution_scope='025a70 retained normal body: masked deque index and saved owner/string, live fields after free and current owner token before next; inline/heap capacity15/16/4094/4095/max and unsigned size/alignment boundaries including terminal; cached code before misc, category after misc cached across log begin, exact code2/category identity suppression; constructed string pointer/length, returned append stream, signed32 virtual offset wrap, widen/put/flush and live message destruction; callback mutation/lifetime ends, required exception prefixes and bounded missing/ambiguous projections; actualRecoveredFinishedWriteEffects/025a70/023dd0/021fd0/0261d0/027c10/0298b0/027a20 recovered scheduler chain, borrowed/owned three-chunk sends, early completion/fallback, queue reset/pop/required next and mapped error logging/suppression'
    required_providers='02a500 next write, native deque/string/category/vtable object projections,006880/007990/01e510/00fe90/00e1e0 logging,04b0f8 free/05c974 terminal,025c40 receive and native factory/TLS/guard/ABI ownership remain required; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; bounded typed map/string/category/stream tokens and deterministic injected SDK callbacks do not establish native ABI/table ownership/heap/SEH/concurrency or full DLL equivalence; exception prefixes do not invent native unwind cleanup; no original/game/real SDK socket/network/TLS/IOCP/event/thread/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'write-finish-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'write-finish-validation.json') -Destination (Join-Path $moduleRoot 'write-finish-validation.json')
Write-Output $writeFinishCombinedOutput

$queuedWriteExe=Join-Path $buildRoot 'socket_queued_write_tests.exe'
$queuedWriteIntegrationExe=Join-Path $buildRoot 'socket_queued_write_integration_tests.exe'
$queuedWriteSupport=$writeFinishSupport+@((Join-Path $moduleRoot 'queued_write.cpp'))
& $compiler @flags @queuedWriteSupport (Join-Path $moduleRoot 'queued_write_tests.cpp') '-lws2_32' '-o' $queuedWriteExe
if($LASTEXITCODE -ne 0){throw 'Queued write core test compilation failed'}
& $compiler @flags @queuedWriteSupport (Join-Path $moduleRoot 'queued_write_integration_tests.cpp') '-lws2_32' '-o' $queuedWriteIntegrationExe
if($LASTEXITCODE -ne 0){throw 'Queued write integration test compilation failed'}
$queuedWriteOutput=(& $queuedWriteExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $queuedWriteOutput}
$queuedWriteIntegrationOutput=(& $queuedWriteIntegrationExe 2>&1|Out-String).Trim()
if($LASTEXITCODE -ne 0){throw $queuedWriteIntegrationOutput}
$queuedWriteHashes=[ordered]@{}
foreach($name in @('queued_write.hpp','queued_write.cpp','queued_write_tests.cpp','queued_write_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $queuedWriteHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$queuedWriteCombinedOutput=$queuedWriteOutput+"`n"+$queuedWriteIntegrationOutput
$queuedWriteCombinedOutput|Set-Content -LiteralPath (Join-Path $buildRoot 'queued-write-test-output.txt') -Encoding utf8
[ordered]@{
    result='PASS';compiler=$compiler;compiler_sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $compiler).Hash.ToLowerInvariant();flags=$flags
    core_checks=[int]([regex]::Match($queuedWriteOutput,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($queuedWriteIntegrationOutput,'PASS: (\d+)').Groups[1].Value)
    output=$queuedWriteCombinedOutput;source_hashes=$queuedWriteHashes;original_execution='none';real_user_files_read=$false;original_procedures_added=1
    execution_scope='02a500 retained normal body: empty queue before all effects, null/expired strong owner terminal, real low32 CAS nonzero acquire and max-to-zero wrap plus bounded synthetic host contention; independent mask/inline/heap/zero/full64 oracle, literal owner+10 and+78 identities, defined initial handler fields, direct actual021fd0 by default; explicit callee boundary only for isolated caller residual cleanup, same cached strong/weak control across mutation, actual synthetic owner/queue/stream/control lifetime ends, bounded registries and normal exception prefixes; actualRecoveredQueuedWriteApi/02a500/021fd0/0261d0/025a70/02a500 cycle for two distinct queued messages, initial owner retention through borrowed/owned scheduler and chunk continuations, 64KiB slices and zero second item, cache reuse, early key2 preservation and key0 fallback replacement, mapped errors and suppression'
    required_providers='026860 terminal, native owner/deque/string/stream projections, category/logger/free providers, native factory/query/move/destroy/scheduler and allocator/free/ref disposal,025c40 receive,GS TLS/guard CRT/exception storage and ABI ownership remain required; CRT production unchanged and paused'
    native_ASIO_IOCP_worker_allocator_ABI_SEH='not established; typed sidecars and deterministic injected SDK callbacks are not native ABI/table/heap/SEH or concurrency equivalence; bounded host thread test proves reconstructed CAS source only; explicit core callee injection does not claim original runtime behavior, integration uses direct actual021fd0; exception prefixes do not invent unwind cleanup; no original/game/real SDK socket/network/TLS/IOCP/event/wait or user cfg/token/achievement execution/read'
}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $buildRoot 'queued-write-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'queued-write-validation.json') -Destination (Join-Path $moduleRoot 'queued-write-validation.json')
Write-Output $queuedWriteCombinedOutput

$asyncReceiveSupport=$queuedWriteSupport+@((Join-Path $moduleRoot 'async_receive.cpp'))
$asyncReceiveOutput=@{}
foreach($case in @('async_receive_tests','async_receive_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @asyncReceiveSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Async receive compilation failed '+$case)}
    $asyncReceiveOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $asyncReceiveOutput[$case]}
}
$asyncReceiveHashes=[ordered]@{}
foreach($name in @('async_receive.hpp','async_receive.cpp','async_receive_test_fixture.hpp','async_receive_tests.cpp','async_receive_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $asyncReceiveHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$asyncReceiveCombined=$asyncReceiveOutput.async_receive_tests+"`n"+$asyncReceiveOutput.async_receive_integration_tests
[ordered]@{
    result='PASS';compiler=$compiler;flags=$flags
    core_checks=[int]([regex]::Match($asyncReceiveOutput.async_receive_tests,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($asyncReceiveOutput.async_receive_integration_tests,'PASS: (\d+)').Groups[1].Value)
    output=$asyncReceiveCombined;source_hashes=$asyncReceiveHashes;original_procedures_added=1
    original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused'
    execution_scope='021d10 normal flow; exact injected WSARecv/GetLastError, actual allocation/publication/cache/023dd0 constructor and IO routing/027710/021a40, full64 reload, weak/strong transfer, EOF/error and actual operation lifetime ends;025c40 capture remains a required provider'
    native_equivalence='not established; typed projections and normal flow only; no native ABI/STL/SEH/concurrency/global/wholeDLL proof; no original target, socket or network execution'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'async-receive-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'async-receive-validation.json') -Destination (Join-Path $moduleRoot 'async-receive-validation.json')
Write-Output $asyncReceiveCombined

$receiveBufferOutput=@{}
foreach($case in @('receive_buffer_tests','receive_buffer_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    $bufferSupport=if($case -eq 'receive_buffer_tests'){@((Join-Path $moduleRoot 'receive_buffer.cpp'))}else{$asyncReceiveSupport+@((Join-Path $moduleRoot 'receive_buffer.cpp'))}
    & $compiler @flags @bufferSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Receive buffer compilation failed '+$case)}
    $receiveBufferOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $receiveBufferOutput[$case]}
}
$receiveBufferHashes=[ordered]@{}
foreach($name in @('receive_buffer.hpp','receive_buffer.cpp','receive_buffer_test_fixture.hpp','receive_buffer_tests.cpp','receive_buffer_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $receiveBufferHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$receiveBufferCombined=$receiveBufferOutput.receive_buffer_tests+"`n"+$receiveBufferOutput.receive_buffer_integration_tests
[ordered]@{
    result='PASS';compiler=$compiler;flags=$flags;tested_by='build.ps1'
    core_checks=[int]([regex]::Match($receiveBufferOutput.receive_buffer_tests,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($receiveBufferOutput.receive_buffer_integration_tests,'PASS: (\d+)').Groups[1].Value)
    output=$receiveBufferCombined;source_hashes=$receiveBufferHashes;original_procedures_added=2
    original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused'
    execution_scope='010040/008350 normal flows; bounded owned byte compatibility adapter, independent payload oracle, growth/alignment/header/free/failure prefixes, symbolic huge arithmetic; actual021d10/023dd0/SDK/027710/021a40 composition with controlled prepare/commit caller'
    native_equivalence='not established;025c40 caller remains open; no original CRT byte routine/STL/allocator/EH/SEH/concurrency/wholeDLL claim'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'receive-buffer-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'receive-buffer-validation.json') -Destination (Join-Path $moduleRoot 'receive-buffer-validation.json')
Write-Output $receiveBufferCombined

$composedReadSupport=$asyncReceiveSupport+@((Join-Path $moduleRoot 'receive_buffer.cpp'),(Join-Path $moduleRoot 'composed_read.cpp'))
$composedReadOutput=@{}
foreach($case in @('composed_read_tests','composed_read_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @composedReadSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Composed read compilation failed '+$case)}
    $composedReadOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $composedReadOutput[$case]}
}
$composedReadHashes=[ordered]@{}
foreach($name in @('composed_read.hpp','composed_read.cpp','composed_read_test_fixture.hpp','composed_read_tests.cpp','composed_read_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $composedReadHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
$composedReadCombined=$composedReadOutput.composed_read_tests+"`n"+$composedReadOutput.composed_read_integration_tests
[ordered]@{
    result='PASS';compiler=$compiler;flags=$flags;tested_by='build.ps1'
    core_checks=[int]([regex]::Match($composedReadOutput.composed_read_tests,'PASS: (\d+)').Groups[1].Value)
    integration_checks=[int]([regex]::Match($composedReadOutput.composed_read_integration_tests,'PASS: (\d+)').Groups[1].Value)
    output=$composedReadCombined;source_hashes=$composedReadHashes;original_procedures_added=1
    original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused'
    execution_scope='025c40 normal body; actual010040/008350/021d10/023dd0/IO route/027710/021a40 and borrowed/owned scheduling; isolated core callee capture, real callee in recovered bridge and integration; deterministic injected SDK and early/fallback protocol fixtures'
    native_equivalence='not established;025480/0280d0 frame bridge, native exception/category/allocator/storage/STL/EH/SEH/concurrency/global/ABI/fullDLL remain open; empty range nonzero iterator unsupported'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'composed-read-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'composed-read-validation.json') -Destination (Join-Path $moduleRoot 'composed-read-validation.json')
Write-Output $composedReadCombined

$frameReadSupport=$composedReadSupport+@((Join-Path $moduleRoot 'frame_read.cpp'))
$frameReadOutput=@{}
foreach($case in @('frame_read_tests','frame_read_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @frameReadSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Frame read compilation failed '+$case)}
    $frameReadOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $frameReadOutput[$case]}
}
$frameReadHashes=[ordered]@{}
foreach($name in @('frame_read.hpp','frame_read.cpp','frame_read_test_fixture.hpp','frame_read_tests.cpp','frame_read_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $frameReadHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$frameReadHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($frameReadOutput.frame_read_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($frameReadOutput.frame_read_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($frameReadOutput.frame_read_tests+"`n"+$frameReadOutput.frame_read_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='025480/0280d0 normal bodies; actual R3/R1/R2/021a40 loop; synthetic thread and controlled completion/environment adapters'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'frame-read-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'frame-read-validation.json') -Destination (Join-Path $moduleRoot 'frame-read-validation.json')
Write-Output ($frameReadOutput.frame_read_tests+"`n"+$frameReadOutput.frame_read_integration_tests)

$messageDispatchSupport=$frameReadSupport+@((Join-Path $moduleRoot 'message_dispatch.cpp'))
$messageDispatchOutput=@{}
foreach($case in @('message_dispatch_tests','message_dispatch_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    $support=$messageDispatchSupport
    if($case -eq 'message_dispatch_integration_tests'){
        $support+=@('../anadius-messages-source/messages.cpp','../anadius-xml-reader-source/reader.cpp','../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp','../anadius-xml-source/memory_printer.cpp')|ForEach-Object{Join-Path $moduleRoot $_}
    }
    & $compiler @flags @support (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Message dispatch compilation failed '+$case)}
    $messageDispatchOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $messageDispatchOutput[$case]}
}
$messageDispatchHashes=[ordered]@{}
foreach($name in @('message_dispatch.hpp','message_dispatch.cpp','message_dispatch_test_fixture.hpp','message_dispatch_tests.cpp','message_dispatch_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $messageDispatchHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$messageDispatchHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($messageDispatchOutput.message_dispatch_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($messageDispatchOutput.message_dispatch_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($messageDispatchOutput.message_dispatch_tests+"`n"+$messageDispatchOutput.message_dispatch_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='021770/028d30 normal bodies; actual025480 and unchanged05e8e0/05e7a8 plus existing033d70 source; selected string/XML compatibility, synthetic immediate/deferred thread and controlled021420 consumer'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'message-dispatch-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'message-dispatch-validation.json') -Destination (Join-Path $moduleRoot 'message-dispatch-validation.json')
Write-Output ($messageDispatchOutput.message_dispatch_tests+"`n"+$messageDispatchOutput.message_dispatch_integration_tests)

$responseDispatchSupport=$messageDispatchSupport+@((Join-Path $moduleRoot 'response_dispatch.cpp'))
$responseDispatchOutput=@{}
foreach($case in @('response_dispatch_tests','response_dispatch_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @responseDispatchSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Response dispatch compilation failed '+$case)}
    $responseDispatchOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $responseDispatchOutput[$case]}
}
$responseDispatchHashes=[ordered]@{}
foreach($name in @('response_dispatch.hpp','response_dispatch.cpp','response_dispatch_test_fixture.hpp','response_dispatch_tests.cpp','response_dispatch_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $responseDispatchHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$responseDispatchHashes;original_procedures_added=3
 core_checks=[int]([regex]::Match($responseDispatchOutput.response_dispatch_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($responseDispatchOutput.response_dispatch_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($responseDispatchOutput.response_dispatch_tests+"`n"+$responseDispatchOutput.response_dispatch_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='021420/028270/022240 normal bodies; actual028d30/023cb0 and queued-write/send/composed/finish chain; controlled owned functor/string copy/constructor/completion, injected send'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'response-dispatch-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'response-dispatch-validation.json') -Destination (Join-Path $moduleRoot 'response-dispatch-validation.json')
Write-Output ($responseDispatchOutput.response_dispatch_tests+"`n"+$responseDispatchOutput.response_dispatch_integration_tests)

$ownedResponseSupport=$responseDispatchSupport+@((Join-Path $moduleRoot 'owned_response.cpp'))
$ownedResponseOutput=@{}
foreach($case in @('owned_response_tests','owned_response_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @ownedResponseSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Owned response compilation failed '+$case)}
    $ownedResponseOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $ownedResponseOutput[$case]}
}
$ownedResponseHashes=[ordered]@{}
foreach($name in @('owned_response.hpp','owned_response.cpp','owned_response_test_fixture.hpp','owned_response_tests.cpp','owned_response_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $ownedResponseHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$ownedResponseHashes;original_procedures_added=3
 core_checks=[int]([regex]::Match($ownedResponseOutput.owned_response_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($ownedResponseOutput.owned_response_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($ownedResponseOutput.owned_response_tests+"`n"+$ownedResponseOutput.owned_response_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='020cb0/022280/0219d0 normal bodies with actual021900 and N1/N2/queued-write/send/finish; controlled009eb0/033d70/023dd0/completion and injected send'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'owned-response-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'owned-response-validation.json') -Destination (Join-Path $moduleRoot 'owned-response-validation.json')
Write-Output ($ownedResponseOutput.owned_response_tests+"`n"+$ownedResponseOutput.owned_response_integration_tests)

$responseContainersSupport=$ownedResponseSupport+@((Join-Path $moduleRoot 'response_containers.cpp'))
$responseContainersOutput=@{}
foreach($case in @('response_containers_tests','response_containers_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @responseContainersSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Response containers compilation failed '+$case)}
    $responseContainersOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $responseContainersOutput[$case]}
}
$responseContainersHashes=[ordered]@{}
foreach($name in @('response_containers.hpp','response_containers.cpp','response_containers_test_fixture.hpp','response_containers_tests.cpp','response_containers_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $responseContainersHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$responseContainersHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($responseContainersOutput.response_containers_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($responseContainersOutput.response_containers_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($responseContainersOutput.response_containers_tests+"`n"+$responseContainersOutput.response_containers_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='009eb0/026670 normal bodies and two provider closures; actual N1/N2/N3/queued-send/finish; controlled allocation/033d70/023dd0/completion and injected send'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'response-containers-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'response-containers-validation.json') -Destination (Join-Path $moduleRoot 'response-containers-validation.json')
Write-Output ($responseContainersOutput.response_containers_tests+"`n"+$responseContainersOutput.response_containers_integration_tests)

$messageCoreBridgeSupport=$responseContainersSupport+@((Join-Path $moduleRoot 'message_core_bridge.cpp'))+@((Join-Path $moduleRoot '../anadius-messages-source/messages.cpp'),(Join-Path $moduleRoot '../anadius-xml-reader-source/reader.cpp'),(Join-Path $moduleRoot '../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp'),(Join-Path $moduleRoot '../anadius-xml-source/memory_printer.cpp'))
$messageCoreBridgeOutput=@{}
foreach($case in @('message_core_bridge_tests','message_core_bridge_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @messageCoreBridgeSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Message core bridge compilation failed '+$case)}
    $messageCoreBridgeOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $messageCoreBridgeOutput[$case]}
}
$messageCoreBridgeHashes=[ordered]@{}
foreach($name in @('message_core_bridge.hpp','message_core_bridge.cpp','message_core_bridge_test_fixture.hpp','message_core_bridge_tests.cpp','message_core_bridge_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $messageCoreBridgeHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$messageCoreBridgeHashes;original_procedures_added=0
 core_checks=[int]([regex]::Match($messageCoreBridgeOutput.message_core_bridge_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($messageCoreBridgeOutput.message_core_bridge_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($messageCoreBridgeOutput.message_core_bridge_tests+"`n"+$messageCoreBridgeOutput.message_core_bridge_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='Existing033d70/router/XML connected via selected NORMAL-RETURN string/state adapter to actual N1/N2/N3/N4/send/finish; native exception/callback visibility open;0 new native bodies'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'message-core-bridge-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'message-core-bridge-validation.json') -Destination (Join-Path $moduleRoot 'message-core-bridge-validation.json')
Write-Output ($messageCoreBridgeOutput.message_core_bridge_tests+"`n"+$messageCoreBridgeOutput.message_core_bridge_integration_tests)

$requestMutationSupport=$messageCoreBridgeSupport+@((Join-Path $moduleRoot 'request_mutation.cpp'))
$requestMutationOutput=@{}
foreach($case in @('request_mutation_tests','request_mutation_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @requestMutationSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Request mutation compilation failed '+$case)}
    $requestMutationOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $requestMutationOutput[$case]}
}
$requestMutationHashes=[ordered]@{}
foreach($name in @('request_mutation.hpp','request_mutation.cpp','request_mutation_test_fixture.hpp','request_mutation_tests.cpp','request_mutation_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $requestMutationHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$requestMutationHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($requestMutationOutput.request_mutation_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($requestMutationOutput.request_mutation_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($requestMutationOutput.request_mutation_tests+"`n"+$requestMutationOutput.request_mutation_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='03ce30/034730 normal bodies, actual010040 plus controlled native prefix preparation before caller callback and N1-N5/send/finish;02b940 and full native033d70 remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'request-mutation-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'request-mutation-validation.json') -Destination (Join-Path $moduleRoot 'request-mutation-validation.json')
Write-Output ($requestMutationOutput.request_mutation_tests+"`n"+$requestMutationOutput.request_mutation_integration_tests)

$requestGrowthSupport=$requestMutationSupport+@((Join-Path $moduleRoot 'request_growth.cpp'))
$requestGrowthOutput=@{}
foreach($case in @('request_growth_tests','request_growth_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @requestGrowthSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Request growth compilation failed '+$case)}
    $requestGrowthOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $requestGrowthOutput[$case]}
}
$requestGrowthHashes=[ordered]@{}
foreach($name in @('request_growth.hpp','request_growth.cpp','request_growth_test_fixture.hpp','request_growth_tests.cpp','request_growth_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $requestGrowthHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$requestGrowthHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($requestGrowthOutput.request_growth_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($requestGrowthOutput.request_growth_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($requestGrowthOutput.request_growth_tests+"`n"+$requestGrowthOutput.request_growth_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='02b940 normal body and RequestMutationApi lower provider closure; actual034730 and controlled native request growth before N1-N6/send/finish; native033d70 remains open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'request-growth-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'request-growth-validation.json') -Destination (Join-Path $moduleRoot 'request-growth-validation.json')
Write-Output ($requestGrowthOutput.request_growth_tests+"`n"+$requestGrowthOutput.request_growth_integration_tests)

$responseHexSupport=$requestGrowthSupport+@((Join-Path $moduleRoot 'response_hex.cpp'))
$responseHexOutput=@{}
foreach($case in @('response_hex_tests','response_hex_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @responseHexSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Response hex compilation failed '+$case)}
    $responseHexOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $responseHexOutput[$case]}
}
$responseHexHashes=[ordered]@{}
foreach($name in @('response_hex.hpp','response_hex.cpp','response_hex_test_fixture.hpp','response_hex_tests.cpp','response_hex_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $responseHexHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$responseHexHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($responseHexOutput.response_hex_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($responseHexOutput.response_hex_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($responseHexOutput.response_hex_tests+"`n"+$responseHexOutput.response_hex_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='03d190 typed-prefix normal body; controlled response preparation before N1-N7/send/finish; native033d70 remains open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'response-hex-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'response-hex-validation.json') -Destination (Join-Path $moduleRoot 'response-hex-validation.json')
Write-Output ($responseHexOutput.response_hex_tests+"`n"+$responseHexOutput.response_hex_integration_tests)

$nativeMessageSupport=$responseHexSupport+@((Join-Path $moduleRoot 'native_message.cpp'))
$nativeMessageOutput=@{}
foreach($case in @('native_message_tests','native_message_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nativeMessageSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Native message compilation failed '+$case)}
    $nativeMessageOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nativeMessageOutput[$case]}
}
$nativeMessageHashes=[ordered]@{}
foreach($name in @('native_message.hpp','native_message.cpp','native_message_test_fixture.hpp','native_message_tests.cpp','native_message_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nativeMessageHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nativeMessageHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($nativeMessageOutput.native_message_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nativeMessageOutput.native_message_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nativeMessageOutput.native_message_tests+"`n"+$nativeMessageOutput.native_message_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='033d70 typed normal orchestration and final source bridge; actual codecs/retries/events to N1-N8/send/finish; mandatory lower providers/native ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'native-message-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'native-message-validation.json') -Destination (Join-Path $moduleRoot 'native-message-validation.json')
Write-Output ($nativeMessageOutput.native_message_tests+"`n"+$nativeMessageOutput.native_message_integration_tests)

$appendGrowthSupport=$nativeMessageSupport+@((Join-Path $moduleRoot 'append_growth.cpp'))
$appendGrowthOutput=@{}
foreach($case in @('append_growth_tests','append_growth_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @appendGrowthSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Append growth compilation failed '+$case)}
    $appendGrowthOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $appendGrowthOutput[$case]}
}
$appendGrowthHashes=[ordered]@{}
foreach($name in @('append_growth.hpp','append_growth.cpp','append_growth_test_fixture.hpp','append_growth_tests.cpp','append_growth_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $appendGrowthHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$appendGrowthHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($appendGrowthOutput.append_growth_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($appendGrowthOutput.append_growth_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($appendGrowthOutput.append_growth_tests+"`n"+$appendGrowthOutput.append_growth_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='012350/007e30 normal growth bodies and2 final lower-source bridges in033d70/N1-N9/send/finish; allocator/cipher/XML/log/ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'append-growth-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'append-growth-validation.json') -Destination (Join-Path $moduleRoot 'append-growth-validation.json')
Write-Output ($appendGrowthOutput.append_growth_tests+"`n"+$appendGrowthOutput.append_growth_integration_tests)

$prefixCipherSupport=$appendGrowthSupport+@((Join-Path $moduleRoot 'prefix_cipher.cpp'),(Join-Path $moduleRoot '../anadius-crypto-source/crypto.cpp'))
$prefixCipherOutput=@{}
foreach($case in @('prefix_cipher_tests','prefix_cipher_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @prefixCipherSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Prefix cipher compilation failed '+$case)}
    $prefixCipherOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $prefixCipherOutput[$case]}
}
$prefixCipherHashes=[ordered]@{}
foreach($name in @('prefix_cipher.hpp','prefix_cipher.cpp','prefix_cipher_test_fixture.hpp','prefix_cipher_tests.cpp','prefix_cipher_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $prefixCipherHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$prefixCipherHashes;original_procedures_added=6
 core_checks=[int]([regex]::Match($prefixCipherOutput.prefix_cipher_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($prefixCipherOutput.prefix_cipher_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($prefixCipherOutput.prefix_cipher_tests+"`n"+$prefixCipherOutput.prefix_cipher_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='042480/042610 plus4 ECB/CBC normal bodies and6 source bridges using existing AES block source; real request/response/event chain to N1-N10/send/finish; key binding/XML/log/native ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'prefix-cipher-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'prefix-cipher-validation.json') -Destination (Join-Path $moduleRoot 'prefix-cipher-validation.json')
Write-Output ($prefixCipherOutput.prefix_cipher_tests+"`n"+$prefixCipherOutput.prefix_cipher_integration_tests)

$cipherSessionSupport=$prefixCipherSupport+@((Join-Path $moduleRoot 'cipher_session.cpp'))
$cipherSessionOutput=@{}
foreach($case in @('cipher_session_tests','cipher_session_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @cipherSessionSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Cipher session compilation failed '+$case)}
    $cipherSessionOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $cipherSessionOutput[$case]}
}
$cipherSessionHashes=[ordered]@{}
foreach($name in @('cipher_session.hpp','cipher_session.cpp','cipher_session_test_fixture.hpp','cipher_session_tests.cpp','cipher_session_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $cipherSessionHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$cipherSessionHashes;original_procedures_added=4
 core_checks=[int]([regex]::Match($cipherSessionOutput.cipher_session_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($cipherSessionOutput.cipher_session_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($cipherSessionOutput.cipher_session_tests+"`n"+$cipherSessionOutput.cipher_session_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='0423d0/042760/042780/02bc70 control projections and2 semantic session/factory adapters; ChallengeResponse key coherence and real N1-N11/send/finish; native object/XML/heap/ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'cipher-session-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'cipher-session-validation.json') -Destination (Join-Path $moduleRoot 'cipher-session-validation.json')
Write-Output ($cipherSessionOutput.cipher_session_tests+"`n"+$cipherSessionOutput.cipher_session_integration_tests)

$nativeDispatchSupport=$cipherSessionSupport+@((Join-Path $moduleRoot 'native_dispatch.cpp'))
$nativeDispatchOutput=@{}
foreach($case in @('native_dispatch_tests','native_dispatch_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nativeDispatchSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Native dispatch compilation failed '+$case)}
    $nativeDispatchOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nativeDispatchOutput[$case]}
}
$nativeDispatchHashes=[ordered]@{}
foreach($name in @('native_dispatch.hpp','native_dispatch.cpp','native_dispatch_test_fixture.hpp','native_dispatch_tests.cpp','native_dispatch_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nativeDispatchHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nativeDispatchHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($nativeDispatchOutput.native_dispatch_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nativeDispatchOutput.native_dispatch_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nativeDispatchOutput.native_dispatch_tests+"`n"+$nativeDispatchOutput.native_dispatch_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='032d00 per-call normal projection and final dispatch binding; registry/DOM/printer/string/handler native providers and ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'native-dispatch-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'native-dispatch-validation.json') -Destination (Join-Path $moduleRoot 'native-dispatch-validation.json')
Write-Output ($nativeDispatchOutput.native_dispatch_tests+"`n"+$nativeDispatchOutput.native_dispatch_integration_tests)

$nativeDocumentSupport=$nativeDispatchSupport+@((Join-Path $moduleRoot 'native_document.cpp'))
$nativeDocumentOutput=@{}
foreach($case in @('native_document_tests','native_document_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nativeDocumentSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Native document compilation failed '+$case)}
    $nativeDocumentOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nativeDocumentOutput[$case]}
}
$nativeDocumentHashes=[ordered]@{}
foreach($name in @('native_document.hpp','native_document.cpp','native_document_test_fixture.hpp','native_document_tests.cpp','native_document_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nativeDocumentHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nativeDocumentHashes;original_procedures_added=3
 core_checks=[int]([regex]::Match($nativeDocumentOutput.native_document_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nativeDocumentOutput.native_document_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nativeDocumentOutput.native_document_tests+"`n"+$nativeDocumentOutput.native_document_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='001830/004280/001e00 ordered document projections and3 final bridges; shared live error cell; lower clear/parser/nodes/pools/allocator/vtable and ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'native-document-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'native-document-validation.json') -Destination (Join-Path $moduleRoot 'native-document-validation.json')
Write-Output ($nativeDocumentOutput.native_document_tests+"`n"+$nativeDocumentOutput.native_document_integration_tests)

$documentClearSupport=$nativeDocumentSupport+@((Join-Path $moduleRoot 'document_clear.cpp'))
$documentClearOutput=@{}
foreach($case in @('document_clear_tests','document_clear_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @documentClearSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Document clear compilation failed '+$case)}
    $documentClearOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $documentClearOutput[$case]}
}
$documentClearHashes=[ordered]@{}
foreach($name in @('document_clear.hpp','document_clear.cpp','document_clear_test_fixture.hpp','document_clear_tests.cpp','document_clear_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $documentClearHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$documentClearHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($documentClearOutput.document_clear_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($documentClearOutput.document_clear_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($documentClearOutput.document_clear_tests+"`n"+$documentClearOutput.document_clear_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='002e60/003110 clear/node normal projections and2 final effect bridges; live raw memory/shared error cell; lower003030/003240/node/pool virtual/parser/full DOM/free/ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'document-clear-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'document-clear-validation.json') -Destination (Join-Path $moduleRoot 'document-clear-validation.json')
Write-Output ($documentClearOutput.document_clear_tests+"`n"+$documentClearOutput.document_clear_integration_tests)

$nodeDeleteSupport=$documentClearSupport+@((Join-Path $moduleRoot 'node_delete.cpp'))
$nodeDeleteOutput=@{}
foreach($case in @('node_delete_tests','node_delete_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeDeleteSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node delete compilation failed '+$case)}
    $nodeDeleteOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeDeleteOutput[$case]}
}
$nodeDeleteHashes=[ordered]@{}
foreach($name in @('node_delete.hpp','node_delete.cpp','node_delete_test_fixture.hpp','node_delete_tests.cpp','node_delete_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeDeleteHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeDeleteHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($nodeDeleteOutput.node_delete_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeDeleteOutput.node_delete_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeDeleteOutput.node_delete_tests+"`n"+$nodeDeleteOutput.node_delete_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='003030/003240 normal node unlink/delete projections and2 final helper bridges; distinct count contracts; virtual/parser/full DOM/pool/free/ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-delete-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-delete-validation.json') -Destination (Join-Path $moduleRoot 'node-delete-validation.json')
Write-Output ($nodeDeleteOutput.node_delete_tests+"`n"+$nodeDeleteOutput.node_delete_integration_tests)

$poolReleaseSupport=$nodeDeleteSupport+@((Join-Path $moduleRoot 'pool_release.cpp'))
$poolReleaseOutput=@{}
foreach($case in @('pool_release_tests','pool_release_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @poolReleaseSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Pool release compilation failed '+$case)}
    $poolReleaseOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $poolReleaseOutput[$case]}
}
$poolReleaseHashes=[ordered]@{}
foreach($name in @('pool_release.hpp','pool_release.cpp','pool_release_test_fixture.hpp','pool_release_tests.cpp','pool_release_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $poolReleaseHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$poolReleaseHashes;original_procedures_added=2
 core_checks=[int]([regex]::Match($poolReleaseOutput.pool_release_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($poolReleaseOutput.pool_release_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($poolReleaseOutput.pool_release_tests+"`n"+$poolReleaseOutput.pool_release_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='0033a0/005a20 memory-only pool release/tracking and2 generic final source bindings; allocator/nodevirtual/parser/full DOM/ABI/EH remain open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'pool-release-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'pool-release-validation.json') -Destination (Join-Path $moduleRoot 'pool-release-validation.json')
Write-Output ($poolReleaseOutput.pool_release_tests+"`n"+$poolReleaseOutput.pool_release_integration_tests)

$poolAllocateSupport=$poolReleaseSupport+@((Join-Path $moduleRoot 'pool_allocate.cpp'))
$poolAllocateOutput=@{}
foreach($case in @('pool_allocate_tests','pool_allocate_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @poolAllocateSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Pool allocate compilation failed '+$case)}
    $poolAllocateOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $poolAllocateOutput[$case]}
}
$poolAllocateHashes=[ordered]@{}
foreach($name in @('pool_allocate.hpp','pool_allocate.cpp','pool_allocate_test_fixture.hpp','pool_allocate_tests.cpp','pool_allocate_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $poolAllocateHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$poolAllocateHashes;original_procedures_added=4
 core_checks=[int]([regex]::Match($poolAllocateOutput.pool_allocate_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($poolAllocateOutput.pool_allocate_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($poolAllocateOutput.pool_allocate_tests+"`n"+$poolAllocateOutput.pool_allocate_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='002b10/002600/002960/0027b0 pool allocation normal byte-state bodies; one new source dispatcher adapter, no historical provider closure; heap/fill/append/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'pool-allocation-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'pool-allocation-validation.json') -Destination (Join-Path $moduleRoot 'pool-allocation-validation.json')
Write-Output ($poolAllocateOutput.pool_allocate_tests+"`n"+$poolAllocateOutput.pool_allocate_integration_tests)

$poolAppendSupport=$poolAllocateSupport+@((Join-Path $moduleRoot 'pool_append.cpp'))
$poolAppendOutput=@{}
foreach($case in @('pool_append_tests','pool_append_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @poolAppendSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Pool append compilation failed '+$case)}
    $poolAppendOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $poolAppendOutput[$case]}
}
$poolAppendHashes=[ordered]@{}
foreach($name in @('pool_append.hpp','pool_append.cpp','pool_append_test_fixture.hpp','pool_append_tests.cpp','pool_append_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $poolAppendHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$poolAppendHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($poolAppendOutput.pool_append_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($poolAppendOutput.pool_append_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($poolAppendOutput.pool_append_tests+"`n"+$poolAppendOutput.pool_append_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='005440 pointer-array append normal body and final N18 provider bridge; raw32 signed growth, saturated allocation bytes and live copy/free/append ordering; heap/copy/fill/free/lifecycle/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'pool-append-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'pool-append-validation.json') -Destination (Join-Path $moduleRoot 'pool-append-validation.json')
Write-Output ($poolAppendOutput.pool_append_tests+"`n"+$poolAppendOutput.pool_append_integration_tests)

$poolClearSupport=$poolAppendSupport+@((Join-Path $moduleRoot 'pool_clear.cpp'))
$poolClearOutput=@{}
foreach($case in @('pool_clear_tests','pool_clear_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @poolClearSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Pool clear compilation failed '+$case)}
    $poolClearOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $poolClearOutput[$case]}
}
$poolClearHashes=[ordered]@{}
foreach($name in @('pool_clear.hpp','pool_clear.cpp','pool_clear_test_fixture.hpp','pool_clear_tests.cpp','pool_clear_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $poolClearHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$poolClearHashes;original_procedures_added=3
 core_checks=[int]([regex]::Match($poolClearOutput.pool_clear_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($poolClearOutput.pool_clear_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($poolClearOutput.pool_clear_tests+"`n"+$poolClearOutput.pool_clear_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='002d70/002dc0/002e10 normal pool block clear projections and3 NativeDocumentEffects final source bridges; raw count/signed index/decrement before live slot/free; ordered QWORD resets; pointer free/destructors/parser/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'pool-clear-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'pool-clear-validation.json') -Destination (Join-Path $moduleRoot 'pool-clear-validation.json')
Write-Output ($poolClearOutput.pool_clear_tests+"`n"+$poolClearOutput.pool_clear_integration_tests)

$poolDestructSupport=$poolClearSupport+@((Join-Path $moduleRoot 'pool_destruct.cpp'))
$poolDestructOutput=@{}
foreach($case in @('pool_destruct_tests','pool_destruct_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @poolDestructSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Pool destruct compilation failed '+$case)}
    $poolDestructOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $poolDestructOutput[$case]}
}
$poolDestructHashes=[ordered]@{}
foreach($name in @('pool_destruct.hpp','pool_destruct.cpp','pool_destruct_test_fixture.hpp','pool_destruct_tests.cpp','pool_destruct_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $poolDestructHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$poolDestructHashes;original_procedures_added=4
 core_checks=[int]([regex]::Match($poolDestructOutput.pool_destruct_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($poolDestructOutput.pool_destruct_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($poolDestructOutput.pool_destruct_tests+"`n"+$poolDestructOutput.pool_destruct_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='001bb0/001c30/001cb0/001d30 normal pool destructor projections and4 NativeDocumentEffects final source bridges; cached entry before type vtable, live block frees, ordered QWORD resets, optional array free and base vtable last; pointer free/nodebase/parser/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'pool-destruct-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'pool-destruct-validation.json') -Destination (Join-Path $moduleRoot 'pool-destruct-validation.json')
Write-Output ($poolDestructOutput.pool_destruct_tests+"`n"+$poolDestructOutput.pool_destruct_integration_tests)

$nodeBaseSupport=$poolDestructSupport+@((Join-Path $moduleRoot 'node_base.cpp'))
$nodeBaseOutput=@{}
foreach($case in @('node_base_tests','node_base_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeBaseSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node base compilation failed '+$case)}
    $nodeBaseOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeBaseOutput[$case]}
}
$nodeBaseHashes=[ordered]@{}
foreach($name in @('node_base.hpp','node_base.cpp','node_base_test_fixture.hpp','node_base_tests.cpp','node_base_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeBaseHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeBaseHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($nodeBaseOutput.node_base_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeBaseOutput.node_base_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeBaseOutput.node_base_tests+"`n"+$nodeBaseOutput.node_base_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='001ea0 normal node-base destructor projection and1 NativeDocumentEffects final source bridge; existing003110 reuse, live parent/sibling unlink, flagged pointer-free and ordered storage resets; node virtual/free/parser/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-base-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-base-validation.json') -Destination (Join-Path $moduleRoot 'node-base-validation.json')
Write-Output ($nodeBaseOutput.node_base_tests+"`n"+$nodeBaseOutput.node_base_integration_tests)

$parseEntrySupport=$nodeBaseSupport+@((Join-Path $moduleRoot 'parse_entry.cpp'))
$parseEntryOutput=@{}
foreach($case in @('parse_entry_tests','parse_entry_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @parseEntrySupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Parse entry compilation failed '+$case)}
    $parseEntryOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $parseEntryOutput[$case]}
}
$parseEntryHashes=[ordered]@{}
foreach($name in @('parse_entry.hpp','parse_entry.cpp','parse_entry_test_fixture.hpp','parse_entry_tests.cpp','parse_entry_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $parseEntryHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$parseEntryHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($parseEntryOutput.parse_entry_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($parseEntryOutput.parse_entry_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($parseEntryOutput.parse_entry_tests+"`n"+$parseEntryOutput.parse_entry_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='0041b0 normal parse-entry prelude and1 final document provider source bridge; cached cursor/live line,BOM/error13/live slot80; classifier/error/004970 parser body/DOM/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'parse-entry-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'parse-entry-validation.json') -Destination (Join-Path $moduleRoot 'parse-entry-validation.json')
Write-Output ($parseEntryOutput.parse_entry_tests+"`n"+$parseEntryOutput.parse_entry_integration_tests)

$documentErrorSupport=$parseEntrySupport+@((Join-Path $moduleRoot 'document_error.cpp'))
$documentErrorOutput=@{}
foreach($case in @('document_error_tests','document_error_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @documentErrorSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Document error compilation failed '+$case)}
    $documentErrorOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $documentErrorOutput[$case]}
}
$documentErrorHashes=[ordered]@{}
foreach($name in @('document_error.hpp','document_error.cpp','document_error_test_fixture.hpp','document_error_tests.cpp','document_error_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $documentErrorHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$documentErrorHashes;original_procedures_added=1
 core_checks=[int]([regex]::Match($documentErrorOutput.document_error_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($documentErrorOutput.document_error_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($documentErrorOutput.document_error_tests+"`n"+$documentErrorOutput.document_error_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='005820 normal error-storage flow and1 final document provider source bridge; cached error/line,signed name-table index,two live ownership-release phases,mandatory formatting/opaque supplied argument tail,post-copy live start; formatter/native varargs/heap/ABI/EH open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'document-error-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'document-error-validation.json') -Destination (Join-Path $moduleRoot 'document-error-validation.json')
Write-Output ($documentErrorOutput.document_error_tests+"`n"+$documentErrorOutput.document_error_integration_tests)

$nodeParseSupport=$documentErrorSupport+@((Join-Path $moduleRoot 'node_parse.cpp'))
$nodeParseOutput=@{}
foreach($case in @('node_parse_tests','node_parse_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeParseSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node parse compilation failed '+$case)}
    $nodeParseOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeParseOutput[$case]}
}
$nodeParseHashes=[ordered]@{}
foreach($name in @('node_parse.hpp','node_parse.cpp','node_parse_test_fixture.hpp','node_parse_tests.cpp','node_parse_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeParseHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeParseHashes;original_procedures_added=1;new_source_bridges=0;new_source_dispatch_adapters=1
 core_checks=[int]([regex]::Match($nodeParseOutput.node_parse_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeParseOutput.node_parse_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeParseOutput.node_parse_tests+"`n"+$nodeParseOutput.node_parse_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='004970 normal node parser byte-state body and one selected-target dispatch source adapter; explicit mapped invocation scratch,depth guard,declaration/closing/error/list/cleanup ordering; other virtual targets,parser primitives,error-vararg transport/heap/nativeABI/EH remain mandatory'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-parse-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-parse-validation.json') -Destination (Join-Path $moduleRoot 'node-parse-validation.json')
Write-Output ($nodeParseOutput.node_parse_tests+"`n"+$nodeParseOutput.node_parse_integration_tests)

$nodeIdentifySupport=$nodeParseSupport+@((Join-Path $moduleRoot 'node_identify.cpp'))
$nodeIdentifyOutput=@{}
foreach($case in @('node_identify_tests','node_identify_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeIdentifySupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node identify compilation failed '+$case)}
    $nodeIdentifyOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeIdentifyOutput[$case]}
}
$nodeIdentifyHashes=[ordered]@{}
foreach($name in @('node_identify.hpp','node_identify.cpp','node_identify_test_fixture.hpp','node_identify_tests.cpp','node_identify_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeIdentifyHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeIdentifyHashes;original_procedures_added=1;new_source_bridges=1;new_source_dispatch_adapters=0;native_typed_providers_newly_closed=1
 core_checks=[int]([regex]::Match($nodeIdentifyOutput.node_identify_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeIdentifyOutput.node_identify_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeIdentifyOutput.node_identify_tests+"`n"+$nodeIdentifyOutput.node_identify_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='0038b0 normal Identify body and one final source provider bridge; cached initial line/live LF,ordered live token-slot loads,pointer bypass,typed constructor routing,CDATA BYTE flag and fallback original-cursor/line rewind; constructors/classifier/compare/token data/native ABI/EH remain mandatory'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-identify-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-identify-validation.json') -Destination (Join-Path $moduleRoot 'node-identify-validation.json')
Write-Output ($nodeIdentifyOutput.node_identify_tests+"`n"+$nodeIdentifyOutput.node_identify_integration_tests)

$nodeConstructorsSupport=$nodeIdentifySupport+@((Join-Path $moduleRoot 'node_constructors.cpp'))
$nodeConstructorsOutput=@{}
foreach($case in @('node_constructors_tests','node_constructors_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeConstructorsSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node constructors compilation failed '+$case)}
    $nodeConstructorsOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeConstructorsOutput[$case]}
}
$nodeConstructorsHashes=[ordered]@{}
foreach($name in @('node_constructors.hpp','node_constructors.cpp','node_constructors_test_fixture.hpp','node_constructors_tests.cpp','node_constructors_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeConstructorsHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeConstructorsHashes;original_procedures_added=5;new_source_bridges=5;new_source_dispatch_adapters=0;native_typed_providers_newly_closed=5
 core_checks=[int]([regex]::Match($nodeConstructorsOutput.node_constructors_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeConstructorsOutput.node_constructors_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeConstructorsOutput.node_constructors_tests+"`n"+$nodeConstructorsOutput.node_constructors_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='Five normal constructor bodies1590/1610/1690/1720/17b0 and five final source bridges; live pool target,ordered typed field stores,padding,cacheddoc/pool/node and existing005440source reuse; allocator/heap/fill/copy/free/nativeABI/EH remain mandatory'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-constructors-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-constructors-validation.json') -Destination (Join-Path $moduleRoot 'node-constructors-validation.json')
Write-Output ($nodeConstructorsOutput.node_constructors_tests+"`n"+$nodeConstructorsOutput.node_constructors_integration_tests)

$nodeInsertionSupport=$nodeConstructorsSupport+@((Join-Path $moduleRoot 'node_insertion.cpp'))
$nodeInsertionOutput=@{}
foreach($case in @('node_insertion_tests','node_insertion_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeInsertionSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node insertion compilation failed '+$case)}
    $nodeInsertionOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeInsertionOutput[$case]}
}
$nodeInsertionHashes=[ordered]@{}
foreach($name in @('node_insertion.hpp','node_insertion.cpp','node_insertion_test_fixture.hpp','node_insertion_tests.cpp','node_insertion_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeInsertionHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeInsertionHashes;original_procedures_added=1;new_source_bridges=1;new_source_dispatch_adapters=0;native_typed_providers_newly_closed=1
 core_checks=[int]([regex]::Match($nodeInsertionOutput.node_insertion_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeInsertionOutput.node_insertion_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeInsertionOutput.node_insertion_tests+"`n"+$nodeInsertionOutput.node_insertion_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='003aa0 normal insertion-preparation body and one final parser provider bridge; ignored incoming owner,linked detach-only,unlinked cached signed search/live unsigned replacement/live DEC and live pool track; node destruction absent,track/native ABI/EH remain lower boundaries'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-insertion-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-insertion-validation.json') -Destination (Join-Path $moduleRoot 'node-insertion-validation.json')
Write-Output ($nodeInsertionOutput.node_insertion_tests+"`n"+$nodeInsertionOutput.node_insertion_integration_tests)

$nodeStringSupport=$nodeInsertionSupport+@((Join-Path $moduleRoot 'node_string.cpp'))
$nodeStringOutput=@{}
foreach($case in @('node_string_tests','node_string_integration_tests')){
    $target=Join-Path $buildRoot ($case+'.exe')
    & $compiler @flags @nodeStringSupport (Join-Path $moduleRoot ($case+'.cpp')) '-lws2_32' '-o' $target
    if($LASTEXITCODE -ne 0){throw ('Node string compilation failed '+$case)}
    $nodeStringOutput[$case]=(& $target 2>&1|Out-String).Trim()
    if($LASTEXITCODE -ne 0){throw $nodeStringOutput[$case]}
}
$nodeStringHashes=[ordered]@{}
foreach($name in @('node_string.hpp','node_string.cpp','node_string_test_fixture.hpp','node_string_tests.cpp','node_string_integration_tests.cpp','build.ps1','CMakeLists.txt')){
    $nodeStringHashes[$name]=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot $name)).Hash.ToLowerInvariant()
}
[ordered]@{
 result='PASS';tested_by='build.ps1';source_hashes=$nodeStringHashes;original_procedures_added=1;new_source_bridges=1;new_source_dispatch_adapters=0;native_typed_providers_newly_closed=1
 core_checks=[int]([regex]::Match($nodeStringOutput.node_string_tests,'PASS: (\d+)').Groups[1].Value)
 integration_checks=[int]([regex]::Match($nodeStringOutput.node_string_integration_tests,'PASS: (\d+)').Groups[1].Value)
 output=($nodeStringOutput.node_string_tests+"`n"+$nodeStringOutput.node_string_integration_tests)
 original_execution='none';real_user_files_read=$false;CRT_production='unchanged and paused';native_equivalence='not established'
 execution_scope='0035e0 normal string materialization and final existing parser provider; ordered flags,live bounds,newline/entity processing and whitespace compaction; numeric decoder,copy,compare,classifier,native stack/ABI/EH remain supplied or open'
}|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $buildRoot 'node-string-validation.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $buildRoot 'node-string-validation.json') -Destination (Join-Path $moduleRoot 'node-string-validation.json')
Write-Output ($nodeStringOutput.node_string_tests+"`n"+$nodeStringOutput.node_string_integration_tests)

# Every older validation is rerun by this script; bind its common build configs
# to the current final bytes as well, preserving each stage's other evidence.
$buildConfigHash=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot 'build.ps1')).Hash.ToLowerInvariant()
$cmakeConfigHash=(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $moduleRoot 'CMakeLists.txt')).Hash.ToLowerInvariant()
foreach($proofPath in Get-ChildItem -LiteralPath $buildRoot -Filter '*validation.json'){
    $proof=Get-Content -LiteralPath $proofPath.FullName -Raw|ConvertFrom-Json
    if(-not $proof.source_hashes){continue}
    $proof.source_hashes.'build.ps1'=$buildConfigHash
    $proof.source_hashes.'CMakeLists.txt'=$cmakeConfigHash
    $proof|ConvertTo-Json -Depth 12|Set-Content -LiteralPath $proofPath.FullName -Encoding utf8
    Copy-Item -LiteralPath $proofPath.FullName -Destination (Join-Path $moduleRoot $proofPath.Name)
}
