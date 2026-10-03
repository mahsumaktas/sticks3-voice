import importlib.util
from pathlib import Path
import tempfile
import unittest
from PIL import Image

spec = importlib.util.spec_from_file_location('render_previews', Path(__file__).parents[1] / 'tools/render_previews.py')
previews = importlib.util.module_from_spec(spec)
spec.loader.exec_module(previews)


class PreviewVerification(unittest.TestCase):
    def test_compression_does_not_change_pixel_comparison(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            frame = Image.new('RGB', (32, 32), (10, 20, 30))
            frame.save(path, compress_level=0)
            original = path.read_bytes()
            previews.save_frame(frame, path, check=True)
            self.assertEqual(path.read_bytes(), original)

    def test_one_changed_pixel_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'frame.png'
            frame = Image.new('RGB', (32, 32), (10, 20, 30))
            frame.save(path)
            frame.putpixel((31, 31), (10, 20, 31))
            with self.assertRaises(ValueError):
                previews.save_frame(frame, path, check=True)
