# Privacy

The firmware is a USB microphone and keyboard controller. It implements no AI service, Wi-Fi connection, cloud backend, transcript store, or recording-file workflow. No API key is stored on the device by this project.

## Audio path

Holding A within the configured time limit sends microphone samples to the USB host. Releasing A or reaching the limit sends zero samples. The gate is software, not a physical microphone disconnect. The level meter and processing operate on device audio; the gate does not establish hardware isolation.

The OS and applications control microphone access. Any permitted application opening this input can receive audio while the gate is open. Foreground keyboard focus does not limit which process can record.

Claude Code and Codex CLI use their own voice services. Their service/account settings govern transmitted audio; this is not offline AI. The device cannot verify upload, retention, deletion, or which account receives audio.

## Logs and backups

The USB serial descriptor is derived from each board's factory identity at runtime. The host can see it locally. No particular board's serial number is embedded in the published source. USB product/vendor identifiers describe the firmware interface and are not secrets.

Diagnostics describe device state, buttons, and audio activity rather than transcripts. Inspect logs before sharing: OS and flashing tools can include serial numbers, USB identifiers, local paths, or other environment information.

Full-flash backups may preserve credentials, previous application data, and device identifiers. Keep them private. Do not upload dumps, private recordings, unredacted logs, or screenshots with account information to public issues.

## Publication checks

`tools/privacy_check.py` inspects Git-listed files for private paths, hardware addresses, private-network addresses, recognizable credentials, forbidden local artifacts, and PNG metadata. CI also runs Gitleaks against source and complete Git history. These are complementary heuristics; review newly added content as well. Ignoring or deleting a file does not remove it from existing Git history.
