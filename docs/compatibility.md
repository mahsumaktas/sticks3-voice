# Compatibility

Target hardware is **M5Stack StickS3 (ESP32-S3)**. Other Stick boards are not assumed compatible.

## Evidence matrix

| Area | Evidence | Public v0.1.0 status |
| --- | --- | --- |
| macOS USB microphone | Internal firmware enumerated as mono 48 kHz input | Physical retest pending |
| Claude Code on macOS | Internal firmware had user-confirmed dictation | End-to-end retest pending |
| Codex CLI on macOS | Internal firmware had user-confirmed voice conversation | End-to-end retest pending |
| Windows / Linux | Standard USB interfaces are used | Untested |
| Public UI and USB identity | Changed for public release | Display and enumeration checks pending |

Historical checks did not establish a comprehensive OS/version matrix. Builds and host tests cannot verify electrical behavior, audio quality, microphone selection, keyboard focus, or cloud availability.

## Host applications

**Claude Code:** intended for hold-to-talk dictation using Space after `/voice hold`. Consult the [official voice dictation documentation](https://code.claude.com/docs/en/voice-dictation) for installed-version requirements. The device does not press Enter; however, the host's `voice.autoSubmit` setting can submit on release. Disable it for review-before-send behavior. Hold detection requires terminal key-repeat events.

**Codex CLI:** F8 is based on the [0.159.2 keymap](https://github.com/openai/codex/blob/rust-v0.159.2/codex-rs/tui/src/keymap.rs) and [realtime implementation](https://github.com/openai/codex/blob/rust-v0.159.2/codex-rs/tui/src/chatwidget/realtime.rs). This is a version-specific integration, not a guarantee for all versions. Confirm availability and mapping in your installation. Codex mode is voice conversation, not draft-only transcription.

Both applications need microphone permissions, service access, and their supported account configuration. The firmware does not bypass these requirements. These mappings do not claim support for desktop apps, browser tabs, or every terminal emulator.

## Focus and input

Keyboard reports go to the foreground application. Microphone selection is separate: any permitted application opening this input can receive gated audio. The device cannot identify sessions, select an input, switch windows, or observe whether F8 succeeded.

After manual host toggles, check and realign both sides. USB connection, a moving meter, and the selected mode do not establish that a voice session is ready.
