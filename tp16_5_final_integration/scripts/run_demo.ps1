param(
    [string]$BenchPort = "COM5",
    [int]$Duration = 90
)

$Root = Split-Path -Parent $PSScriptRoot
& python "$Root\host\demo_supervisor.py" --port $BenchPort --duration $Duration --require-cycle
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
