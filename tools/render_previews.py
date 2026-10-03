"""Compile the real framebuffer renderer and export clean PNGs for both locales."""
from pathlib import Path
import argparse
import os
import subprocess
import tempfile
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/previews"


def save_frame(frame, path, check=False):
    if check:
        with Image.open(path) as expected:
            if expected.size != frame.size or expected.convert("RGB").tobytes() != frame.convert("RGB").tobytes():
                raise ValueError(f"Preview pixels changed: {path.name}")
    else:
        frame.save(path, optimize=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Compare decoded pixels without rewriting PNGs")
    check = parser.parse_args().check
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
                    save_frame(frame.convert("RGB"), OUT / f"{language}-{path.stem}.png", check)
            sheet = Image.new("RGB", (240*2,135*7))
            states = ("ready", "speaking", "waiting", "released", "usb", "timeout", "error")
            for col, theme in enumerate(("claude", "codex")):
                for row, state in enumerate(states):
                    with Image.open(OUT / f"{language}-{theme}-{state}.png") as frame:
                        sheet.paste(frame, (col*240, row*135))
            save_frame(sheet.resize((960,1890),Image.Resampling.NEAREST), OUT / f"{language}-all-states.png", check)
        hero = Image.new("RGB", (480,135))
        for col, theme in enumerate(("claude", "codex")):
            with Image.open(OUT / f"en-{theme}-ready.png") as frame:
                hero.paste(frame, (col*240,0))
        save_frame(hero.resize((1440,405), Image.Resampling.NEAREST), OUT / "hero.png", check)
    print(("Verified" if check else "Generated") + " both locales and hero from the actual C++ renderer.")


if __name__ == "__main__":
    main()
