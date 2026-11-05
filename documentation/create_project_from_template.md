# `create_project_from_template.py`

Scaffold a brand‑new project by cloning this repository and renaming the
`onbings` / `vista` identifiers to your own **company** and **project** names.

The script mirrors the whole source tree into a destination directory and, while
copying, rewrites both **path names** and **text file contents** — preserving the
original letter casing of every match.

---

## What it does

- Copies **every** file and folder of the source template, including **empty** and
  **hidden** ones.
- Excludes only the **root `.git`** directory (plus anything you pass with
  `--exclude`).
- Substitutes two template tokens everywhere (file names, directory names and text
  file contents):

  | Template token | Replaced by      | CLI flag      |
  | -------------- | ---------------- | ------------- |
  | `onbings`      | *company* name   | `--company`   |
  | `vista`        | *project* name   | `--project`   |

- **Binary files** are copied byte‑for‑byte (never rewritten). A file is treated as
  binary if it contains a NUL byte or is not valid UTF‑8.
- File permissions (mode bits) are preserved.

### Case preservation

The casing of each matched token is reproduced on the replacement:

| Source            | Result (`company=evs-hwfw-`, `project=prism`) |
| ----------------- | --------------------------------------------- |
| `onbings`         | `evs-hwfw-`                                   |
| `ONBINGS`         | `EVS-HWFW-`                                    |
| `Onbings`         | `Evs-hwfw-`                                    |
| `VISTA`           | `PRISM`                                        |
| `Vista`           | `Prism`                                        |
| `VistA`           | `PrisM`                                        |
| `Onbings_VISTA`   | `Evs-hwfw-_PRISM`                              |

Separators (`-`, `_`, …) and every other character are kept verbatim; only the
`onbings` / `vista` tokens are replaced.

### Special rules (exceptions)

1. **This script is copied verbatim.** `create_project_from_template.py` contains
   the literal tokens `onbings` / `vista`; it is copied unchanged so it keeps
   working in the generated project (and never corrupts itself).

2. **CMake dependency namespaces stay intact.** In `CMakeLists.txt` and
   `*.cmake(.in)` files, a scoped reference such as `ONBINGS::bofstd` is **not**
   renamed — otherwise the link to the installed package would break. The project's
   own (unscoped) target names, e.g. `onbings-vista-objects`, **are** renamed.

3. **C++ identifiers stay valid.** When the company replacement contains a `-`
   (illegal in an identifier) and the token sits inside an identifier — next to `_`
   or `::` — the `-` becomes `_`. Example:

   ```cpp
   #define VISTA OnBings::Vista
   // becomes
   #define PRISM EvS_hwfw::Prism
   ```

   Dash‑joined names that are **not** identifiers (file names, CMake targets) keep
   their `-`, e.g. `onbings-vista-lib` → `evs-hwfw-prism-lib`.

---

## Usage

```text
python create_project_from_template.py -s <source> -c <company> -p <project> -d <destination> [options]
```

### Required arguments

| Flag               | Description                                            | Example              |
| ------------------ | ------------------------------------------------------ | -------------------- |
| `-s`, `--source`   | Source template directory                              | `C:\pro\onbings-vista` |
| `-c`, `--company`  | Company name that replaces `onbings`                   | `evs-hwfw`           |
| `-p`, `--project`  | Project name that replaces `vista`                     | `prism`              |
| `-d`, `--destination` | Destination directory                               | `C:\tmp\prism`       |

### Optional arguments

| Flag               | Description                                                        |
| ------------------ | ----------------------------------------------------------------- |
| `--from-company`   | Template token replaced by `--company` (default: `onbings`)       |
| `--from-project`   | Template token replaced by `--project` (default: `vista`)         |
| `--exclude NAME`   | Extra file/dir name to skip at any level (repeatable)             |
| `--force`          | Allow copying into a **non‑empty** destination directory          |
| `--dry-run`        | Report what would be done without writing anything                |

### Safety checks

The run aborts if:

- the source directory does not exist, or company/project is empty;
- source and destination are the same, or one is nested inside the other;
- the destination is non‑empty and `--force` was not given.

---

## Examples

Scaffold `prism` for company `evs-hwfw`, skipping the `out` build folder
(Windows, using the bundled virtual environment):

```powershell
.\venv\Scripts\python.exe documentation\create_project_from_template.py `
    -s C:\pro\onbings-vista `
    -c "evs-hwfw" `
    -p "prism" `
    -d C:\tmp\fromvista `
    --exclude out
```

Preview the operation first (nothing is written):

```powershell
.\venv\Scripts\python.exe documentation\create_project_from_template.py `
    -s C:\pro\onbings-vista -c "evs-hwfw" -p "prism" -d C:\tmp\fromvista `
    --exclude out --exclude venv --dry-run
```

Typical excludes for a clean scaffold: `--exclude out --exclude venv --exclude .vscode`.

### Sample output

```text
Source       : C:\pro\onbings-vista
Destination  : C:\tmp\fromvista
Company      : 'onbings' -> 'evs-hwfw'
Project      : 'vista' -> 'prism'
Excluding    : out

Copied: 128 directories, 642 files (598 text, 44 binary).
```

---

## Notes

- Per the "copy everything" rule, only the root `.git` is excluded by default, so
  large generated folders such as `out/` and `venv/` are copied unless you exclude
  them explicitly.
- Never point `--destination` inside `--source` (or vice‑versa); the script refuses
  this to avoid recursive copies and corrupting the template.
