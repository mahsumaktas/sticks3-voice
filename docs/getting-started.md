# Getting started

## Prepare and build

Use an **M5Stack StickS3**, USB data cable, Python **3.12** for host tools, and ESP-IDF **5.4**. Other M5Stick models are not interchangeable targets. Follow the [ESP-IDF setup guide](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32s3/get-started/index.html) and [StickS3 hardware documentation](https://docs.m5stack.com/en/core/StickS3).

Clone or download this repository. Commands run from its root. `PORT` and `BACKUP.bin` are placeholders.

```sh
. "$IDF_PATH/export.sh"
idf.py -C firmware set-target esp32s3
idf.py -C firmware menuconfig
idf.py -C firmware build
```

Menuconfig is optional. Project settings provide UI language, keyboard usages, and maximum hold duration. Defaults: English, Space (USB HID usage 44), F8 (usage 65), and 90 seconds. These are USB keyboard codes, not ASCII. Keep the dependency lockfile.

## Back up and flash

Use a separate Python 3.12 tool environment with pinned esptool 5.4.0. In the commands below `python3` must point to Python 3.12; use `python3.12` explicitly if needed. Do not replace ESP-IDF's own Python dependencies. Commands use POSIX paths; on Windows use `.venv\Scripts\python.exe` in place of `.venv/bin/python` and build in the ESP-IDF command prompt.

```sh
python3 -m venv .venv
.venv/bin/python -m pip install --require-hashes -r requirements.txt
.venv/bin/python tools/device.py --help
.venv/bin/python tools/device.py list
```

Enter manual download/bootloader mode using the power/reset control, following the manufacturer's instructions. The earlier hardware workflow held this control until the green light flashed. Re-list ports afterward because the port can change. Do not confuse this control with mode button B.

```sh
.venv/bin/python tools/device.py flash --port PORT
# Optional explicit directories:
.venv/bin/python tools/device.py flash --port PORT --build-dir firmware/build --backup-dir backups
```

The helper backs up full flash and verifies it before writing by default, then verifies the flashed images. Retain the backup and metadata. A failed backup or verification is a reason to stop and investigate. `--no-backup` deliberately skips this protection; use it only with a verified recovery image already available.

Full flash may contain credentials, application data, and device identifiers. Never attach a backup to an issue or commit it. No factory firmware image is distributed here.

## Select microphone and host workflow

1. Select **StickS3 Microphone** in the computer's audio input settings and, where available, the application's input selector.
2. Grant microphone access to the terminal/application as required by your OS.
3. Open the intended conversation and keep that terminal in the foreground.
4. For Claude Code, enable `/voice hold`, hold A, wait for the application's listening indication, then speak and release A. Disable the host's `voice.autoSubmit` setting if you want to review the text and submit it yourself.
5. For Codex CLI with the documented F8 mapping, press B while Codex is focused. Wait for the application's voice connection, then hold A to talk. Responses play through the computer's selected output.
6. To return to Claude, keep Codex focused and press B to send the closing toggle before changing terminals.

The USB indicator does not confirm a cloud connection. The device cannot fix the wrong input selection or foreground window. See [compatibility](compatibility.md).

On macOS, the optional one-shot CoreAudio helper selects the device:

```sh
mkdir -p .local
swiftc tools/audio_input.swift -o .local/audio-input
.local/audio-input --select-sticks3
```

It does not need to remain running. If an application retains an old input, reopen its voice mode after selecting the microphone.

## Restore

Use a verified full-flash backup from **your own device**, together with the adjacent `.json` manifest produced by the helper. Restore checks the file size and SHA-256 against that manifest before accessing the device. Enter manual bootloader mode again and select its current port:

```sh
.venv/bin/python tools/device.py restore --port PORT --backup BACKUP.bin
```

Restore overwrites flash. Keep power and USB stable through write and verification. If the device stays in its loader afterward, restart normally. A missing or damaged backup requires a suitable firmware image from the vendor; this repository cannot reconstruct previous user data.

## Acceptance check

Before relying on a new build, check enumeration, silent samples with A released, audible capture with A held, release and timeout behavior, shortcuts in the correct foreground application, and both voice workflows. Record OS/application versions without publishing private audio or device identifiers.
