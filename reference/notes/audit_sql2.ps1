param([string[]]$Dirs = @(), [string]$Tag = 'chunk')
$ErrorActionPreference = 'Continue'
$root = 'D:\FILES\project\modernization RanOnline'
$csv  = Join-Path $env:TEMP ("sqlaudit_{0}.csv" -f $Tag)
$cp   = [Text.Encoding]::GetEncoding(949)
$skip = '\\_Build\\|\\_Bin\\|\\.git\\|\\.vs\\|\\Debug\\|\\Release\\|\\x64\\|\\Win32\\'
$exts = @('.cpp','.h','.hpp','.c','.cxx','.hxx','.inl')

$printfRe = [regex]'(?i)\b(_snwprintf|_snprintf|sprintf_s|_sntprintf|_stprintf|sprintf|StringCchPrintf)\s*\('
$formatRe = [regex]'(?i)\.Format\s*\('
$concatRe = [regex]'(<<|\+=|\.append\s*\(|\+\s*[A-Za-z_(L"])|\bstr(?:cat|cpy)(?:_s)?\s*\('
$strRe    = [regex]'"[^"\r\n]*"'
$kwRe     = [regex]'(?i)\b(select|insert|update|delete|exec|execute|from|where|into)\b|\{call'
$intSRe   = [regex]'(?i)%[sS]|%hs|%ls'
$numRe    = [regex]'(?i)%[dDiIuUxX]|%I64[dui]|%[fGeEgG]'
$execRe   = [regex]'(?i)\b(ExecuteSpInt|ExecuteSQL|ExecuteSp|SQLExecDirect)\s*\('

$rows = New-Object System.Collections.Generic.List[object]

