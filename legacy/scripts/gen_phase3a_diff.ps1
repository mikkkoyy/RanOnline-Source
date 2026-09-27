param([switch]$ForReal)
$ErrorActionPreference = 'Stop'
$src     = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$scratch = Join-Path $env:TEMP 'phase3a_preview\s_COdbcUserCheck.cpp'
$diffOut = 'D:\FILES\project\modernization RanOnline\phase3a_2sites.diff'
$cp949   = [Text.Encoding]::GetEncoding(949)
$EOL     = "`r`n"
$TAB     = [char]9
$Q       = [char]34   # double quote
$text    = $cp949.GetString([IO.File]::ReadAllBytes($src))
$lines   = $text -split "`r`n"

function J([string[]]$L) { return ($L -join $EOL) }

# Slice a 1-based inclusive line range as the OLD anchor. Validates that the
# first line and at least one other line contain the required markers, so a
# line-number drift fails loudly instead of patching the wrong region.
function Slice([int]$a, [int]$b, [string]$firstMust, [string[]]$mustAny) {
  $seg = $lines[($a - 1)..($b - 1)]
  if (-not $seg[0].Contains($firstMust)) {
    throw ("SLICE {0}-{1}: first line does not contain '{2}'" -f $a, $b, $firstMust)
  }
  foreach ($m in $mustAny) {
    if (-not (($seg -join ' ').Contains($m))) {
      throw ("SLICE {0}-{1}: does not contain '{2}'" -f $a, $b, $m)
    }
  }
  return $seg
}

# ---- OLD anchors, extracted verbatim from the current file ----------------
$oldP1a = @(Slice 354 364 'CString strUserID = szUsrID;' @('strRandomPasswd.Replace'))
$oldP1b = @(Slice 382 391 'user_verify' @('ExecuteSpInt(szTemp);'))
$oldP2  = @(Slice 965 983 'CString strUserID = szUsrID;' @('gs_user_verify'))

# ---- NEW blocks ------------------------------------------------------------
$trimLine = { param($v) ($TAB + $v + '.Trim(_T(' + $Q + ' ' + $Q + '));') }

$refUser = $TAB + '//' + ((($oldP1b[0..7] | ForEach-Object { $_.Trim() }) -join ' '))
$refGsp  = $TAB + '//' + ((($oldP2[11..16]  | ForEach-Object { $_.Trim() }) -join ' '))

$newP1a = @(
  ($TAB + '// PHASE2 : the legacy quote escaping is superseded - no value reaches the'),
  ($TAB + '// statement text any more. Trimming is kept. The comments above are kept'),
  ($TAB + '// for historical reference.'),
  ($TAB + 'CString strUserID = szUsrID;'),
  (& $trimLine 'strUserID'),
  '',
  ($TAB + 'CString strPasswd = szPasswd;'),
  (& $trimLine 'strPasswd'),
  '',
  ($TAB + 'CString strRandomPasswd = szRandomPasswd;'),
  (& $trimLine 'strRandomPasswd')
)

$newP1b = @(
  ($TAB + '// PHASE2 SECURITY FIX : the four string arguments and the two server'),
  ($TAB + '// numbers (plus the random number rendered as text by the kept _itot'),
  ($TAB + '// buffer) are no longer interpolated into the call text, they are bound as'),
  ($TAB + '// parameters 1..7. The trailing ? stays bound as parameter 8 by'),
  ($TAB + '// COdbcSupervisor::ExecuteSpInt.'),
  ($TAB + '// Legacy (vulnerable) statement kept for reference :'),
  $refUser,
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
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 7 );')
)

$newP2 = @(
  ($TAB + '// PHASE2 : the legacy quote escaping is superseded - no value reaches the'),
  ($TAB + '// statement text any more. Trimming is kept. The comments above are kept'),
  ($TAB + '// for historical reference.'),
  ($TAB + 'CString strUserID = szUsrID;'),
  (& $trimLine 'strUserID'),
  '',
  ($TAB + 'CString strPasswd = szPasswd;'),
  (& $trimLine 'strPasswd'),
  '',
  ($TAB + '//std::strstream strTemp;'),
  ($TAB + 'TCHAR szTemp[512] = {0};'),
  '',
  ($TAB + '// PHASE2 SECURITY FIX : the three string arguments and the two server'),
  ($TAB + '// numbers are no longer interpolated into the call text, they are bound as'),
  ($TAB + '// parameters 1..5. The trailing ? stays bound as parameter 6 by'),
  ($TAB + '// COdbcSupervisor::ExecuteSpInt.'),
  ($TAB + '// Legacy (vulnerable) statement kept for reference :'),
  $refGsp,
  ($TAB + 'StringCchCopy( szTemp, 512, ' + $Q + '{call gs_user_verify(?,?,?,?,?,?)}' + $Q + ' );'),
  '',
  ($TAB + 'SP_PARAM pParams[5];'),
  ($TAB + 'pParams[0].SetString( strUserID.GetString() );'),
  ($TAB + 'pParams[1].SetString( strPasswd.GetString() );'),
  ($TAB + 'pParams[2].SetString( szUsrIP );'),
  ($TAB + 'pParams[3].SetInt( nSvrGrp );'),
  ($TAB + 'pParams[4].SetInt( nSvrNum );'),
  '',
  ($TAB + 'int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 5 );')
)
# MARKER_PATCHES
$patches = @(
@{ name = 'UserCheck : escape-block (keep Trim, drop Replace)'; old = $oldP1a; new = $newP1a },
@{ name = 'UserCheck : user_verify call (7 inputs + output)';   old = $oldP1b; new = $newP1b },
@{ name = 'GsUserCheck : full block (keep Trim, drop Replace, 5 inputs + output)'; old = $oldP2; new = $newP2 }
)

$ok = $true
foreach ($p in $patches) {
  $oldStr = J $p.old
  $newStr = J $p.new
  $cnt = ([regex]::Matches($text, [regex]::Escape($oldStr))).Count
  if ($cnt -ne 1) {
    Write-Host ("FAIL [{0}] : anchor found {1} time(s), expected 1" -f $p.name, $cnt)
    $ok = $false
    break
  }
  $text = $text.Replace($oldStr, $newStr)
  Write-Host ("OK   [{0}]" -f $p.name)
}

if ($ok) {
  if ($ForReal) {
    [IO.File]::WriteAllBytes($src, $cp949.GetBytes($text))
    Write-Host ("APPLIED to {0}" -f $src)
  } else {
    New-Item -ItemType Directory -Force -Path (Split-Path $scratch) | Out-Null
    [IO.File]::WriteAllBytes($scratch, $cp949.GetBytes($text))
    Write-Host ("SCRATCH written to {0}" -f $scratch)
  }
} else {
  Write-Host 'PATCH ABORTED - no files written'
}
