# Contributing

Small, reproducible fixes and documented hardware observations are welcome. For a larger change, an issue explaining the use case helps align scope. Keep behavior changes focused and avoid unrelated cleanup or dependency upgrades.

## Local checks

From the repository root, with `python3` pointing to Python 3.12:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install --require-hashes -r requirements-dev.txt
.venv/bin/python tools/check.py
. "$IDF_PATH/export.sh"
idf.py -C firmware build
```

Host checks need a C++17 compiler with AddressSanitizer and UndefinedBehaviorSanitizer. For the first firmware build, run `idf.py -C firmware set-target esp32s3` first. Check both UI languages when changing layout or text. Include editable asset sources and regenerated outputs when changing graphics.

Describe what changed and what you actually tested. A successful build does not replace physical testing. For USB, audio, button, or power changes, report board, OS/application versions, release/timeout behavior, and observed results when hardware is available. Mark missing hardware validation explicitly.

## Public contributions

Use placeholders for ports and local directories. Never include recordings, full-flash backups, credentials, SSIDs, device identifiers, private logs, or personal setup files. Preserve third-party licenses and avoid product mascots or artwork without a compatible license. Report vulnerabilities through [the security process](SECURITY.md).

Contributions are made under the project's MIT license, except separately identified third-party material under its original license.
