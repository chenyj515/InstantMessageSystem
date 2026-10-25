param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$ToolchainBin = '',
    [switch]$RunTests
)
$ErrorActionPreference = 'Stop'
if ($ToolchainBin) {
    if (-not (Test-Path -LiteralPath $ToolchainBin -PathType Container)) { throw 'ToolchainBin does not exist' }
    $env:PATH = "$ToolchainBin;$env:PATH"
}
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake) { throw 'CMake 3.16+ is required. Open a developer terminal or supply -ToolchainBin.' }
$buildDir = Join-Path $PSScriptRoot 'build'
$configureArgs = @('-S', $PSScriptRoot, '-B', $buildDir, "-DCMAKE_BUILD_TYPE=$Configuration")
if (Get-Command g++ -ErrorAction SilentlyContinue) { $configureArgs += @('-G', 'MinGW Makefiles') }
& $cmake @configureArgs
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
& $cmake --build $buildDir --config $Configuration --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
if ($RunTests) {
    $ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
    & $ctest --test-dir $buildDir -C $Configuration --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
}
Write-Host "Build complete: $buildDir"
