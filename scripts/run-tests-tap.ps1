param(
    [string]$BinDir = "e:\AvatarQuest\AvatarQuest\bin\Debug",
    [string]$OutDir = "e:\AvatarQuest\AvatarQuest\build\test_logs"
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

$tests = @(
    'aq_tests_weapon',
    'aq_tests_armor',
    'aq_tests_skills',
    'aq_tests_combat',
    'aq_tests_equipment',
    'aq_tests_leveling',
    'aq_tests_naming',
    'aq_tests_misc'
)

foreach ($t in $tests) {
    $exe = Join-Path $BinDir ("{0}.exe" -f $t)
    if (-not (Test-Path -LiteralPath $exe)) {
        Write-Error "Missing test exe: $exe"
        exit 3
    }
    $out = Join-Path $OutDir ("{0}.tap.txt" -f ($t -replace '^aq_tests_',''))
    Write-Host "Running $t -> $out"
    & $exe -r tap -s | Out-File -FilePath $out -Encoding UTF8
}

Write-Host "TAP logs written to: $OutDir"