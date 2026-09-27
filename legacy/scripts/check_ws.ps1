$ErrorActionPreference = 'Continue'
$f = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$t = [Text.Encoding]::GetEncoding(949).GetString([IO.File]::ReadAllBytes($f))
$lines = $t -split "`r`n"

function Show([int[]]$nums) {
  foreach ($n in $nums) {
    $ln = $lines[$n - 1]
    $vis = $ln -replace "`t", '<TAB>'
    Write-Host ("{0}: [{1}]" -f $n, $vis)
  }
}
Write-Host '=== UserCheck region ==='
Show @(350, 354, 355, 356, 357, 358, 362, 366, 367, 368, 382, 383, 389, 390, 391)
Write-Host '=== GsUserCheck region ==='
Show @(960, 965, 966, 967, 968, 969, 973, 976, 977, 981, 982, 983)
