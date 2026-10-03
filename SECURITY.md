# Security

Report vulnerabilities through [GitHub private vulnerability reporting](https://github.com/mahsumaktas/sticks3-voice/security/advisories/new). If unavailable, open a minimal issue requesting a private reporting channel without exploit details or private data.

Include revision, reproduction steps, expected and actual behavior, and impact. Do not send flash backups, credentials, private audio, or unredacted device logs.

This initial release has no guaranteed response time or long-term support window. Fixes target the current development/release line; older versions should not be assumed to receive backports.

Relevant boundaries include the audio gate, USB keyboard reports, USB descriptors, and flashing/backup tools. The device cannot enforce host microphone permissions or third-party voice-service privacy rules. See [privacy](docs/privacy.md).
