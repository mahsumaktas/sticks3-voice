#!/usr/bin/env python3
"""Run offline Python safety tests and sanitized C++ renderer tests."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(*args):
    subprocess.run(args, cwd=ROOT, check=True)


def main():
    run(sys.executable, '-m', 'unittest', 'discover', '-s', 'tests', '-v')
    with tempfile.TemporaryDirectory(prefix='sticks3-ui-') as directory:
        for language in ('en', 'tr'):
            binary = str(Path(directory) / ('test-ui-' + language))
            flags = ['-DSTICKS3_UI_TURKISH=1'] if language == 'tr' else []
            run(os.environ.get('CXX', 'c++'), '-std=c++17', '-Wall', '-Wextra', '-Werror',
                '-fsanitize=address,undefined', '-fno-omit-frame-pointer', *flags,
                '-Ifirmware/main', 'tests/test_ui.cpp', 'firmware/main/voice_ui.cpp', '-o', binary)
            run(binary)
    run(sys.executable, 'tools/privacy_check.py')
    run('git', 'diff', '--check')
    run('git', 'diff', '--cached', '--check')


if __name__ == '__main__':
    main()
