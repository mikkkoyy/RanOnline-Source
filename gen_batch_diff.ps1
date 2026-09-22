param([switch]$ForReal)
$ErrorActionPreference = 'Stop'
$src     = 'D:\FILES\project\modernization RanOnline\Lib_Network\s_COdbcUserCheck.cpp'
$scratch = Join-Path $env:TEMP 'sp_batch_preview\s_COdbcUserCheck.cpp'
$diffOut = 'D:\FILES\project\modernization RanOnline\sp_batch_8sites.diff'
$cp949   = [Text.Encoding]::GetEncoding(949)
$EOL     = "`r`n"

$text = $cp949.GetString([IO.File]::ReadAllBytes($src))
$orig = $text

function J([string[]]$L) { return ($L -join $EOL) }

$patches = @(
@{
  name = 'DaumUserPassCheck (security) #1'
  old = @(
'	_snprintf( szTemp, 128, "{call daum_user_passcheck(''%s'',''%s'',%d,?)}", szDaumGID, szDaumPasswd, nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'	// PHASE2 SECURITY FIX : the two string arguments and the check flag are no',
'	// longer interpolated into the call text, they are bound as parameters 1..3.',
'	// The trailing ? stays bound as parameter 4 by COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 128, "{call daum_user_passcheck(''%s'',''%s'',%d,?)}", szDaumGID, szDaumPasswd, nCheckFlag );',
'	StringCchCopy( szTemp, 128, "{call daum_user_passcheck(?,?,?,?)}" );',
'',
'	SP_PARAM pParams[3];',
'	pParams[0].SetString( szDaumGID );',
'	pParams[1].SetString( szDaumPasswd );',
'	pParams[2].SetInt( nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 3 );')
},
@{
  name = 'TerraUserPassCheck (security) #2'
  old = @(
'	_snprintf( szTemp, 128, "{call terra_user_passcheck(''%s'',''%s'',%d,?)}", szTID, szPasswd, nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'	// PHASE2 SECURITY FIX : the two string arguments and the check flag are no',
'	// longer interpolated into the call text, they are bound as parameters 1..3.',
'	// The trailing ? stays bound as parameter 4 by COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 128, "{call terra_user_passcheck(''%s'',''%s'',%d,?)}", szTID, szPasswd, nCheckFlag );',
'	StringCchCopy( szTemp, 128, "{call terra_user_passcheck(?,?,?,?)}" );',
'',
'	SP_PARAM pParams[3];',
'	pParams[0].SetString( szTID );',
'	pParams[1].SetString( szPasswd );',
'	pParams[2].SetInt( nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 3 );')
},
@{
  name = 'ExciteUserPassCheck (security) #3'
  old = @(
'	_snprintf( szTemp, 128, "{call excite_user_passcheck(''%s'',''%s'',''%s'',%d,?)}", szUID, szUserID,szPasswd, nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'	// PHASE2 SECURITY FIX : the three string arguments and the check flag are',
'	// no longer interpolated into the call text, they are bound as parameters',
'	// 1..4. The trailing ? stays bound as parameter 5 by',
'	// COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 128, "{call excite_user_passcheck(''%s'',''%s'',''%s'',%d,?)}", szUID, szUserID,szPasswd, nCheckFlag );',
'	StringCchCopy( szTemp, 128, "{call excite_user_passcheck(?,?,?,?,?)}" );',
'',
'	SP_PARAM pParams[4];',
'	pParams[0].SetString( szUID );',
'	pParams[1].SetString( szUserID );',
'	pParams[2].SetString( szPasswd );',
'	pParams[3].SetInt( nCheckFlag );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 4 );')
},
@{
  name = 'daum_user_verify (DaumUserCheck, Kor flow) #4'
  old = @(
'		_snprintf( szTemp, 256, "{call daum_user_verify(''%s'',''%s'',%d,%d,?)}", szKorUserID, szUserIP, nSvrGrp, nSvrNum );',
'',
'		int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'		// PHASE2 SECURITY FIX : the two string arguments and the two server',
'		// numbers are no longer interpolated into the call text, they are bound',
'		// as parameters 1..4. The trailing ? stays bound as parameter 5 by',
'		// COdbcSupervisor::ExecuteSpInt.',
'		// Legacy (vulnerable) statement kept for reference :',
'		//_snprintf( szTemp, 256, "{call daum_user_verify(''%s'',''%s'',%d,%d,?)}", szKorUserID, szUserIP, nSvrGrp, nSvrNum );',
'		StringCchCopy( szTemp, 256, "{call daum_user_verify(?,?,?,?,?)}" );',
'',
'		SP_PARAM pParams[4];',
'		pParams[0].SetString( (const TCHAR*) szKorUserID );',
'		pParams[1].SetString( szUserIP );',
'		pParams[2].SetInt( nSvrGrp );',
'		pParams[3].SetInt( nSvrNum );',
'',
'		int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 4 );')
},
@{
  name = 'excite_user_verify (ExciteUserCheck) #5'
  old = @(
'	_snprintf( szTemp, 512, "{call excite_user_verify(''%s'',''%s'',%d,%d,?)}", szUserID, szUserIP, nSvrGrp, nSvrNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'	// PHASE2 SECURITY FIX : the two string arguments and the two server',
'	// numbers are no longer interpolated into the call text, they are bound as',
'	// parameters 1..4. The trailing ? stays bound as parameter 5 by',
'	// COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 512, "{call excite_user_verify(''%s'',''%s'',%d,%d,?)}", szUserID, szUserIP, nSvrGrp, nSvrNum );',
'	StringCchCopy( szTemp, 512, "{call excite_user_verify(?,?,?,?,?)}" );',
'',
'	SP_PARAM pParams[4];',
'	pParams[0].SetString( szUserID );',
'	pParams[1].SetString( szUserIP );',
'	pParams[2].SetInt( nSvrGrp );',
'	pParams[3].SetInt( nSvrNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 4 );')
},
@{
  name = 'Japan_user_verify (JapanUserCheck) #6'
  old = @(
'	_snprintf( szTemp, 512, "{call Japan_user_verify(''%s'',''%s'',%d,%d,%d,?)}", szUserID, szUserIP,',
'		nSvrGrp, nSvrNum, nUserNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);	')
  new = @(
'	// PHASE2 SECURITY FIX : the two string arguments and the three server',
'	// numbers are no longer interpolated into the call text, they are bound as',
'	// parameters 1..5. The trailing ? stays bound as parameter 6 by',
'	// COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 512, "{call Japan_user_verify(''%s'',''%s'',%d,%d,%d,?)}", szUserID, szUserIP, nSvrGrp, nSvrNum, nUserNum );',
'	StringCchCopy( szTemp, 512, "{call Japan_user_verify(?,?,?,?,?,?)}" );',
'',
'	SP_PARAM pParams[5];',
'	pParams[0].SetString( szUserID );',
'	pParams[1].SetString( szUserIP );',
'	pParams[2].SetInt( nSvrGrp );',
'	pParams[3].SetInt( nSvrNum );',
'	pParams[4].SetInt( nUserNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 5 );')
},
@{
  name = 'gsp_user_verify (GspUserCheck) #7'
  old = @(
'		_snprintf( szTemp, 256, "{call gsp_user_verify(''%s'',''%s'',%d,%d,?)}", szGspUserID2, szUserIP, nSvrGrp, nSvrNum );',
'',
'		int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'		// PHASE2 SECURITY FIX : the two string arguments and the two server',
'		// numbers are no longer interpolated into the call text, they are bound',
'		// as parameters 1..4. The trailing ? stays bound as parameter 5 by',
'		// COdbcSupervisor::ExecuteSpInt.',
'		// Legacy (vulnerable) statement kept for reference :',
'		//_snprintf( szTemp, 256, "{call gsp_user_verify(''%s'',''%s'',%d,%d,?)}", szGspUserID2, szUserIP, nSvrGrp, nSvrNum );',
'		StringCchCopy( szTemp, 256, "{call gsp_user_verify(?,?,?,?,?)}" );',
'',
'		SP_PARAM pParams[4];',
'		pParams[0].SetString( (const TCHAR*) szGspUserID2 );',
'		pParams[1].SetString( szUserIP );',
'		pParams[2].SetInt( nSvrGrp );',
'		pParams[3].SetInt( nSvrNum );',
'',
'		int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 4 );')
},
@{
  name = 'terra_user_verify (TerraUserCheck) #8'
  old = @(
'	_snprintf( szTemp, 256, "{call terra_user_verify(''%s'',''X'',''X'',''X'',''%s'',%d,%d,?)}", szTerraLoginName, szUsrIP, nSvrGrp, nSvrNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt(szTemp);')
  new = @(
'	// PHASE2 SECURITY FIX : the login name and IP strings and the two server',
'	// numbers are no longer interpolated into the call text, they are bound as',
'	// parameters 1..4. The constant ''X'',''X'',''X'' literals stay in the call',
'	// text to preserve the stored procedure''s argument list. The trailing ?',
'	// stays bound as parameter 5 by COdbcSupervisor::ExecuteSpInt.',
'	// Legacy (vulnerable) statement kept for reference :',
'	//_snprintf( szTemp, 256, "{call terra_user_verify(''%s'',''X'',''X'',''X'',''%s'',%d,%d,?)}", szTerraLoginName, szUsrIP, nSvrGrp, nSvrNum );',
'	StringCchCopy( szTemp, 256, "{call terra_user_verify(?,''X'',''X'',''X'',?,?,?,?)}" );',
'',
'	SP_PARAM pParams[4];',
'	pParams[0].SetString( szTerraLoginName );',
'	pParams[1].SetString( szUsrIP );',
'	pParams[2].SetInt( nSvrGrp );',
'	pParams[3].SetInt( nSvrNum );',
'',
'	int nReturn = m_pUserDB->ExecuteSpInt( szTemp, pParams, 4 );')
}
)

# NOTE : the '' sequences above are PowerShell doubled single quotes inside
# single-quoted strings; they resolve to a literal ' in the actual patch text.

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
  $left = ([regex]::Matches($text, '\{call [^\r\n]*%s')).Count
  Write-Host ("remaining {{call lines containing %s : {0}" -f $left)
} else {
  Write-Host 'PATCH ABORTED - no files written'
}
