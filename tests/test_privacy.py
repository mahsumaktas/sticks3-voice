import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('privacy_check', Path(__file__).parents[1] / 'tools/privacy_check.py')
scanner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(scanner)


class PublicationChecks(unittest.TestCase):
    def test_local_identity_patterns(self):
        # Construct synthetic examples so the scan does not flag its own fixtures.
        self.assertIn('personal home path', scanner.findings('doc.md', b'/Users/' + b'example/file'))
        self.assertIn('hardware address', scanner.findings('x.c', b':'.join([b'ab'] * 6)))
        self.assertIn('service token', scanner.findings('doc.md', b'ghp_' + b'x' * 36))

    def test_local_artifact_names(self):
        self.assertIn('private/generated file', scanner.findings('backups/data.bin', b''))
        self.assertIn('unreviewed binary or local artifact', scanner.findings('firmware.elf', b''))

    def test_generic_instructions_and_runtime_identity(self):
        self.assertEqual(scanner.findings('README.md', b'$IDF_PATH/export.sh; esp_efuse_mac_get_default(mac);'), [])
        self.assertEqual(scanner.findings('firmware.bin', b'generic firmware', release=True), [])

    def test_release_directory_must_exist_and_contain_files(self):
        with tempfile.TemporaryDirectory() as directory, patch.object(scanner.subprocess, 'check_output', return_value=b''):
            root = Path(directory)
            for path in (root / 'missing', root):
                self.assertEqual(scanner.main(['--extra', str(path)]), 1)
            artifact = root / 'README.md'
            artifact.write_text('public source archive')
            self.assertEqual(scanner.main(['--extra', str(artifact)]), 1)
            self.assertEqual(scanner.main(['--extra', str(root)]), 0)

    def test_release_symlink_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory, patch.object(scanner.subprocess, 'check_output', return_value=b''):
            root = Path(directory)
            artifact = root / 'README.md'
            artifact.write_text('public source archive')
            (root / 'linked.md').symlink_to(artifact)
            self.assertEqual(scanner.main(['--extra', str(root)]), 1)


if __name__ == '__main__':
    unittest.main()
