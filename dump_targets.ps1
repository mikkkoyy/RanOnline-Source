$ErrorActionPreference = 'Continue'
$all = Import-Csv 'D:\FILES\project\modernization RanOnline\sql_audit_partC.csv'
$targets = @(
  'Lib_Network\s_COdbcUser.cpp',
  'Lib_Client\G-Logic\GLCharInvenMsg.cpp',
  'Lib_Client\G-Logic\GLChar.cpp',
  'Lib_Client\G-Logic\GLCharPMarketMsg.cpp',
  'GMTool\Logic\GMToolOdbcUser.cpp',
  'GMTool\Logic\GMToolOdbcConn.cpp',
  'GMTool\Logic\GMToolOdbcChar.cpp',
  'GMTool\Logic\GMToolOdbcShop.cpp',
  'GMTool\Logic\GMToolOdbcGM.cpp',
  'Lib_Network\s_COdbcSupervisor.cpp',
  'Lib_Network\s_COdbcLogEvent.cpp',
  'Lib_Network\s_COdbcSMS.cpp',
  'Lib_Network\s_COdbcUserGetUserInfo.cpp',
  'Lib_Network\s_OdbcGameVehicle.cpp'
)
$sb = New-Object System.Text.StringBuilder
foreach ($t in $targets) {
  $rows = $all | Where-Object { $_.File -eq $t -and $_.Class -eq 'VULNERABLE' }
  [void]$sb.AppendLine("--- $t  (VULNERABLE: $($rows.Count)) ---")
  foreach ($r in $rows) {
    [void]$sb.AppendLine(("{0} [{1}] {2} | {3}" -f $r.Line, $r.Family, $r.Details, $r.Snippet))
  }
  [void]$sb.AppendLine('')
}
[IO.File]::WriteAllText('D:\FILES\project\modernization RanOnline\audit_target_rows.txt', $sb.ToString(), [Text.UTF8Encoding]::new($false))
Write-Host 'written audit_target_rows.txt'
