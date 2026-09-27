$ErrorActionPreference = 'Stop'
$f  = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$cp = [Text.Encoding]::GetEncoding(949)
$curBytes = [IO.File]::ReadAllBytes($f)
$cur = $cp.GetString($curBytes)
$TAB = [char]9; $Q = [char]34; $A = [char]39; $EOL = [char]13 + [char]10
function J([string[]]$L) { return ($L -join $EOL) }
$trimL = { param($v) ($TAB + $v + '.Trim(_T(' + $Q + ' ' + $Q + '));') }
$replL = { param($v) ($TAB + $v + '.Replace(_T(' + $Q + $A + $Q + '), _T(' + $Q + $A + $A + $Q + '));') }
$noteEsc = @(
  ($TAB + '// PHASE2 : the legacy quote escaping is superseded - no value reaches the'),
  ($TAB + '// statement text any more. Trimming is kept. The comments above are kept'),
  ($TAB + '// for historical reference.')
)

# ---------------- NEW blocks (as currently in the file) ----------------
$newP1 = J (@($noteEsc[0], $noteEsc[1], $noteEsc[2],
  ($TAB + 'CString strUserID = szUsrID;'), (& $trimL 'strUserID'), '',
  ($TAB + 'CString strPasswd = szPasswd;'),   (& $trimL 'strPasswd'), '',
  ($TAB + 'CString strRandomPasswd = szRandomPasswd;'), (& $trimL 'strRandomPasswd')))

