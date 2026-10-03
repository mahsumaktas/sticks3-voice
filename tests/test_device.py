import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('device', Path(__file__).parents[1] / 'tools/device.py')
device = importlib.util.module_from_spec(spec)
spec.loader.exec_module(device)


class DeviceSafetyTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)

    def build(self):
        for path in ('bootloader/bootloader.bin', 'partition_table/partition-table.bin', 'sticks3_native_voice.bin'):
            target = self.root / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(b'firmware')

    def test_missing_artifact_never_opens_device(self):
        with patch.object(device, 'require_bootloader') as serial:
            self.assertEqual(device.main(['flash', '--port', 'TEST', '--build-dir', str(self.root)]), 1)
            serial.assert_not_called()

    def test_failed_backup_verification_prevents_write(self):
        self.build()
        with patch.object(device, 'require_bootloader'), patch.object(device, 'backup', side_effect=ValueError('verify failed')), patch.object(device, 'run_esptool') as flash:
            self.assertEqual(device.main(['flash', '--port', 'TEST', '--build-dir', str(self.root)]), 1)
            flash.assert_not_called()

    def test_explicit_skip_writes_only_expected_layout(self):
        self.build()
        with patch.object(device, 'require_bootloader'), patch.object(device, 'backup') as backup, patch.object(device, 'run_esptool') as flash:
            self.assertEqual(device.main(['flash', '--port', 'TEST', '--build-dir', str(self.root), '--no-backup']), 0)
            backup.assert_not_called()
            arguments = flash.call_args.args
            self.assertIn('0x10000', arguments)
            self.assertIn('0x8000', arguments)
            self.assertEqual(flash.call_args.kwargs, {'reset': True})

    def test_tampered_backup_prevents_serial_access(self):
        binary = self.root / 'flash.bin'
        binary.write_bytes(b'\x00' * device.FLASH_SIZE)
        binary.with_suffix('.json').write_text(json.dumps({'format': 1, 'bytes': device.FLASH_SIZE,
            'sha256': 'not-the-digest', 'verified_against_device': True}))
        with patch.object(device, 'require_bootloader') as serial:
            self.assertEqual(device.main(['restore', '--port', 'TEST', '--backup', str(binary)]), 1)
            serial.assert_not_called()

    def test_verified_backup_manifest_only_after_device_compare(self):
        def esptool(_port, command, *args):
            if command == 'read-flash':
                Path(args[-1]).write_bytes(b'\x00' * device.FLASH_SIZE)
            elif command == 'verify-flash':
                raise subprocess.CalledProcessError(1, 'esptool')
        with patch.object(device, 'run_esptool', side_effect=esptool):
            with self.assertRaises(subprocess.CalledProcessError):
                device.backup('TEST', self.root)
        self.assertEqual(list(self.root.glob('*.json')), [])

    def test_private_verified_backup_round_trip(self):
        def esptool(_port, command, *args):
            if command == 'read-flash':
                Path(args[-1]).write_bytes(b'\x00' * device.FLASH_SIZE)
        with patch.object(device, 'run_esptool', side_effect=esptool) as operation:
            result = device.backup('TEST', self.root)
        self.assertEqual(operation.call_args.args[1], 'verify-flash')
        device.validate_backup(result)
        self.assertEqual(result.stat().st_mode & 0o777, 0o600)


if __name__ == '__main__':
    unittest.main()
