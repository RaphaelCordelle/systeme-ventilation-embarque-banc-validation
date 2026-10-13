param(
    [string]$DutPort = "COM7",
    [string]$BenchPort = "COM5"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    if (-not $env:IDF_PATH) {
        throw "ESP-IDF absent du PATH et variable IDF_PATH non définie."
    }

    $ExportScript = Join-Path $env:IDF_PATH "export.ps1"
    if (-not (Test-Path -LiteralPath $ExportScript)) {
        throw "Script ESP-IDF introuvable : $ExportScript"
    }
    . $ExportScript
}

function Invoke-Idf {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Label,

        [Parameter(Mandatory = $true)]
        [string[]]$IdfArguments
    )

    $IdfCommand = Get-Command idf.py -ErrorAction Stop
    if ($IdfCommand.Path -and $IdfCommand.Path.EndsWith(".py", [StringComparison]::OrdinalIgnoreCase)) {
        $PythonCommand = Get-Command python -ErrorAction Stop
        & $PythonCommand.Path $IdfCommand.Path @IdfArguments
    }
    else {
        & $IdfCommand @IdfArguments
    }

    if ($LASTEXITCODE -ne 0) {
        throw "$Label a échoué avec le code $LASTEXITCODE."
    }
}

Write-Host "=== Flash DUT on $DutPort ==="
Push-Location "$Root\dut_final"
try {
    Invoke-Idf -Label "Le flash DUT" -IdfArguments @("-p", $DutPort, "flash")
}
finally {
    Pop-Location
}

Write-Host "=== Flash BENCH on $BenchPort ==="
Push-Location "$Root\bench_final"
try {
    Invoke-Idf -Label "Le flash BENCH" -IdfArguments @("-p", $BenchPort, "flash")
}
finally {
    Pop-Location
}

Write-Host "Flash complete."
