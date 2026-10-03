#!/usr/bin/env python3
"""Explicit-port backup, verified flash and restore for an 8 MiB StickS3.

This tool never picks a board automatically or enters its running 1200-baud CDC
interface. Put the intended board into ROM download mode before using it.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import uuid

ROOT = Path(__file__).resolve().parents[1]
FLASH_SIZE = 8 * 1024 * 1024


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run_esptool(port: str, *arguments: str, reset: bool = False) -> None:
    command = [sys.executable, '-m', 'esptool', '--chip', 'esp32s3', '--port', port,
               '--baud', '460800', '--before', 'usb-reset', '--after',
               'watchdog-reset' if reset else 'no-reset', *map(str, arguments)]
    subprocess.run(command, check=True)


def require_bootloader(port: str) -> None:
    from serial.tools import list_ports
    selected = next((p for p in list_ports.comports() if p.device == port), None)
    if not selected or selected.vid != 0x303A or selected.pid not in (0x0009, 0x1001):
        raise ValueError('Selected port is not an Espressif ROM/JTAG download interface. '
                         'Hold the StickS3 power/reset button until the green LED blinks, then list ports again.')


def firmware_files(build: Path) -> list[tuple[str, Path]]:
    # Validate every input before opening the serial port or writing any flash.
    files = [('0x0', build / 'bootloader/bootloader.bin'),
             ('0x8000', build / 'partition_table/partition-table.bin'),
             ('0x10000', build / 'sticks3_native_voice.bin')]
    for _, path in files:
        if not path.is_file() or path.stat().st_size == 0:
            raise ValueError(f'Missing build artifact: {path}')
    if files[0][1].stat().st_size > 0x8000 or files[1][1].stat().st_size > 0x1000:
        raise ValueError('Bootloader or partition table exceeds the expected layout')
    if files[2][1].stat().st_size > 0x100000:
        raise ValueError('Application exceeds the configured 1 MiB factory partition')
    return files


def backup(port: str, directory: Path) -> Path:
    directory.mkdir(parents=True, exist_ok=True, mode=0o700)
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    path = directory / f'flash-{stamp}-{uuid.uuid4().hex[:8]}.bin'
    # Precreate privately; esptool overwrites this file in place.
    fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    os.close(fd)
    run_esptool(port, 'read-flash', '0', hex(FLASH_SIZE), str(path))
    if path.stat().st_size != FLASH_SIZE:
        raise ValueError('Backup is not exactly 8 MiB; refusing to flash')
    run_esptool(port, 'verify-flash', '0', str(path))
    metadata = {'format': 1, 'bytes': FLASH_SIZE, 'sha256': digest(path),
                'verified_against_device': True}
    manifest = path.with_suffix('.json')
    with open(manifest, 'x', encoding='utf-8') as output:
        json.dump(metadata, output, indent=2)
        output.write('\n')
    os.chmod(manifest, 0o600)
    print(f'Full flash backup verified: {path}')
    return path


def validate_backup(path: Path) -> None:
    metadata = json.loads(path.with_suffix('.json').read_text(encoding='utf-8'))
    if (not isinstance(metadata, dict) or metadata.get('format') != 1 or metadata.get('bytes') != FLASH_SIZE
            or metadata.get('verified_against_device') is not True
            or path.stat().st_size != FLASH_SIZE or digest(path) != metadata.get('sha256')):
        raise ValueError('Backup metadata, size or SHA-256 verification failed')


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    commands.add_parser('list', help='List USB ports without printing hardware serial identifiers')
    for name in ('backup', 'flash', 'restore'):
        command = commands.add_parser(name)
        command.add_argument('--port', required=True, help='Exact port of the intended board in download mode')
        if name in ('flash', 'backup'):
            command.add_argument('--backup-dir', type=Path, default=ROOT / 'backups')
        if name == 'flash':
            command.add_argument('--build-dir', type=Path, default=ROOT / 'firmware/build')
            command.add_argument('--no-backup', action='store_true', help='Explicitly skip backup (only if a verified backup exists)')
        if name == 'restore':
            command.add_argument('--backup', type=Path, required=True, help='Full .bin backup with adjacent .json manifest')
    args = parser.parse_args(argv)
    if args.command == 'list':
        from serial.tools import list_ports
        for port in list_ports.comports():
            if port.vid == 0x303A:
                print(f'{port.device}: {port.description} (USB {port.vid:04x}:{port.pid:04x})')
        return 0
    try:
        files = firmware_files(args.build_dir) if args.command == 'flash' else []
        if args.command == 'restore':
            validate_backup(args.backup)
        require_bootloader(args.port)
        if args.command == 'backup':
            backup(args.port, args.backup_dir)
        elif args.command == 'flash':
            if not args.no_backup:
                backup(args.port, args.backup_dir)
            image_args = [part for offset, path in files for part in (offset, str(path))]
            run_esptool(args.port, 'write-flash', '--flash-mode', 'dio', '--flash-freq', '80m',
                        '--flash-size', '8MB', *image_args, reset=True)
        else:
            run_esptool(args.port, 'write-flash', '--flash-size', '8MB', '0', str(args.backup), reset=True)
        return 0
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'Stopped: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
