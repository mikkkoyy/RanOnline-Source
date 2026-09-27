$ErrorActionPreference = 'Stop'
$f = 'D:\FILES\project\modernization RanOnline\audit_sql2.ps1'
$bs = [char]92
$p = '(<<|' + $bs + '+=|' + $bs + '.append' + $bs + 's*' + $bs + '(' + '|' + $bs + '+' + $bs + 's*[A-Za-z_(L"])|' + $bs + 'bstr(?:cat|cpy)(?:_s)?' + $bs + 's*' + $bs + '('
$lines = [IO.File]::ReadAllLines($f)
$fixed = $false
for ($i = 0; $i -lt $lines.Count; $i++) {
  if ($lines[$i].StartsWith('$concatRe')) {
    $lines[$i] = '$concatRe = [regex]''' + $p + ''''
    $fixed = $true
    Write-Host ("fixed line {0}: {1}" -f ($i + 1), $lines[$i])
  }
}
if (-not $fixed) { throw 'concatRe line not found' }
[IO.File]::WriteAllLines($f, $lines)
# sanity: the new pattern must compile
$r = [regex]$p
Write-Host ('pattern compiles OK; test: ' + $r.IsMatch('a << b'))
