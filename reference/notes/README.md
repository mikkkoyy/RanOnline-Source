# notes/

Audit output, the dependency register, and one-off research scripts.

| File | What it is |
| ---- | ---------- |
| [modern-legacy-dependencies.md](modern-legacy-dependencies.md) | **Read this first.** The register of every place `modern/` is allowed to touch `legacy/`, with the reason for each. |
| `audit_sql.ps1`, `audit_sql2.ps1` | Audit scripts: scan the legacy `Lib_Network` ODBC layer for SQL built by string formatting. |
| `audit_vulnerable_list.txt` | Findings from the audit — call sites where a value is interpolated into SQL. |
| `audit_target_rows.txt` | The same findings grouped by file, with the flagged source line. |
| `sql_audit_partC.csv` | Structured findings: class, family, file, line, priority, detail, source snippet. Includes `DEAD` rows (commented-out or inside `/* */`) so they are not counted as live. |
| `phase3a_2sites.diff`, `sp_batch_8sites.diff` | Preview diffs of the remediation batches. Review material, not patches to apply — paths and line numbers predate the move to `legacy/`. |

## About the `.ps1` and `.diff` artifacts

These are kept as a record of the audit and of what was proposed. They contain
**absolute paths from before the repository restructure** and will not run
as-is:

- scripts point at `...\modernization RanOnline\Lib_Network\...`, which is now
  `legacy/Lib_Network\...`;
- diffs were produced against a temp preview directory and are already applied
  to the legacy source, so applying them again would conflict.

They are documentation of past work, not tooling. If an audit needs to be re-run,
copy the script, fix the paths, and keep the copy out of the repository.

## Related

- The modern replacement for the ODBC layer is not written yet. When it is, the
  findings here are the checklist for what the parameterised implementation must
  not reproduce.
