$ErrorActionPreference = 'Continue'
$t = [Text.Encoding]::GetEncoding(949).GetString([IO.File]::ReadAllBytes('D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'))
$c = ($t -split "`r`n") | Where-Object { $_.Contains('ExecuteSpInt( szTemp, pParams, ') }
Write-Host ('3-arg ExecuteSpInt total: ' + $c.Count)
$c | ForEach-Object { Write-Host ('  ' + $_.Trim()) }
$s = ($t -split "`r`n") | Where-Object { $_.Contains('StringCchCopy( szTemp, ') -and $_.Contains('{call') }
Write-Host ('parameterized {call via StringCchCopy total: ' + $s.Count)
