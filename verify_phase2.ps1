# Phase 2 close-out : post-condition assertions for the 8-site batch.
# Reuses the patch table from gen_batch_diff.ps1 (dot-sourced for data only;
# its own run FAILs by design because the old anchors no longer exist).
$ErrorActionPreference = 'Continue'
. 'D:\FILES\project\modernization RanOnline\gen_batch_diff.ps1'

$cur = $text   # current patched file, decoded CP949 (set by dot-sourced script)

Write-Host '=== POST-CONDITIONS ==='
Write-Host ("1) U+FFFD count                    : {0}  (expect 0)" -f ([regex]::Matches($cur, [char]0xFFFD)).Count)

# Reconstruct the pre-batch original by inverting the 8 replacements.
$orig2 = $cur
$invOk = $true
foreach ($p in $patches) {
  $oldStr = J $p.old
  $newStr = J $p.new
  $cnt = ([regex]::Matches($orig2, [regex]::Escape($newStr))).Count
  if ($cnt -ne 1) { Write-Host ("   INVERT FAIL [{0}] new-block count {1}" -f $p.name, $cnt); $invOk = $false; break }
  $orig2 = $orig2.Replace($newStr, $oldStr)
}
$hiCur = (($cp949.GetBytes($cur) | Where-Object { $_ -gt 127 }) -join ',')
$hiOrg = (($cp949.GetBytes($orig2) | Where-Object { $_ -gt 127 }) -join ',')
Write-Host ("2) inverse-patch reconstruction     : {0}" -f $(if ($invOk) { 'OK (8/8 inverted)' } else { 'FAILED' }))
Write-Host ("   Korean CP949 high-byte identical: {0}  ({1} bytes)" -f ($hiCur -eq $hiOrg), $hiCur.Split(',').Count)

$lines = $cur -split "`r`n"
Write-Host ("3) 3-arg ExecuteSpInt calls         : {0}  (expect 9 = 8 batch + 1 ThaiUserCheck pilot)" -f ($lines | Where-Object { $_ -match 'ExecuteSpInt\( szTemp, pParams, ' }).Count)
Write-Host ("   parameterized {{call statements   : {0}  (expect 9, via StringCchCopy)" -f ($lines | Where-Object { $_ -match 'StringCchCopy\( szTemp, \d+, "\{call' }).Count)
Write-Host ("   single-arg ExecuteSpInt(szTemp)  : {0}  (expect 2 = UserCheck + GsUserCheck)" -f ($lines | Where-Object { $_ -match 'ExecuteSpInt\(szTemp\)' }).Count)

$i = 0; $live = 0
foreach ($ln in $lines) {
  $i++
  if ($ln -match '^\t*_snprintf\(') { Write-Host ("   live _snprintf @ line {0}" -f $i); $live++ }
}
Write-Host ("4) live _snprintf lines             : {0}  (expect 3 = 1 dead /* */ block ~line 43 + UserCheck + GsUserCheck)" -f $live)
Write-Host ("   commented legacy refs (//_snprintf): {0}  (expect 9 = 8 batch + 1 pilot)" -f ($lines | Where-Object { $_ -match '^\t*//_snprintf\(' }).Count)
Write-Host ("   thai_user_verify parameterized    : {0}  (expect 1)" -f ($lines | Where-Object { $_ -match '\{call thai_user_verify\(?,?,?,?,?,\?\)\}' }).Count)
