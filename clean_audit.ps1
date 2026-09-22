$ErrorActionPreference = 'Continue'
$all = Import-Csv 'D:\FILES\project\modernization RanOnline\sql_audit_partC.csv'
$clean = $all | Where-Object { $_.File -notmatch 'batch_preview' }
Write-Host ("rows total={0}  clean={1}  excluded(artifact)={2}" -f $all.Count, $clean.Count, ($all.Count - $clean.Count))

Write-Host ''
Write-Host '=== CLEANED TOTALS BY CLASS ==='
$clean | Group-Object Class | Sort-Object Name | ForEach-Object { Write-Host ("{0,-16}: {1}" -f $_.Name, $_.Count) }

$out = 'D:\FILES\project\modernization RanOnline\audit_vulnerable_list.txt'
$sb = New-Object System.Text.StringBuilder
[void]$sb.AppendLine('=== VULNERABLE (cleaned) ===')
foreach ($r in ($clean | Where-Object { $_.Class -eq 'VULNERABLE' })) {
  [void]$sb.AppendLine(("{0}:{1} [{2}] {3} | {4} | {5}" -f $r.File, $r.Line, $r.Family, $r.Priority, $r.Details, $r.Snippet))
}
[void]$sb.AppendLine('')
[void]$sb.AppendLine('=== ESCAPED (cleaned) ===')
foreach ($r in ($clean | Where-Object { $_.Class -eq 'ESCAPED' })) {
  [void]$sb.AppendLine(("{0}:{1} [{2}] {3} | {4}" -f $r.File, $r.Line, $r.Family, $r.Details, $r.Snippet))
}
[void]$sb.AppendLine('')
[void]$sb.AppendLine('=== VULNERABLE counts by file ===')
foreach ($g in ($clean | Where-Object { $_.Class -eq 'VULNERABLE' } | Group-Object File | Sort-Object Count -Descending)) {
  [void]$sb.AppendLine(("{0}  :  {1}" -f $g.Name, $g.Count))
}
[void]$sb.AppendLine('')
[void]$sb.AppendLine('=== EXCLUDED (tooling artifact, not source) ===')
foreach ($g in ($all | Where-Object { $_.File -match 'batch_preview' } | Group-Object Class)) {
  [void]$sb.AppendLine(("{0}  :  {1}" -f $g.Name, $g.Count))
}
[IO.File]::WriteAllText($out, $sb.ToString(), [Text.UTF8Encoding]::new($false))
Write-Host ("written: {0}" -f $out)
