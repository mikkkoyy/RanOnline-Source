$ErrorActionPreference = 'Continue'
$tmp = $env:TEMP
$tags = @('net','gmtool','client','engine','servers','editors','misc')
$all = @()
foreach ($t in $tags) {
  $p = Join-Path $tmp ("sqlaudit_{0}.csv" -f $t)
  if (Test-Path $p) {
    $c = Import-Csv $p
    Write-Host ("{0,-8}: {1} rows" -f $t, $c.Count)
    $all += $c
  }
}
Write-Host ("TOTAL merged rows: {0}" -f $all.Count)
$all | Export-Csv 'D:\FILES\project\modernization RanOnline\sql_audit_partC.csv' -NoTypeInformation -Encoding UTF8

Write-Host ''
Write-Host '=== TOTALS BY CLASS ==='
$all | Group-Object Class | Sort-Object Name | ForEach-Object { Write-Host ("{0,-16}: {1}" -f $_.Name, $_.Count) }

Write-Host ''
Write-Host '=== VULNERABLE ROWS (all) ==='
$all | Where-Object { $_.Class -eq 'VULNERABLE' } | ForEach-Object {
  Write-Host ("{0}:{1} [{2}] {3} | {4} | {5}" -f $_.File, $_.Line, $_.Family, $_.Priority, $_.Details, $_.Snippet)
}

Write-Host ''
Write-Host '=== ESCAPED ROWS (all) ==='
$all | Where-Object { $_.Class -eq 'ESCAPED' } | ForEach-Object {
  Write-Host ("{0}:{1} [{2}] {3} | {4}" -f $_.File, $_.Line, $_.Family, $_.Details, $_.Snippet)
}

Write-Host ''
Write-Host '=== PARAMETERIZED breakdown by file/family ==='
$all | Where-Object { $_.Class -eq 'PARAMETERIZED' } | Group-Object File, Family | Sort-Object Count -Descending | ForEach-Object { Write-Host ("{0,-80} {1}" -f $_.Name, $_.Count) }

Write-Host ''
Write-Host '=== FALSE_POSITIVE breakdown by detail-reason ==='
$all | Where-Object { $_.Class -eq 'FALSE_POSITIVE' } | ForEach-Object {
  if ($_.Details -like 'numeric-only*') { 'numeric-only' }
  elseif ($_.Details -like 'literal-only*') { 'literal-only' }
  elseif ($_.Details -like 'log*') { 'log-context' } else { 'other' }
} | Group-Object | ForEach-Object { Write-Host ("{0,-16}: {1}" -f $_.Name, $_.Count) }

Write-Host ''
Write-Host '=== DEAD breakdown by file (top 15) ==='
$all | Where-Object { $_.Class -eq 'DEAD' } | Group-Object File | Sort-Object Count -Descending | Select-Object -First 15 | ForEach-Object { Write-Host ("{0,-80} {1}" -f $_.Name, $_.Count) }