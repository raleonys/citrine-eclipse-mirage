# Dot-source this file in the shell used for project maintenance.
# This selects tools only; it does not build, test, link or execute the DLL.
$env:GHIDRA_INSTALL_DIR='C:\tools\ghidra_12.1.4_PUBLIC'
$env:GHIDRA_LATEST_INSTALL_DIR='C:\tools\ghidra_12.2_PUBLIC'
$env:JAVA_HOME='C:\tools\jdk-25.0.5+8'
$env:ANADIUS_TOOLCHAIN_ROOT=Join-Path $PSScriptRoot 'toolchain\extracted\llvm-mingw-20261020-ucrt-x86_64'
$env:REA_COMMAND='C:\tools\rea-6.8.0\node_modules\rea-agents\scripts\rea.mjs'
$toolPaths=@('C:\tools\node-v26.13.0-win-x64','C:\tools\rea-6.8.0\node_modules\.bin',
    (Join-Path $env:JAVA_HOME 'bin'),(Join-Path $env:ANADIUS_TOOLCHAIN_ROOT 'bin'))
$priorPaths=@($env:Path -split ';'|Where-Object {$_ -and $_ -notin $toolPaths})
$env:Path=($toolPaths+$priorPaths)-join ';'
Remove-Variable toolPaths,priorPaths
