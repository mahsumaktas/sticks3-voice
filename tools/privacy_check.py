#!/usr/bin/env python3
"""Scan publication files for common secrets, private paths and device IDs.

Heuristics supplement review; they are not a proof that arbitrary data is public.
Only Git-listed files are inspected by default. No machine-specific denylist is
stored in the repository. Use --extra on a release directory before publishing.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PATTERNS = {
    'personal home path': re.compile(rb'(?:/Users/|/home/)[A-Za-z0-9_.-]+|[A-Z]:\\Users\\[^\\\r\n]+'),
    'hardware address': re.compile(rb'(?i)(?<![0-9a-f])(?:[0-9a-f]{2}:){5}[0-9a-f]{2}(?![0-9a-f])'),
    'private LAN address': re.compile(rb'(?<![0-9])(?:192\.168\.[0-9]{1,3}\.[0-9]{1,3}|10\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})(?![0-9])'),
    'private key': re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
    'service token': re.compile(rb'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|sk-(?:proj-)?[A-Za-z0-9_-]{32,}|AIza[A-Za-z0-9_-]{30,})'),
}
FORBIDDEN_PARTS = {'backups', '.venv', 'managed_components', '__pycache__', '.git', 'node_modules'}
FORBIDDEN_NAMES = {'credentials.h', '.env', 'sdkconfig', 'sdkconfig.old', 'id_rsa', 'id_ed25519'}


def findings(name: str, content: bytes, release: bool = False) -> list[str]:
    path = Path(name)
    failures = []
    if set(path.parts) & FORBIDDEN_PARTS or path.name in FORBIDDEN_NAMES:
        failures.append('private/generated file')
    if path.suffix in {'.log', '.elf', '.map', '.pem', '.key'} or (path.suffix == '.bin' and not release):
        failures.append('unreviewed binary or local artifact')
    for label, pattern in PATTERNS.items():
        if pattern.search(content):
            failures.append(label)
    if path.suffix == '.png':
        from PIL import Image
        from io import BytesIO
        image = Image.open(BytesIO(content))
        if set(image.info) - {'srgb', 'gamma'} or image.getexif():
            failures.append('image metadata')
    return failures


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--extra', type=Path, help='Also scan release contents, including firmware bytes')
    args = parser.parse_args(argv)
    listed = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=ROOT)
    paths = sorted(set(p.decode() for p in listed.split(b'\0') if p))
    errors = []
    for name in paths:
        path = ROOT / name
        if path.is_symlink():
            errors.append((name, ['symlink requires manual review']))
        elif path.is_file():
            hits = findings(name, path.read_bytes())
            if hits:
                errors.append((name, hits))
    if args.extra:
        extra_paths = list(args.extra.rglob('*')) if args.extra.is_dir() else []
        if args.extra.is_symlink() or not any(p.is_file() for p in extra_paths):
            errors.append(('release', ['expected a nonempty directory, not a symlink']))
        for path in extra_paths:
            if path.is_symlink():
                errors.append(('release/' + str(path.relative_to(args.extra)), ['symlink requires manual review']))
            elif path.is_file():
                name = str(path.relative_to(args.extra))
                hits = findings(name, path.read_bytes(), release=True)
                if hits:
                    errors.append(('release/' + name, hits))
    for name, reasons in errors:
        # Print category + filename, never the matching private value.
        print(f'{name}: {", ".join(reasons)}', file=sys.stderr)
    if errors:
        return 1
    print(f'Privacy heuristics passed for {len(paths)} publication files' + (' and release contents.' if args.extra else '.'))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
