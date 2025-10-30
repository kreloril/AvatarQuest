param(
    [string]$LogDir = "e:\AvatarQuest\AvatarQuest\build\test_logs",
    [string]$OutFile = "e:\AvatarQuest\AvatarQuest\build\test_logs\all.tap.txt"
)

if (-not (Test-Path -LiteralPath $LogDir)) {
    Write-Error "Log directory not found: $LogDir"
    exit 1
}

$files = Get-ChildItem -Path $LogDir -Filter '*.tap.txt' | Sort-Object Name
if (-not $files) {
    Write-Error "No TAP logs found in $LogDir"
    exit 2
}

$newline = "`r`n"
"# Aggregated TAP logs generated on $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" | Out-File -FilePath $OutFile -Encoding UTF8

foreach ($f in $files) {
    $content = Get-Content -LiteralPath $f.FullName -Raw -Encoding UTF8
    $okCount = ([regex]::Matches($content, "(?m)^(ok|not ok) \d+")).Count
    $failCount = ([regex]::Matches($content, "(?m)^not ok ")).Count
    $status = if ($failCount -gt 0) { "FAIL" } else { "PASS" }

    "" | Out-File -FilePath $OutFile -Append -Encoding UTF8
    ("## BEGIN SUITE: {0} [{1}] (ok/not ok lines: {2}, failures: {3})" -f $f.Name, $status, $okCount, $failCount) | Out-File -FilePath $OutFile -Append -Encoding UTF8
    "" | Out-File -FilePath $OutFile -Append -Encoding UTF8

    # Write content
    $content | Out-File -FilePath $OutFile -Append -Encoding UTF8

    "" | Out-File -FilePath $OutFile -Append -Encoding UTF8
    ("## END SUITE: {0}" -f $f.Name) | Out-File -FilePath $OutFile -Append -Encoding UTF8
}

Write-Host "Aggregated logs written to: $OutFile"