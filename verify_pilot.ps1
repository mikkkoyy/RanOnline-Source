param(
    [string[]]$ExtraFlags = @()
)
$ErrorActionPreference = 'Continue'
$msvc   = 'D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207'
$cl     = Join-Path $msvc 'bin\Hostx64\x86\cl.exe'
$winsdk = (Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\Include' |
           Sort-Object Name -Descending | Select-Object -First 1).FullName
$r      = 'D:\FILES\project\modernization RanOnline'
$libNet = Join-Path $r 'Lib_Network'

$projDirs = @(
    '.',
    '..\Lib_Engine', '..\Lib_Engine\Common', '..\Lib_Engine\DxCommon',
    '..\Lib_Engine\DxCommon9', '..\Lib_Engine\DxEffect', '..\Lib_Engine\DxEffect\Char',
    '..\Lib_Engine\DxEffect\EffAni', '..\Lib_Engine\DxEffect\EffKeep',
    '..\Lib_Engine\DxEffect\EffProj', '..\Lib_Engine\DxEffect\Single',
    '..\Lib_Engine\DxFrame', '..\Lib_Engine\DxOctree', '..\Lib_Engine\Meshs',
    '..\Lib_Engine\DxSound', '..\Lib_Engine\G-Logic', '..\Lib_Engine\GUInterface',
    '..\Lib_Engine\Utils', '..\Lib_Helper', '..\Lib_Client', '..\Lib_Client\G-Logic',
    '..\Lib_Client\NpcTalk', '..\Lib_ClientUI', '..\Lib_ClientUI\Interface',
    '..\Tik\DXInclude', '..\Tik\Include'
)

$resolve = { param($rel)
    if ($rel -eq '.') { return $libNet }
    $full = Join-Path $r ($rel -replace '^\.\.\\', '')
    if (Test-Path $full) { return $full }
    return $null
}

$incParts = @(
    (Join-Path $msvc 'include'),
    (Join-Path $msvc 'atlmfc\include'),
    (Join-Path $winsdk 'ucrt'),
    (Join-Path $winsdk 'um'),
    (Join-Path $winsdk 'shared')
)
foreach ($d in $projDirs) {
    $p = & $resolve $d
    if ($p) { $incParts += $p }
}
$env:INCLUDE = ($incParts -join ';')
$env:LIB = @(
    (Join-Path $msvc 'lib\x86'),
    (Join-Path $msvc 'atlmfc\lib\x86'),
    (Join-Path $winsdk 'ucrt\lib\x86'),
    (Join-Path $winsdk 'um\lib\x86')
) -join ';'

Set-Location $libNet
Write-Host '=== CL INVOCATION ==='
Write-Host ('cl ' + (($ExtraFlags | ForEach-Object { $_ }) -join ' ') + ' /c /EHsc /W3 /DWIN32 /DNDEBUG /D_LIB /D_WINSOCK_DEPRECATED_NO_WARNINGS s_COdbcUserCheck.cpp s_COdbcSupervisor.cpp')
Write-Host '=== RAW OUTPUT (C4828 codepage noise filtered) ==='
$out = & $cl /nologo @ExtraFlags /c /EHsc /W3 /DWIN32 /DNDEBUG /D_LIB /D_WINSOCK_DEPRECATED_NO_WARNINGS s_COdbcUserCheck.cpp s_COdbcSupervisor.cpp 2>&1
$exit = $LASTEXITCODE
($out | ForEach-Object { "$_" } | Where-Object { $_ -notmatch 'C4828' }) -join "`n" | Write-Host
Write-Host "=== EXIT CODE: $exit ==="