function Add-Row([string]$cls,[string]$fam,[string]$file,[int]$ln,[string]$prio,[string]$det,[string]$snip) {
  if ($snip.Length -gt 120) { $snip = $snip.Substring(0,120) }
  $sb = New-Object System.Text.StringBuilder
  foreach ($ch in $snip.ToCharArray()) { [void]$sb.Append((([int]$ch -lt 128) ? $ch : '?')) }
  $rows.Add([pscustomobject]@{ Class=$cls; Family=$fam; File=$file; Line=$ln; Priority=$prio; Details=$det; Snippet=$sb.ToString() })
}
# MARKER_PASS2
foreach ($d in $Dirs) {
  $dir = Join-Path $root $d
  if (-not (Test-Path $dir)) { Write-Host ("SKIP missing dir: {0}" -f $d); continue }
  $files = Get-ChildItem -Path $dir -Recurse -File | Where-Object {
    ($exts -contains $_.Extension.ToLower()) -and ($_.FullName -notmatch $skip) -and ($_.Length -lt 2MB)
  }
  foreach ($f in $files) {
    try { $t = $cp.GetString([IO.File]::ReadAllBytes($f.FullName)) } catch { Write-Host ("READ-FAIL: {0}" -f $f.Name); continue }
    $t = $t -replace "`r`n", "`n"
    $lines = $t -split "`n"
    $rel = $f.FullName.Substring($root.Length + 1)
    # pass 1: live-code map + escape/exec sets + comment state
    $codeMap = @{}; $escSet = @{}; $execSet = @{}
    $inBlock = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
      $work = $lines[$i]; $dead = $false
      if ($inBlock) {
        $ix = $work.IndexOf('*/')
        if ($ix -lt 0) { $dead = $true } else { $work = $work.Substring($ix + 2); $inBlock = $false }
      }
      if (-not $dead) {
        while ($true) {
          $o = $work.IndexOf('/*'); if ($o -lt 0) { break }
          $c = $work.IndexOf('*/', $o + 2)
          if ($c -lt 0) { $inBlock = $true; $work = $work.Substring(0, $o); break }
          $work = $work.Remove($o, $c - $o + 2)
        }
        $sl = $work.IndexOf('//')
        if ($sl -ge 0) { $work = $work.Substring(0, $sl) }
        if ($work.Contains('Replace(_T(') -and $work.Contains("''")) { $escSet[$i] = $true }
        if ($execRe.IsMatch($work)) { $execSet[$i] = $true }
        if (-not [string]::IsNullOrWhiteSpace($work)) { $codeMap[$i] = $work }
      }
    }
# MARKER_PASS3
    # pass 2: classify live code lines
    foreach ($i in ($codeMap.Keys | Sort-Object)) {
      $code = $codeMap[$i]; $ln = $i + 1
      $fam = ''
      if ($printfRe.IsMatch($code)) { $fam = 'printf' }
      elseif ($formatRe.IsMatch($code)) { $fam = 'Format' }
      elseif ($concatRe.IsMatch($code)) { $fam = 'concat/stream' }
      if ($fam -eq '' -and $code.Contains('ExecuteSpInt') -and $code.Contains('pParams')) { $fam = 'bind-call-3arg' }
      elseif ($fam -eq '' -and $code.Contains('SQLBindParameter')) { $fam = 'SQLBindParameter' }
      if ($fam -eq '') { continue }

      $kw = $false; $kwTxt = ''; $hasPctS = $false; $hasNum = $false; $hasQ = $false; $logApi = $false
      foreach ($m in $strRe.Matches($code)) {
        $s = $m.Value
        if ($kwRe.IsMatch($s)) { $kw = $true; if ($kwTxt -eq '') { $kwTxt = $s.Trim('"'); if ($kwTxt.Length -gt 50) { $kwTxt = $kwTxt.Substring(0,50) } } }
        if ($s.Contains('?')) { $hasQ = $true }
        if ($intSRe.IsMatch($s)) { $hasPctS = $true }
        if ($numRe.IsMatch($s)) { $hasNum = $true }
      }
      if ($code.Contains('CConsoleMessage') -or $code.Contains('GetInstance()->Write')) { $logApi = $true }

      if ($fam -eq 'bind-call-3arg' -or $fam -eq 'SQLBindParameter') {
        Add-Row 'PARAMETERIZED' $fam $rel $ln '-' 'bound-parameter mechanism' $code.Trim()
        continue
      }
      if (-not $kw) { continue }
      if ($logApi) { Add-Row 'FALSE_POSITIVE' $fam $rel $ln '-' 'log/trace message context' $code.Trim(); continue }

      $chainVar = $false
      if ($code -match '(<<|\+=|\.append\s*\(|\+\s*)[A-Za-z_(]') { $chainVar = $true }
      if (-not $chainVar) {
        for ($j = $i + 1; $j -lt [Math]::Min($i + 12, $lines.Count); $j++) {
          $l2 = $lines[$j].Trim()
          if ($l2 -match '^(<<|\+=|\+|\.append)') {
            if ($l2 -match '(<<|\+=|\+\s*|\.append\s*\(\s*)[A-Za-z_(]') { $chainVar = $true; break }
            if ($l2 -match '(<<|\+|,|\(|$)$') { continue }
            break
          } elseif ($l2 -eq '' -or $l2.StartsWith('//')) { continue } else { break }
        }
      }
      $escaped = $false
      for ($w = [Math]::Max(0, $i - 40); $w -le [Math]::Min($i + 9, $lines.Count - 1); $w++) { if ($escSet.ContainsKey($w)) { $escaped = $true; break } }
      $executed = $false
      for ($w = $i + 1; $w -le [Math]::Min($i + 45, $lines.Count - 1); $w++) { if ($execSet.ContainsKey($w)) { $executed = $true; break } }
      $ctx = 'exec:' + $(if ($executed) {'Y'} else {'N'}) + ';esc:' + $(if ($escaped) {'Y'} else {'N'})
      if ($hasPctS -or ($chainVar -and -not $hasNum)) {
        $det = "$kwTxt | $ctx | interp:$(if ($hasPctS) {'%s-family'} else {'var-concat'})"
        if ($escaped) { Add-Row 'ESCAPED' $fam $rel $ln 'MEDIUM' $det $code.Trim() }
        else {
          $prio = 'HIGH'; if (-not $executed) { $prio = 'MEDIUM(no-exec-in-window)' }
          Add-Row 'VULNERABLE' $fam $rel $ln $prio $det $code.Trim()
        }
      }
      elseif ($hasQ) { Add-Row 'PARAMETERIZED' $fam $rel $ln '-' "placeholder stmt: $kwTxt | $ctx" $code.Trim() }
      elseif ($hasNum) { Add-Row 'FALSE_POSITIVE' $fam $rel $ln '-' "numeric-only dynamic SQL (not injectable): $kwTxt | $ctx" $code.Trim() }
      else { Add-Row 'FALSE_POSITIVE' $fam $rel $ln '-' "literal-only SQL (no interpolation): $kwTxt | $ctx" $code.Trim() }
    }
# MARKER_PASS4
    # pass 3: dead (commented) findings
    $inBlock = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
      $work = $lines[$i]; $deadAll = $false
      if ($inBlock) {
        $ix = $work.IndexOf('*/')
        if ($ix -lt 0) { $deadAll = $true } else { $work = $work.Substring($ix + 2); $inBlock = $false }
      }
      $commentPart = ''
      if ($deadAll) { $commentPart = $work }
      else {
        $tmp = $work
        while ($true) {
          $o = $tmp.IndexOf('/*'); if ($o -lt 0) { break }
          $c = $tmp.IndexOf('*/', $o + 2)
          if ($c -lt 0) { $inBlock = $true; $tmp = $tmp.Substring(0, $o); break }
          $tmp = $tmp.Remove($o, $c - $o + 2)
        }
        $sl = $tmp.IndexOf('//')
        if ($sl -ge 0) { $commentPart = $tmp.Substring($sl) }
      }
      if ($commentPart -eq '') { continue }
      $hasKw = $false; foreach ($m in $strRe.Matches($commentPart)) { if ($kwRe.IsMatch($m.Value)) { $hasKw = $true; break } }
      if (-not $hasKw) { continue }
      if ($printfRe.IsMatch($commentPart)) { $fam = 'printf' }
      elseif ($formatRe.IsMatch($commentPart)) { $fam = 'Format' }
      elseif ($concatRe.IsMatch($commentPart)) { $fam = 'concat/stream' } else { $fam = 'comment-text' }
      Add-Row 'DEAD' $fam $rel ($i + 1) '-' 'commented-out / in /* */ block' $commentPart.Trim()
    }
  }
}
$rows | Export-Csv -Path $csv -NoTypeInformation -Encoding UTF8
Write-Host ("[{0}] rows: {1} -> {2}" -f $Tag, $rows.Count, $csv)
$rows | Group-Object Class | ForEach-Object { Write-Host ("  {0,-22}: {1}" -f $_.Name, $_.Count) }


