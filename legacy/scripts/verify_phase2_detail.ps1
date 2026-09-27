$ErrorActionPreference = 'Continue'
$f = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$t = [Text.Encoding]::GetEncoding(949).GetString([IO.File]::ReadAllBytes($f))
$lines = $t -split "`r`n"

Write-Host '--- thai_user_verify parameterized line ---'
$i = 0
foreach ($ln in $lines) { $i++; if ($ln.Contains('{call thai_user_verify')) { Write-Host ("{0} : {1}" -f $i, $ln.Trim()) } }

Write-Host '--- single-arg ExecuteSpInt(szTemp) occurrences ---'
$i = 0
foreach ($ln in $lines) { $i++; if ($ln -match 'ExecuteSpInt\(szTemp\)') { Write-Host ("{0} : {1}" -f $i, $ln.Trim()) } }

Write-Host '--- commented //_snprintf lines (categorize) ---'
$i = 0
foreach ($ln in $lines) {
  $i++
  if ($ln -match '^\t*//_snprintf\(') {
    $s = $ln.Trim()
    if ($s.Length -gt 120) { $s = $s.Substring(0, 120) }
    Write-Host ("{0} : {1}" -f $i, $s)
  }
}
