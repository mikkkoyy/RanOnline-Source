$ErrorActionPreference = 'Continue'
$f   = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$bak = Join-Path $env:TEMP 'phase3a_original\s_COdbcUserCheck.cpp'
$cp949 = [Text.Encoding]::GetEncoding(949)
$t = $cp949.GetString([IO.File]::ReadAllBytes($f))
$lines = $t -split "`r`n"

Write-Host '=== PHASE 3 STEP 1 POST-CONDITIONS ==='
Write-Host ("1) U+FFFD count                      : {0}  (expect 0)" -f ([regex]::Matches($t, [char]0xFFFD)).Count)

$o = [IO.File]::ReadAllBytes($bak)
$n = [IO.File]::ReadAllBytes($f)
$ho = (($o | Where-Object { $_ -gt 127 }) -join ',')
$hn = (($n | Where-Object { $_ -gt 127 }) -join ',')
Write-Host ("2) CP949 high-byte sequence identical: {0}  ({1} -> {2} total bytes, {3} high bytes)" -f ($ho -eq $hn), $o.Length, $n.Length, ($n | Where-Object { $_ -gt 127 }).Count)

Write-Host ("3) 3-arg ExecuteSpInt calls          : {0}  (expect 11 = 8 batch + 1 pilot + 2 new)" -f ($lines | Where-Object { $_ -match 'ExecuteSpInt\( szTemp, pParams, ' }).Count)
Write-Host ("   parameterized {call via StringCchCopy : {0}  (expect 11)" -f ($lines | Where-Object { $_ -match 'StringCchCopy\( szTemp, \d+, "\{call' }).Count)
Write-Host ("   single-arg ExecuteSpInt(szTemp)  : {0}  (expect 1 = dead /* */ block only)" -f ($lines | Where-Object { $_ -match 'ExecuteSpInt\(szTemp\)' }).Count)

$i = 0; $live = 0
foreach ($ln in $lines) {
  $i++
  if ($ln -match '^\t*_snprintf\(') { Write-Host ("   live _snprintf @ line {0}" -f $i); $live++ }
}
Write-Host ("4) live _snprintf lines              : {0}  (expect 1 = dead /* */ block @ ~line 43)" -f $live)
Write-Host ("   Replace quote-doubling remaining  : {0}  (expect 0)" -f ($lines | Where-Object { $_.Contains('.Replace(_T(') } | Measure-Object).Count)
Write-Host ("   user_verify parameterized         : {0}  (expect 1)" -f ($lines | Where-Object { $_.Contains('{call user_verify(?,?,?,?,?,?,?,?)}') }).Count)
Write-Host ("   gs_user_verify parameterized      : {0}  (expect 1)" -f ($lines | Where-Object { $_.Contains('{call gs_user_verify(?,?,?,?,?,?)}') }).Count)
Write-Host ("   thai_user_verify parameterized    : {0}  (expect 1)" -f ($lines | Where-Object { $_.Contains('{call thai_user_verify(?,?,?,?,?,?)}') }).Count)
