#!/usr/bin/env python3
"""Scaffold a new project from this template.

Copy every file and folder (including empty and hidden ones) from a source
template directory to a destination, excluding only the root ``.git`` folder.
While copying, the template identifiers are substituted both in path names and
in text file contents, preserving the original casing:

    onbings  ->  <company>   (e.g. evs-hwfw-)
    vista    ->  <project>   (e.g. prism)

Case is preserved per matched token, for example::

    Onbings_VISTA          ->  Evs-hwfw-_PRISM
    ONBINGS-projet-VistA   ->  EVS-HWFW--projet-PrisM

Separators (``-``, ``_`` ...) and every other character are kept verbatim; only
the ``onbings`` / ``vista`` tokens are replaced.

Example::

    python create_project_from_template.py \
        --source C:\\pro\\onbings-vista \
        --company evs-hwfw- \
        --project prism \
        --destination C:\\tmp

"""
#.\venv\Scripts\python.exe documentation\create_project_from_template.py -s C:\pro\onbings-vista -c "evs-hwfw" -p "prism" -d C:\tmp\fromvista --exclude out        

from __future__ import annotations

import argparse
import os
import re
import shutil
import sys
from pathlib import Path

# Size of the leading chunk read to sniff whether a file is binary.
_SNIFF_BYTES = 8192


def fail(message: str) -> "None":
    print(f"error: {message}", file=sys.stderr)
    sys.exit(2)


def apply_case(source_match: str, replacement: str) -> str:
    """Return *replacement* re-cased to mimic the casing of *source_match*."""
    if source_match.isupper():
        return replacement.upper()
    if source_match.islower():
        return replacement.lower()
    if source_match == source_match.capitalize():  # Title case: first upper, rest lower.
        return replacement.capitalize()
    # Mixed casing: copy the case position by position; extra characters stay lower.
    base = replacement.lower()
    out = []
    for i, ch in enumerate(base):
        if i < len(source_match) and source_match[i].isupper():
            out.append(ch.upper())
        else:
            out.append(ch)
    return "".join(out)


def build_substituter(from_company: str, company: str, from_project: str, project: str):
    """Return a function that applies the case-preserving token substitutions."""
    replacements = {from_company.lower(): company, from_project.lower(): project}
    # Longest token first so overlapping tokens never shadow each other.
    tokens = sorted({from_company, from_project}, key=len, reverse=True)
    pattern = re.compile("|".join(re.escape(t) for t in tokens), re.IGNORECASE)

    def _repl(match: "re.Match[str]", kind: str) -> str:
        matched = match.group(0)
        text = match.string
        prev_ch = text[match.start() - 1] if match.start() > 0 else ""
        next_ch = text[match.end()] if match.end() < len(text) else ""
        next2 = text[match.end():match.end() + 2]
        prev2 = text[max(0, match.start() - 2):match.start()]
        # Exception 1: in CMake, a scoped dependency namespace (e.g. ONBINGS::bofstd)
        # must stay as-is, otherwise the link to the installed package breaks.
        if kind == "cmake" and (next2 == "::" or prev2 == "::"):
            return matched
        replacement = apply_case(matched, replacements[matched.lower()])
        # Exception 2: inside an identifier (underscore-joined macro like
        # ONBINGS_VISTA_EXPORT, or a C++ namespace like OnBings::Vista), a '-' coming
        # from the company token is illegal, so switch it to '_'. Dash-joined names
        # (files, CMake targets) keep '-'.
        if "-" in replacement:
            in_identifier = (prev_ch == "_" or next_ch == "_"
                             or next2 == "::" or prev2 == "::")
            if in_identifier:
                replacement = replacement.replace("-", "_")
        return replacement

    def substitute(text: str, kind: str = "other") -> str:
        return pattern.sub(lambda m: _repl(m, kind), text)

    return substitute


def substitute_path(relative: str, substitute) -> Path:
    return Path(*[substitute(part) for part in Path(relative).parts])


def is_binary(path: Path) -> bool:
    """Best-effort detection: a NUL byte or non-UTF-8 content marks a binary file."""
    with open(path, "rb") as handle:
        chunk = handle.read(_SNIFF_BYTES)
    if b"\x00" in chunk:
        return True
    try:
        path.read_bytes().decode("utf-8")
    except UnicodeDecodeError:
        return True
    return False


def file_kind(name: str) -> str:
    """Classify a file so substitution can apply CMake-specific namespace rules."""
    low = name.lower()
    if low in ("cmakelists.txt", "cmakelists.txt.in") or low.endswith((".cmake", ".cmake.in")):
        return "cmake"
    return "other"


