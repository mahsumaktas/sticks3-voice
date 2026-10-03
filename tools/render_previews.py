"""Compile the real framebuffer renderer and export clean PNGs for both locales."""
from pathlib import Path
import os
import subprocess
import tempfile
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/previews"


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="sticks3-preview-") as temp:
        temp = Path(temp)
        for language in ("en", "tr"):
            binary = temp / ("preview-" + language)
            subprocess.run([os.environ.get("CXX", "c++"), "-std=c++17", "-O2",
                            "-Ifirmware/main", f"-DSTICKS3_UI_TURKISH={int(language == 'tr')}",
                            "tools/preview.cpp", "firmware/main/voice_ui.cpp", "-o", str(binary)],
                           cwd=ROOT, check=True)
            frames = temp / language
            subprocess.run([str(binary), str(frames)], check=True, stdout=subprocess.DEVNULL)
            for path in sorted(frames.glob("*.ppm")):
                with Image.open(path) as frame:
                    frame.convert("RGB").save(OUT / f"{language}-{path.stem}.png", optimize=False)
            sheet = Image.new("RGB", (240*2,135*7))
            states = ("ready", "speaking", "waiting", "released", "usb", "timeout", "error")
            for col, theme in enumerate(("claude", "codex")):
                for row, state in enumerate(states):
                    with Image.open(OUT / f"{language}-{theme}-{state}.png") as frame:
                        sheet.paste(frame, (col*240, row*135))
            sheet.resize((960,1890),Image.Resampling.NEAREST).save(OUT / f"{language}-all-states.png")
        hero = Image.new("RGB", (480,135))
        for col, theme in enumerate(("claude", "codex")):
            with Image.open(OUT / f"en-{theme}-ready.png") as frame:
                hero.paste(frame, (col*240,0))
        hero.resize((1440,405), Image.Resampling.NEAREST).save(OUT / "hero.png")
    print("Generated both locales and hero from the actual C++ renderer.")


if __name__ == "__main__":
    main()
