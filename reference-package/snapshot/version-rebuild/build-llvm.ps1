param(
    [Parameter(Mandatory=$true)][string]$ToolchainPath,
    [string]$SourceArchivePath = '',
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'build-llvm')
)
$ErrorActionPreference = 'Stop'
$toolchain = (Resolve-Path -LiteralPath $ToolchainPath).Path
$bin = Join-Path $toolchain 'bin'
$clang = Join-Path $bin 'x86_64-w64-mingw32-clang++.exe'
$ml = Join-Path $bin 'llvm-ml.exe'
$readobj = Join-Path $bin 'llvm-readobj.exe'
$objdump = Join-Path $bin 'llvm-objdump.exe'
$rc = Join-Path $bin 'llvm-rc.exe'
foreach ($path in @($clang,$ml,$readobj,$objdump,$rc)) { if (-not (Test-Path -LiteralPath $path)) { throw "Missing tool $path" } }
$archiveHash = if ($SourceArchivePath) {(Get-FileHash -LiteralPath $SourceArchivePath -Algorithm SHA256).Hash.ToLowerInvariant()} else {$null}
if ($archiveHash -and $archiveHash -ne '317492c456aa27ee607a5919f1d2d38dcdc1112516a24d0bf4b00d078f52d17a') { throw 'Toolchain archive digest mismatch' }
$null = New-Item -ItemType Directory -Path $BuildDirectory -Force
$build = (Resolve-Path -LiteralPath $BuildDirectory).Path
$commands = New-Object System.Collections.Generic.List[object]
function Invoke-Tool([string]$Exe, [string[]]$Arguments) {
    $commands.Add([pscustomobject]@{exe=$Exe; arguments=$Arguments})
    $output = & $Exe @Arguments 2>&1
    if ($output) { $output | Out-Host }
    if ($LASTEXITCODE -ne 0) { throw "Tool failed with exit $LASTEXITCODE : $Exe" }
}
$loader = Join-Path $build 'loader.obj'; $wrappers = Join-Path $build 'wrappers.obj'
$resource = Join-Path $build 'version.res'
$dll = Join-Path $build 'version.dll'; $map = Join-Path $build 'version.map'
$utf8 = New-Object System.Text.UTF8Encoding($false)
# llvm-ml rejects MSVC's casemap option and .const shortcut. These changes only
# restate assembler directives; every instruction and operand stays identical.
$llvmAssembly = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'wrappers.asm') -Raw).
    Replace('option casemap:none','; llvm-ml uses case-sensitive symbols').
    Replace('.const','.rdata SEGMENT READONLY').Replace('.code',".rdata ENDS`r`n.code")
$llvmAssemblyPath = Join-Path $build 'wrappers-llvm.asm'
[IO.File]::WriteAllText($llvmAssemblyPath,$llvmAssembly,$utf8)
Invoke-Tool $clang @('-c','-O2','-std=c++17','-fno-exceptions','-fno-rtti','-fno-stack-protector','-fno-builtin',(Join-Path $PSScriptRoot 'loader.cpp'),'-o',$loader)
Invoke-Tool $ml @('--m64','/c','/Fo',$wrappers,$llvmAssemblyPath)
Invoke-Tool $rc @('/fo',$resource,(Join-Path $PSScriptRoot 'version.rc'))
Invoke-Tool $clang @('-shared','-nostdlib','-Wl,--entry,ReconstructedDllMain','-Wl,--image-base,0x180000000','-Wl,--no-insert-timestamp','-Wl,--exclude-all-symbols',('-Wl,-Map,'+$map),$loader,$wrappers,$resource,(Join-Path $PSScriptRoot 'version.def'),'-lkernel32','-o',$dll)
$headers = & $readobj '--file-headers' '--sections' '--coff-exports' '--coff-imports' $dll 2>&1
if ($LASTEXITCODE -ne 0) { throw 'llvm-readobj failed' }
$disassembly = & $objdump '-d' '--x86-asm-syntax=intel' '--no-show-raw-insn' '--print-imm-hex' $dll 2>&1
if ($LASTEXITCODE -ne 0) { throw 'llvm-objdump failed' }
[IO.File]::WriteAllText((Join-Path $build 'readobj.txt'),($headers -join "`r`n"),$utf8)
[IO.File]::WriteAllText((Join-Path $build 'disassembly.txt'),($disassembly -join "`r`n"),$utf8)
$toolHashes=[ordered]@{}
foreach ($name in @('x86_64-w64-mingw32-clang++.exe','clang-23.exe','llvm-ml.exe','llvm-rc.exe','ld.lld.exe','libclang-cpp.dll')) {
    $path=Join-Path $bin $name
    if (Test-Path -LiteralPath $path) {$toolHashes[$name]=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()}
}
$record = [ordered]@{
    compiler_version=(& $clang '--version') -join "`n"
    assembler_version=(& $ml '/help' | Select-Object -First 2) -join "`n"
    distribution='llvm-mingw 20261006 UCRT x86_64, official mstorsjo/llvm-mingw GitHub release'
    source_url='https://github.com/mstorsjo/llvm-mingw/releases/download/20261006/llvm-mingw-20261006-ucrt-x86_64.zip'
    source_archive_sha256=$archiveHash
    expected_github_asset_sha256='317492c456aa27ee607a5919f1d2d38dcdc1112516a24d0bf4b00d078f52d17a'
    tool_sha256=$toolHashes
    assembly_portability='Only option casemap:none removed, .const replaced with explicit .rdata SEGMENT READONLY / ENDS; instructions unchanged.'
    output_sha256=(Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash.ToLowerInvariant()
    commands=$commands
    execution='Only compiler, assembler, linker and read-only inspection tools executed; DLL never loaded or executed.'
}
[IO.File]::WriteAllText((Join-Path $build 'build-record.json'),($record | ConvertTo-Json -Depth 7),$utf8)
$record | ConvertTo-Json -Depth 7