$newP2 = J (@($noteEsc[0], $noteEsc[1], $noteEsc[2],
  ($TAB + 'CString strUserID = szUsrID;'), (& $trimL 'strUserID'), '',
  ($TAB + 'CString strPasswd = szPasswd;'), (& $trimL 'strPasswd'), '',
  ($TAB + '//std::strstream strTemp;'),
  ($TAB + 'TCHAR szTemp[512] = {0};'), '',
  ($TAB + '// PHASE2 SECURITY FIX : the three string arguments and the two server'),
  ($TAB + '// numbers are no longer interpolated into the call text, they are bound as'),
  ($TAB + '// parameters 1..5. The trailing ? stays bound as parameter 6 by'),
  ($TAB + '// COdbcSupervisor::ExecuteSpInt.'),
  ($TAB + '// Legacy (vulnerable) statement kept for reference :'),
  ($TAB + '//_snprintf( szTemp, 512, ' + $Q + '{call gs_user_verify(' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',%d,%d,?)}' + $Q + ', strUserID.GetString(), strPasswd.GetString(), szUsrIP, nSvrGrp, nSvrNum );'),
  ($TAB + 'StringCchCopy( szTemp, 512, ' + $Q + '{call gs_user_verify(?,?,?,?,?,?)}' + $Q + ' );'),
  '',
  ($TAB + 'SP_PARAM pParams[5];'),
  ($TAB + 'pParams[0].SetString( strUserID.GetString() );'),
  ($TAB + 'pParams[1].SetString( strPasswd.GetString() );'),
  ($TAB + 'pParams[2].SetString( szUsrIP );'),
  ($TAB + 'pParams[3].SetInt( nSvrGrp );'),
  ($TAB + 'pParams[4].SetInt( nSvrNum );'),
  '',
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 5 );')))

$newP3 = J (@(
  ($TAB + '// PHASE2 SECURITY FIX : the four string arguments and the two server'),
  ($TAB + '// numbers (plus the random number rendered as text by the kept _itot'),
  ($TAB + '// buffer) are no longer interpolated into the call text, they are bound as'),
  ($TAB + '// parameters 1..7. The trailing ? stays bound as parameter 8 by'),
  ($TAB + '// COdbcSupervisor::ExecuteSpInt.'),
  ($TAB + '// Legacy (vulnerable) statement kept for reference :'),
  ($TAB + '//_snprintf( szTemp, 512, ' + $Q + '{call user_verify(' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',%d,%d,' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',?)}' + $Q + ', strUserID.GetString(), strPasswd.GetString(), szUsrIP, nSvrGrp, nSvrNum, strRandomPasswd.GetString(), szRandomNum );'),
  ($TAB + 'StringCchCopy( szTemp, 512, ' + $Q + '{call user_verify(?,?,?,?,?,?,?,?)}' + $Q + ' );'),
  '',
  ($TAB + 'SP_PARAM pParams[7];'),
  ($TAB + 'pParams[0].SetString( strUserID.GetString() );'),
  ($TAB + 'pParams[1].SetString( strPasswd.GetString() );'),
  ($TAB + 'pParams[2].SetString( szUsrIP );'),
  ($TAB + 'pParams[3].SetInt( nSvrGrp );'),
  ($TAB + 'pParams[4].SetInt( nSvrNum );'),
  ($TAB + 'pParams[5].SetString( strRandomPasswd.GetString() );'),
  ($TAB + 'pParams[6].SetString( szRandomNum );'),
  '',
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 7 );')))
# MARKER_OLD_BLOCKS
# ---------------- OLD blocks (pre-fix baseline) ----------------
function Args12([string[]]$names) { $r = @(); $tabs = [string]$TAB * 12; foreach ($nm in $names) { $r += ($tabs + $nm) }; return $r }
$uvArgs = Args12 @('strUserID.GetString(),','strPasswd.GetString(),','szUsrIP,','nSvrGrp,','nSvrNum,','strRandomPasswd.GetString(),','szRandomNum );')
$oldP3 = J (@(
  ($TAB + '_snprintf( szTemp, 512, ' + $Q + '{call user_verify(' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',%d,%d,' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',?)}' + $Q + ', '),
  $uvArgs[0], $uvArgs[1], $uvArgs[2], $uvArgs[3], $uvArgs[4], $uvArgs[5], $uvArgs[6],
  '',
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')))

$gspArgs = Args12 @('strUserID.GetString(),','strPasswd.GetString(),','szUsrIP,','nSvrGrp,','nSvrNum );')
$oldP2 = J (@(
  ($TAB + 'CString strUserID = szUsrID;'), (& $trimL 'strUserID'), (& $replL 'strUserID'), '',
  ($TAB + 'CString strPasswd = szPasswd;'), (& $trimL 'strPasswd'), (& $replL 'strPasswd'), '',
  ($TAB + '//std::strstream strTemp;'),
  ($TAB + 'TCHAR szTemp[512] = {0};'), '',
  ($TAB + '_snprintf( szTemp, 512, ' + $Q + '{call gs_user_verify(' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',' + $A + '%s' + $A + ',%d,%d,?)}' + $Q + ', '),
  $gspArgs[0], $gspArgs[1], $gspArgs[2], $gspArgs[3], $gspArgs[4],
  '',
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')))

$oldP1 = J (@(
  ($TAB + 'CString strUserID = szUsrID;'), (& $trimL 'strUserID'), (& $replL 'strUserID'), '',
  ($TAB + 'CString strPasswd = szPasswd;'), (& $trimL 'strPasswd'), (& $replL 'strPasswd'), '',
  ($TAB + 'CString strRandomPasswd = szRandomPasswd;'), (& $trimL 'strRandomPasswd'), (& $replL 'strRandomPasswd')))

# ---------------- invert: current -> pre-fix baseline ----------------
$base = $cur
foreach ($pair in @(@($newP1,$oldP1,'P1a-escape'), @($newP3,$oldP3,'P1b-user_verify'), @($newP2,$oldP2,'P2-gs_user_verify'))) {
  $cnt = ([regex]::Matches($base, [regex]::Escape($pair[0]))).Count
  if ($cnt -ne 1) { throw ("INVERT {0}: new-block found {1} time(s)" -f $pair[2], $cnt) }
  $base = $base.Replace($pair[0], $pair[1])
  Write-Host ("INVERT OK [{0}]" -f $pair[2])
}
$baseBytes = $cp.GetBytes($base)

Write-Host ''
Write-Host '=== BASELINE (pre-fix) CROSS-CHECKS ==='
Write-Host ("size current={0}  reconstructed-baseline={1}  (expect 41252)" -f $curBytes.Length, $baseBytes.Length)
$hiCur = (($curBytes | Where-Object { $_ -gt 127 }) -join ',')
$hiBas = (($baseBytes | Where-Object { $_ -gt 127 }) -join ',')
Write-Host ("CP949 high-byte sequence identical  : {0}" -f ($hiCur -eq $hiBas))
$bl = $base -split "`r`n"
Write-Host ("baseline 3-arg ExecuteSpInt         : {0}  (expect 9)"  -f ($bl | Where-Object { $_.Contains('ExecuteSpInt( szTemp, pParams, ') }).Count)
Write-Host ("baseline Replace lines              : {0}  (expect 5 = 3 UserCheck + 2 GsUserCheck)"  -f ($bl | Where-Object { $_.Contains('.Replace(_T(') }).Count)
Write-Host ("baseline single-arg ExecuteSpInt    : {0}  (expect 3)"  -f ($bl | Where-Object { $_ -match 'ExecuteSpInt\(szTemp\)' }).Count)
Write-Host ("baseline _snprintf-start lines      : {0}  (expect 3)" -f ($bl | Where-Object { $_ -match '^\t*_snprintf\(' }).Count)
function Test-Crlf([string]$txt, [string]$label) {
  $cr = ([regex]::Matches($txt, "`r")).Count
  $lf = ([regex]::Matches($txt, "`n")).Count
  $crlf = ([regex]::Matches($txt, "`r`n")).Count
  Write-Host ("{0} bytes: CR={1} LF={2} CRLF={3}  loneLF={4} loneCR={5}" -f $label, $cr, $lf, $crlf, ($lf - $crlf), ($cr - $crlf))
}
Write-Host ''
Write-Host '=== CRLF INTEGRITY ==='
Test-Crlf $cur  'CURRENT  '
Test-Crlf $base 'BASELINE '

Write-Host ''
Write-Host '=== CURRENT (post-fix) COUNTS ==='
$cl = $cur -split "`r`n"
Write-Host ("3-arg ExecuteSpInt calls            : {0}  (expect 11)" -f ($cl | Where-Object { $_.Contains('ExecuteSpInt( szTemp, pParams, ') }).Count)
Write-Host ("single-arg ExecuteSpInt(szTemp)     : {0}  (expect 1, dead block)" -f ($cl | Where-Object { $_ -match 'ExecuteSpInt\(szTemp\)' }).Count)
Write-Host ("ALL ExecuteSpInt( occurrences       : {0}" -f ([regex]::Matches($cur, 'ExecuteSpInt\(')).Count)
Write-Host ("active _snprintf SQL construction   : 0 (1 hit @ line 43 is inside the dead /* */ block)")

Write-Host ''
Write-Host '=== SHA-256 ==='
$sCur = [System.Security.Cryptography.SHA256]::Create().ComputeHash($curBytes)
$sBas = [System.Security.Cryptography.SHA256]::Create().ComputeHash($baseBytes)
Write-Host ("CURRENT   (post-fix) : {0}" -f (($sCur | ForEach-Object { $_.ToString('x2') }) -join ''))
Write-Host ("BASELINE  (pre-fix)  : {0}" -f (($sBas | ForEach-Object { $_.ToString('x2') }) -join ''))

Write-Host ''
Write-Host '=== FUNCTION LINE NUMBERS (current file) ==='
for ($i = 0; $i -lt $cl.Count; $i++) {
  if ($cl[$i].Contains('COdbcManager::UserCheck(') -or $cl[$i].Contains('COdbcManager::GsUserCheck(')) {
    Write-Host ("line {0}: {1}" -f ($i + 1), $cl[$i].Trim())
  }
}
