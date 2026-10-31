[CmdletBinding()]
param([switch]$Clean)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

function Invoke-Checked {
    param([string]$FilePath, [string[]]$Arguments, [string]$Description)
    Write-Host ""
    Write-Host "== $Description ==" -ForegroundColor Cyan
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Description failed with exit code $LASTEXITCODE." }
}

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$manifestPath = Join-Path $projectRoot "vcpkg.json"
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
    throw "vcpkg.json was not found. Run this script from a valid FPV Signal Trainer checkout."
}
if ($env:OS -ne "Windows_NT") { throw "FPV Signal Trainer currently supports Windows x64 only." }

$git = Get-Command git.exe -ErrorAction SilentlyContinue
if (-not $git) { throw "Git for Windows is required. Install Git, reopen PowerShell, and retry." }

$manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$baseline = [string]$manifest.'builtin-baseline'
if ($baseline -notmatch '^[0-9a-f]{40}$') {
    throw "vcpkg.json must contain a valid pinned builtin-baseline commit."
}

$localRoot = Join-Path $env:LOCALAPPDATA "FPVSignalTrainer"
$toolsRoot = Join-Path $localRoot "tools"
$vcpkgRoot = Join-Path $toolsRoot "vcpkg-$baseline"
$binaryCache = Join-Path $localRoot "vcpkg-binary-cache"
New-Item -ItemType Directory -Force -Path $toolsRoot, $binaryCache | Out-Null

Write-Host "FPV Signal Trainer dependency bootstrap" -ForegroundColor Green
Write-Host "Project : $projectRoot"
Write-Host "vcpkg   : $vcpkgRoot"
Write-Host "Baseline: $baseline"

if (-not (Test-Path -LiteralPath (Join-Path $vcpkgRoot ".git"))) {
    if (Test-Path -LiteralPath $vcpkgRoot) {
        Remove-Item -LiteralPath $vcpkgRoot -Recurse -Force
    }
    Invoke-Checked -FilePath $git.Source -Arguments @(
        "clone", "--filter=blob:none", "--no-checkout",
        "https://github.com/microsoft/vcpkg.git", $vcpkgRoot
    ) -Description "Download vcpkg source"
}

Invoke-Checked -FilePath $git.Source -Arguments @(
    "-C", $vcpkgRoot, "fetch", "--depth=1", "origin", $baseline
) -Description "Fetch pinned vcpkg baseline"
Invoke-Checked -FilePath $git.Source -Arguments @(
    "-C", $vcpkgRoot, "checkout", "--detach", "FETCH_HEAD"
) -Description "Select pinned vcpkg version"

$bootstrap = Join-Path $vcpkgRoot "bootstrap-vcpkg.bat"
if (-not (Test-Path -LiteralPath $bootstrap -PathType Leaf)) {
    throw "vcpkg bootstrap script is missing: $bootstrap"
}
Invoke-Checked -FilePath $bootstrap -Arguments @("-disableMetrics") -Description "Build vcpkg package manager"

$env:VCPKG_ROOT = $vcpkgRoot
$env:VCPKG_DEFAULT_BINARY_CACHE = $binaryCache
$env:PATH = "$vcpkgRoot;$env:PATH"

$cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
$cmake = if ($cmakeCommand) { $cmakeCommand.Source } else { $null }

$programFilesX86 = [Environment]::GetFolderPath("ProgramFilesX86")
$vswhere = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"

if (-not $cmake -and (Test-Path -LiteralPath $vswhere)) {
    $installPath = & $vswhere -latest -products "*" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -eq 0 -and $installPath) {
        $candidate = Join-Path $installPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        if (Test-Path -LiteralPath $candidate) { $cmake = $candidate }
    }
}
if (-not $cmake) {
    throw "CMake 3.25+ was not found. Install Visual Studio 2022 with Desktop development with C++ and its CMake component, or install CMake from https://cmake.org/download/."
}

$cmakeVersionText = & $cmake --version
if ($LASTEXITCODE -ne 0) { throw "CMake could not be started." }
Write-Host ($cmakeVersionText | Select-Object -First 1)

if (Test-Path -LiteralPath $vswhere) {
    $vsInstall = & $vswhere -latest -products "*" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $vsInstall) {
        throw "MSVC x64 tools were not found. Install Visual Studio 2022 with Desktop development with C++."
    }
}

if ($Clean) {
    $buildRoot = Join-Path $projectRoot "out"
    if (Test-Path -LiteralPath $buildRoot) { Remove-Item -LiteralPath $buildRoot -Recurse -Force }
}

# Manifest mode resolves and downloads every C++ dependency declared by the project.
Invoke-Checked -FilePath (Join-Path $vcpkgRoot "vcpkg.exe") -Arguments @(
    "install", "--triplet", "x64-windows-static", "--x-manifest-root", $projectRoot
) -Description "Download and install project libraries"

Invoke-Checked -FilePath $cmake -Arguments @("--preset", "windows-msvc") -Description "Configure CMake project"
Invoke-Checked -FilePath $cmake -Arguments @("--build", "--preset", "windows-release", "--parallel") -Description "Compile FPV Signal Trainer"
Invoke-Checked -FilePath $cmake -Arguments @(
    "--test-dir", (Join-Path $projectRoot "out\build\windows-msvc"),
    "-C", "Release", "--output-on-failure"
) -Description "Run automated tests"

$stage = Join-Path $projectRoot "dist"
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
Invoke-Checked -FilePath $cmake -Arguments @(
    "--install", (Join-Path $projectRoot "out\build\windows-msvc"),
    "--config", "Release", "--prefix", $stage
) -Description "Stage standalone application"

Write-Host ""
Write-Host "Build completed successfully." -ForegroundColor Green
Write-Host "Executable: $(Join-Path $stage 'FPVSignalTrainer.exe')"
Write-Host "Meta Quest Link / OpenXR runtime must be installed separately."
