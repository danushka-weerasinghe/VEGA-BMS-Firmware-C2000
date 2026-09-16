<# :
@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-Command -ScriptBlock ([scriptblock]::Create([System.IO.File]::ReadAllText('%~f0'))) -ArgumentList \"%~1\""
exit /b %errorlevel%
#>

param(
    [string]$InputFile,
    [int]$Divisor = 12,
    [int]$NumData = 32
)

if (-not $InputFile) {
    Write-Host "Error: Please provide an input file."
    exit 1
}

if (-not (Test-Path $InputFile)) {
    Write-Host "Error: File '$InputFile' not found."
    exit 1
}

# Get absolute path to prevent directory from being empty
$InputFile = (Get-Item $InputFile).FullName

# Read entire content as single string
$data = [IO.File]::ReadAllText($InputFile)

# Equivalent to Python's data[1:-3]
if ($data.Length -gt 4) {
    $data = $data.Substring(1, $data.Length - 4)
}

# Replace spaces with commas and remove newlines
$data = $data -replace " ", "," -replace "`n", "" -replace "`r", ""

# Split by comma (ignoring empty entries if any)
$hexs = $data.Split(",", [System.StringSplitOptions]::RemoveEmptyEntries)

# Padding
$rem = $hexs.Count % $Divisor
if ($rem -ne 0) {
    $padCount = $Divisor - $rem
    $padding = @("00") * $padCount
    $hexs = $hexs + $padding
}

# Add 0x
$nums = $hexs | ForEach-Object { "0x" + $_ }

# Group and join
$lines = @()
for ($i = 0; $i -lt $nums.Count; $i += $NumData) {
    $chunkSize = [Math]::Min($NumData, $nums.Count - $i)
    $chunk = $nums[$i..($i + $chunkSize - 1)]
    $lines += ($chunk -join ",")
}
$newData = $lines -join ",`n"

# Paths
$directory = [System.IO.Path]::GetDirectoryName($InputFile)
$baseName = [System.IO.Path]::GetFileNameWithoutExtension($InputFile)
$outputDir = Join-Path $directory "OBD fimware"
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$timestamp = Get-Date -Format "yyyy_MM_dd_HH_mm_ss"
$outputFilename = "${baseName}_${timestamp}.txt"
$outputPath = Join-Path $outputDir $outputFilename

Write-Host "Processing: $InputFile"
Write-Host "Saving to:  $outputPath"

[IO.File]::WriteAllText($outputPath, $newData)