def copy_file(src: Path, dst: Path, substitute, dry_run: bool, verbatim: bool = False) -> str:
    binary = is_binary(src)
    if dry_run:
        return "binary" if binary else "text"
    dst.parent.mkdir(parents=True, exist_ok=True)
    # Verbatim files (e.g. this script itself) must keep their original tokens.
    if binary or verbatim:
        shutil.copyfile(src, dst)
    else:
        new_text = substitute(src.read_bytes().decode("utf-8"), file_kind(src.name))
        dst.write_bytes(new_text.encode("utf-8"))
    shutil.copymode(src, dst)
    return "binary" if binary else "text"


def copy_tree(source: Path, dest: Path, substitute, excludes: "set[str]", dry_run: bool):
    dirs = files = texts = binaries = 0
    # This script holds literal 'onbings'/'vista' tokens; copy it as-is to avoid corrupting it.
    self_path = Path(__file__).resolve()
    for dirpath, dirnames, filenames in os.walk(source):
        rel = os.path.relpath(dirpath, source)
        # Exclude the root .git directory only; prune user-provided excludes everywhere.
        if rel == os.curdir:
            dirnames[:] = [d for d in dirnames if d != ".git" and d not in excludes]
        else:
            dirnames[:] = [d for d in dirnames if d not in excludes]
        dirnames.sort()

        dest_dir = dest if rel == os.curdir else dest / substitute_path(rel, substitute)
        if not dry_run:
            dest_dir.mkdir(parents=True, exist_ok=True)
        dirs += 1

        for name in sorted(filenames):
            if name in excludes:
                continue
            src_file = Path(dirpath) / name
            dst_file = dest_dir / substitute(name)
            verbatim = src_file.resolve() == self_path
            kind = copy_file(src_file, dst_file, substitute, dry_run, verbatim)
            files += 1
            if kind == "text":
                texts += 1
            else:
                binaries += 1
    return dirs, files, texts, binaries


def parse_args(argv=None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("-s", "--source", required=True,
                        help=r"Source template directory (e.g. C:\pro\onbings-vista)")
    parser.add_argument("-c", "--company", required=True,
                        help="Company name replacing 'onbings' (e.g. evs-hwfw-)")
    parser.add_argument("-p", "--project", required=True,
                        help="Project name replacing 'vista' (e.g. prism)")
    parser.add_argument("-d", "--destination", required=True,
                        help=r"Destination directory (e.g. C:\tmp)")
    parser.add_argument("--from-company", default="onbings",
                        help="Template token replaced by --company (default: onbings)")
    parser.add_argument("--from-project", default="vista",
                        help="Template token replaced by --project (default: vista)")
    parser.add_argument("--exclude", action="append", default=[], metavar="NAME",
                        help="Additional file/dir name to skip at any level (repeatable)")
    parser.add_argument("--force", action="store_true",
                        help="Allow copying into a non-empty destination directory")
    parser.add_argument("--dry-run", action="store_true",
                        help="Report what would be done without writing anything")
    return parser.parse_args(argv)


def validate(args: argparse.Namespace):
    source = Path(args.source).expanduser().resolve()
    dest = Path(args.destination).expanduser().resolve()

    if not source.is_dir():
        fail(f"source directory does not exist: {source}")
    if not args.company.strip():
        fail("company name must not be empty")
    if not args.project.strip():
        fail("project name must not be empty")
    if not args.from_company.strip() or not args.from_project.strip():
        fail("template tokens (--from-company / --from-project) must not be empty")
    if source == dest:
        fail("source and destination must differ")
    if dest.is_relative_to(source):
        fail("destination must not be inside the source directory")
    if source.is_relative_to(dest):
        fail("source must not be inside the destination directory")
    if dest.exists() and any(dest.iterdir()) and not args.force:
        fail(f"destination is not empty: {dest} (use --force to proceed)")
    return source, dest


def main(argv=None) -> int:
    args = parse_args(argv)
    source, dest = validate(args)
    substitute = build_substituter(args.from_company, args.company,
                                   args.from_project, args.project)
    excludes = set(args.exclude)

    print(f"Source       : {source}")
    print(f"Destination  : {dest}")
    print(f"Company      : {args.from_company!r} -> {args.company!r}")
    print(f"Project      : {args.from_project!r} -> {args.project!r}")
    if excludes:
        print(f"Excluding    : {', '.join(sorted(excludes))}")
    if args.dry_run:
        print("Mode         : DRY-RUN (no files written)")

    dirs, files, texts, binaries = copy_tree(source, dest, substitute, excludes, args.dry_run)

    verb = "Would copy" if args.dry_run else "Copied"
    print(f"\n{verb}: {dirs} directories, {files} files "
          f"({texts} text, {binaries} binary).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
