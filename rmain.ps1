# Create logs directory if it does not exist
if (-not (Test-Path "logs")) { New-Item -ItemType Directory -Path "logs" }

# Generate timestamp
$timestamp = Get-Date -f "yyyyMMdd_HHmmss"
$logFile = "logs\backtest_$timestamp.txt"

# Execute and redirect output
.\src\main.exe > $logFile
Write-Host "Backtest completed. Report saved to: $logFile"