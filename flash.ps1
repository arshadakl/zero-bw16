# Zero-BW16 Flash Script
# Usage: Right-click -> Run with PowerShell  OR  .\flash.ps1

$BOARD_URL = "https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek.com_amebad_index.json"
$SKETCH = $PSScriptRoot

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  Zero-BW16 Flash Tool" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Check arduino-cli
$acli = Get-Command arduino-cli -ErrorAction SilentlyContinue
if (-not $acli) {
    Write-Host "[*] arduino-cli not found. Installing..." -ForegroundColor Yellow
    $installer = "$env:TEMP\arduino-cli-install.ps1"
    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh" -OutFile "$env:TEMP\install.sh"
    # Windows: download binary directly
    $arch = if ([System.Environment]::Is64BitOperatingSystem) { "Windows_64bit" } else { "Windows_32bit" }
    $release = Invoke-RestMethod "https://api.github.com/repos/arduino/arduino-cli/releases/latest"
    $asset = $release.assets | Where-Object { $_.name -like "*$arch*.zip" } | Select-Object -First 1
    $zipPath = "$env:TEMP\arduino-cli.zip"
    Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zipPath
    Expand-Archive -Path $zipPath -DestinationPath "$env:LOCALAPPDATA\arduino-cli" -Force
    $env:PATH += ";$env:LOCALAPPDATA\arduino-cli"
    Write-Host "[+] arduino-cli installed." -ForegroundColor Green
}

# Install board package if not present
Write-Host "[*] Checking Realtek AmebaD board package..." -ForegroundColor Yellow
$boards = arduino-cli board listall 2>$null | Select-String "AmebaD"
if (-not $boards) {
    Write-Host "[*] Installing Realtek AmebaD board package (includes BW16/RTL8720DN)..." -ForegroundColor Yellow
    arduino-cli config init --overwrite | Out-Null
    arduino-cli config add board_manager.additional_urls $BOARD_URL | Out-Null
    arduino-cli core update-index | Out-Null
    arduino-cli core install realtek:AmebaD
    Write-Host "[+] Board package installed." -ForegroundColor Green
} else {
    Write-Host "[+] Board package already installed." -ForegroundColor Green
}

# Auto-detect BW16 FQBN
$FQBN = (arduino-cli board listall 2>$null | Select-String -Pattern "bw16" -CaseSensitive:$false | Select-Object -First 1).ToString().Split()[-1]
if (-not $FQBN) {
    Write-Host "[!] Could not detect BW16 board FQBN. Package may not include BW16." -ForegroundColor Red
    arduino-cli board listall | Select-String "realtek"
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host "[+] Detected FQBN: $FQBN" -ForegroundColor Green

# Detect connected COM ports (use registry — more reliable than WMI for USB serial)
Write-Host ""
Write-Host "[*] Scanning for connected serial ports..." -ForegroundColor Yellow

$comPorts = @()
# Method 1: Registry (catches CH340/CP210x/etc reliably)
$regPath = "HKLM:\HARDWARE\DEVICEMAP\SERIALCOMM"
if (Test-Path $regPath) {
    Get-ItemProperty $regPath | Get-Member -MemberType NoteProperty |
        Where-Object { $_.Name -notlike "PS*" } |
        ForEach-Object { $comPorts += (Get-ItemProperty $regPath).$($_.Name) }
}
# Method 2: .NET fallback
if ($comPorts.Count -eq 0) {
    $comPorts = [System.IO.Ports.SerialPort]::GetPortNames()
}

if ($comPorts.Count -eq 0) {
    Write-Host "[!] No COM ports detected. Connect your BW16 module." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "Available COM ports:" -ForegroundColor Cyan
$comPorts | ForEach-Object { Write-Host "  $_" }

Write-Host ""
$port = Read-Host "Enter COM port (e.g. COM3)"

if (-not $port) {
    Write-Host "[!] No port entered. Exiting." -ForegroundColor Red
    exit 1
}

# Remind user about download mode
Write-Host ""
Write-Host "======================================" -ForegroundColor Yellow
Write-Host "  ENTER DOWNLOAD MODE NOW:" -ForegroundColor Yellow
Write-Host "  1. Hold BOOT button" -ForegroundColor Yellow
Write-Host "  2. Press and release RESET" -ForegroundColor Yellow
Write-Host "  3. Release BOOT" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Yellow
Write-Host ""
$confirm = Read-Host "Module in download mode? Press Enter to flash or type 'n' to cancel"
if ($confirm -eq 'n') { exit 0 }

# Compile
Write-Host ""
Write-Host "[*] Compiling firmware..." -ForegroundColor Yellow
$buildDir = "$env:TEMP\zero-bw16-build"
arduino-cli compile --fqbn $FQBN --build-path $buildDir --warnings none $SKETCH
if ($LASTEXITCODE -ne 0) {
    Write-Host "[!] Compilation failed." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host "[+] Compilation successful." -ForegroundColor Green

# Flash
Write-Host ""
Write-Host "[*] Flashing to $port ..." -ForegroundColor Yellow
arduino-cli upload --fqbn $FQBN --port $port --input-dir $buildDir $SKETCH
if ($LASTEXITCODE -ne 0) {
    Write-Host "[!] Flash failed. Check port and download mode." -ForegroundColor Red
} else {
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Green
    Write-Host "  Flash complete!" -ForegroundColor Green
    Write-Host "  Connect to WiFi: zero-bw16" -ForegroundColor Green
    Write-Host "  Password:        zerobw16!" -ForegroundColor Green
    Write-Host "  Web UI:          http://192.168.1.1" -ForegroundColor Green
    Write-Host "======================================" -ForegroundColor Green
}

Write-Host ""
Read-Host "Press Enter to exit"
