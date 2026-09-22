$ErrorActionPreference = 'Continue'
$root = 'D:\FILES\project\modernization RanOnline'
$csv  = 'D:\FILES\project\modernization RanOnline\sql_audit_baseline.csv'
$cp949 = [Text.Encoding]::GetEncoding(949)
$skipDirs = '\\_Build\\|\\_Bin\\|\\.git\\|\\Debug\\|\\Release\\|\\x64\\|\\Win32\\'
$exts = @('.cpp','.h','.hpp','.c','.cxx','.hxx')

$printfRe  = [regex]'(?i)(_snprintf|sprintf_s|_sntprintf|_stprintf|sprintf|StringCchPrintf)\s*\('
$formatRe  = [regex]'(?i)\.Format\s*\('
$strRe     = [regex]'"[^"\r\n]*"'
$sqlKwRe   = [regex]'(?i)\b(select|insert|update|delete|exec|call)\b'
$interpRe  = [regex]'(?i)%[sS]|%l[si]|%hs'
$streamRe  = [regex]'<<\s*\"'
$concatExecRe = [regex]'(?i)ExecuteSQL?\w*\s*\('

$files = Get-ChildItem -Path $root -Recurse -File | Where-Object {
    ($exts -contains $_.Extension.ToLower()) -and
    ($_.FullName -notmatch $skipDirs) -and
    ($_.Length -lt 2MB)
}
Write-Host ("Files scanned: {0}" -f $files.Count)

$rows = New-Object System.Collections.Generic.List[object]

function Test-SqlLine([string]$code, [string]$file, [int]$ln, [string]$cls) {
    if ([string]::IsNullOrWhiteSpace($code)) { return }
    $family = $null
    if ($printfRe.IsMatch($code)) { $family = 'printf' }
    elseif ($formatRe.IsMatch($code)) { $family = 'Format' }
    elseif (($streamRe.IsMatch($code)) -or ($concatExecRe.IsMatch($code) -and $code.Contains('+'))) { $family = 'stream/concat' }
    if (-not $family) { return }

    # SQL keywords must appear inside a string literal of this code region
    $kw = $false; $interp = $false; $kwText = ''
    foreach ($m in $strRe.Matches($code)) {
        $s = $m.Value
        if ($sqlKwRe.IsMatch($s)) {
            $kw = $true
            if ($kwText -eq '') { $kwText = ($s.Length -gt 60) ? $s.Substring(0,60) : $s }
        }
        if ($interpRe.IsMatch($s)) { $interp = $true }
    }
    if (-not $kw) { return }

    $cls2 = $cls
    if ($cls2 -eq 'LIVE') { $cls2 = $interp ? 'VULNERABLE' : 'SAFE-numeric-or-literal' }
    $snip = $code.Trim()
    if ($snip.Length -gt 140) { $snip = $snip.Substring(0,140) }
    # sanitize non-ascii (CP949 comments) for stable CSV/console
    $sb = New-Object System.Text.StringBuilder
    foreach ($ch in $snip.ToCharArray()) { [void]$sb.Append( ([int]$ch -lt 128) ? $ch : '?' ) }
    $rows.Add([pscustomobject]@{
        Class = $cls2; Family = $family; Interp = $interp
        File = $file; Line = $ln; Snippet = $sb.ToString()
    })
}

foreach ($f in $files) {
    $t = $cp949.GetString([IO.File]::ReadAllBytes($f.FullName))
    $t = $t -replace "`r`n", "`n"; $t = $t -replace "`r", "`n"
    $lines = $t -split "`n"
    $rel = $f.FullName.Substring($root.Length + 1)
    $inBlock = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]; $ln = $i + 1
        $work = $line; $dead = $false
        if ($inBlock) {
            $idx = $work.IndexOf('*/')
            if ($idx -lt 0) { $dead = $true }
            else { $work = $work.Substring($idx + 2); $inBlock = $false }
        }
        if ($dead) {
            Test-SqlLine $line $rel $ln 'DEAD'
            continue
        }
        # strip /* */ pairs; track unterminated opens
        while ($true) {
            $o = $work.IndexOf('/*')
            if ($o -lt 0) { break }
            $c = $work.IndexOf('*/', $o + 2)
            if ($c -lt 0) { $inBlock = $true; $work = $work.Substring(0, $o); break }
            $work = $work.Remove($o, $c - $o + 2)
        }
        $sl = $work.IndexOf('//')
        if ($sl -ge 0) {
            $commentPart = $work.Substring($sl)
            $codePart    = $work.Substring(0, $sl)
        } else {
            $commentPart = ''
            $codePart    = $work
        }
        Test-SqlLine $codePart $rel $ln 'LIVE'
        Test-SqlLine $commentPart $rel $ln 'DEAD'
    }
}

$rows | Export-Csv -Path $csv -NoTypeInformation -Encoding UTF8

Write-Host ''
Write-Host '=== AUDIT SUMMARY ==='
$rows | Group-Object Class | ForEach-Object { Write-Host ("{0,-24} : {1}" -f $_.Name, $_.Count) }
Write-Host ''
Write-Host '=== VULNERABLE (needs conversion) ==='
$vuln = $rows | Where-Object { $_.Class -eq 'VULNERABLE' }
Write-Host ("count: {0}" -f $vuln.Count)
$vuln | ForEach-Object { Write-Host ("{0}:{1} [{2}] {3}" -f $_.File, $_.Line, $_.Family, $_.Snippet) }
Write-Host ''
Write-Host '=== per-file aggregate (DEAD / SAFE) ==='
$rows | Where-Object { $_.Class -ne 'VULNERABLE' } |
    Group-Object File | ForEach-Object {
        $d = ($_.Group | Where-Object { $_.Class -eq 'DEAD' }).Count
        $s = ($_.Group | Where-Object { $_.Class -eq 'SAFE-numeric-or-literal' }).Count
        Write-Host ("{0}  DEAD={1} SAFE={2}" -f $_.Name, $d, $s)
    }
