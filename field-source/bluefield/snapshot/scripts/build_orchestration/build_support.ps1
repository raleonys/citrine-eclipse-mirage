# SPDX-License-Identifier: GPL-3.0-only
# Internal local-build helpers; scoped HF19 environment/locking contracts.
function Clear-CompilerSelectors {
    foreach($name in @('CL','_CL_','LINK','_LINK_','LINK_REPRO','CC','CXX','CMAKE_GENERATOR','CMAKE_GENERATOR_INSTANCE','CMAKE_TOOLCHAIN_FILE')) {
        [Environment]::SetEnvironmentVariable($name,$null,'Process')
    }
}
function Open-BuildLock {
    param([string]$Path)
    try {
        return [IO.File]::Open($Path,[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    } catch {
        throw "Another build owns this output directory: $Path"
    }
}
