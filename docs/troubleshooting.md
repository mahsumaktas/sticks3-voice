# Troubleshooting

| Symptom | Check |
| --- | --- |
| No USB device | Use a data cable and direct port; distinguish normal firmware from bootloader enumeration. |
| No flashing port | Enter manual bootloader mode, then run `.venv/bin/python tools/device.py list` again. |
| Missing esptool | Install the pinned `requirements.txt` into the separate tool environment and use its Python. |
| Backup/verification failure | Stop. Check power, cable, free space, current port, and exclusive port access. |
| Black screen after flash | Confirm verification passed; restart normally if still in download mode. |
| Meter moves but no host audio | Select StickS3 Microphone, grant permission, reopen voice mode, and hold A. |
| Silence after a long press | Release A and press again; the default limit is 90 seconds. |
| Claude receives spaces | Focus its conversation, enable hold-to-talk, and check the version's mapping. |
| B affects another window | Focus Codex before toggling; USB keys go to the foreground window. |
| Device says Codex, host voice closed | Manual F8 or a missed toggle can desynchronize states. Check both sides. |
| Device speaker is silent | No USB speaker output is provided; select the computer's output. |
| Accessory has no 5 V | External boost power is intentionally disabled. |
| Unexpected bootloader entry | Avoid CDC clients that open at 1200 baud; use 115200 for diagnostics. |

Recover with your own verified full-flash backup using the [restore instructions](getting-started.md#restore). Do not use another person's flash dump.

Reports should include revision, build result, board, OS/application versions, and reproduction steps. Distinguish device display from host state. Remove identifiers and personal paths; do not attach dumps or private audio. See [contributing](../CONTRIBUTING.md).
