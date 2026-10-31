# Read-only OpenXR runtime diagnostic for FPV Signal Trainer.
# This script does not edit the registry, environment variables, or Meta Quest Link settings.

$ErrorActionPreference = "Continue"

function Write-Section([string]$Title) {
    Write-Host ""
    Write-Host "=== $Title ===" -ForegroundColor Cyan
}

function Test-PathStatus([string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return "NOT SET"
    }
    if (Test-Path -LiteralPath $Path -PathType Leaf) {
        return "FOUND"
    }
    return "NOT FOUND"
}

Write-Host "FPV Signal Trainer - OpenXR diagnostic (read-only)" -ForegroundColor Green
Write-Host "Process architecture: $([IntPtr]::Size * 8)-bit"
Write-Host "PowerShell: $($PSVersionTable.PSVersion)"

Write-Section "XR_RUNTIME_JSON environment override"
$processOverride = [Environment]::GetEnvironmentVariable("XR_RUNTIME_JSON", "Process")
$userOverride = [Environment]::GetEnvironmentVariable("XR_RUNTIME_JSON", "User")
$machineOverride = [Environment]::GetEnvironmentVariable("XR_RUNTIME_JSON", "Machine")

Write-Host "Process : $(if ($processOverride) { $processOverride } else { '<not set>' })"
Write-Host "User    : $(if ($userOverride) { $userOverride } else { '<not set>' })"
Write-Host "Machine : $(if ($machineOverride) { $machineOverride } else { '<not set>' })"

Write-Section "OpenXR active runtime registry"
$registryPath = $null
try {
    $baseKey = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine,
        [Microsoft.Win32.RegistryView]::Registry64
    )
    $openXrKey = $baseKey.OpenSubKey("SOFTWARE\Khronos\OpenXR\1")
    if ($null -ne $openXrKey) {
        $registryPath = [string]$openXrKey.GetValue("ActiveRuntime", $null)
        $openXrKey.Dispose()
    }
    $baseKey.Dispose()
} catch {
    Write-Host "Registry read failed: $($_.Exception.Message)" -ForegroundColor Yellow
}

Write-Host "HKLM\SOFTWARE\Khronos\OpenXR\1\ActiveRuntime:"
Write-Host "  $(if ($registryPath) { $registryPath } else { '<not set>' })"

Write-Section "Effective runtime manifest"
$effectivePath = if ($processOverride) { $processOverride } else { $registryPath }
$source = if ($processOverride) { "XR_RUNTIME_JSON process override" } else { "HKLM ActiveRuntime registry value" }

Write-Host "Source: $source"
Write-Host "Path  : $(if ($effectivePath) { $effectivePath } else { '<no runtime path configured>' })"
Write-Host "File  : $(Test-PathStatus $effectivePath)"

if ($effectivePath -and (Test-Path -LiteralPath $effectivePath -PathType Leaf)) {
    try {
        $manifest = Get-Content -LiteralPath $effectivePath -Raw | ConvertFrom-Json
        Write-Host "Manifest JSON: valid"

        $runtimeName = $null
        $libraryPath = $null
        if ($manifest.runtime) {
            $runtimeName = $manifest.runtime.name
            $libraryPath = $manifest.runtime.library_path
        }

        Write-Host "Runtime name : $(if ($runtimeName) { $runtimeName } else { '<not specified>' })"
        Write-Host "Library path : $(if ($libraryPath) { $libraryPath } else { '<not specified>' })"

        if ($libraryPath) {
            if (-not [System.IO.Path]::IsPathRooted($libraryPath)) {
                $libraryPath = Join-Path (Split-Path -Parent $effectivePath) $libraryPath
            }
            Write-Host "Library file : $(Test-PathStatus $libraryPath)"
            Write-Host "Resolved DLL : $libraryPath"
        }
    } catch {
        Write-Host "Manifest JSON: INVALID - $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Section "Meta Quest Link processes"
$processNames = @("OculusClient", "OVRServer_x64", "OVRRedir")
foreach ($name in $processNames) {
    $found = Get-Process -Name $name -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "$name : RUNNING"
    } else {
        Write-Host "$name : not running"
    }
}

Write-Section "Next steps"
Write-Host "1. Start the Meta Quest Link desktop app."
Write-Host "2. Connect Quest 3 by USB and launch Quest Link inside the headset."
Write-Host "3. In Meta Quest Link > Settings > General, set Meta Quest Link as the active OpenXR runtime."
Write-Host "4. Close and reopen FPV Signal Trainer."
Write-Host "5. Share this complete diagnostic output if XR_ERROR_RUNTIME_FAILURE remains."
Write-Host ""
Write-Host "This script is diagnostic only. It does not modify Windows settings."
