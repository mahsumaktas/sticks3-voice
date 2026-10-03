# Releases and verification

## Initial release

v0.1.0 is a **source release**. GitHub provides source archives; build the firmware using [the setup guide](getting-started.md). No full-flash dump, factory image, executable host utility, or prebuilt firmware is distributed. Screenshots are generated from the actual renderer and are not photographs of a flashed board.

The internal predecessor was exercised on StickS3 with macOS, Claude Code, and Codex CLI. The public revision changes assets, configuration, and USB serial generation. It still needs physical acceptance as its own revision. See [the compatibility record](compatibility.md); do not equate CI with device validation.

## Automated checks

- Python tests exercise missing artifacts, failed backup verification, tampered backups, flash layout, and privacy detection.
- C++ renderer tests run in English and Turkish under AddressSanitizer and UndefinedBehaviorSanitizer on Linux and macOS.
- Asset generation checks label bounds and contrast, then compares regenerated assets with the committed outputs on Linux.
- ESP-IDF 5.4 builds both firmware languages with the component lockfile unchanged.
- pip-audit checks the locked Python dependencies for known vulnerabilities. This does not audit every ESP-IDF component.
- Privacy heuristics and Gitleaks check publication files and history. No test sends audio, accesses cloud AI, or flashes a device.

Workflow actions are pinned to commit hashes. CI has read-only repository permissions. Weekly runs and Dependabot proposals help detect changes; updates still need review and device testing where relevant.

## Maintainer release procedure

1. Review the intended diff and third-party notices. Keep backups, local configuration, build outputs, recordings, and private identifiers outside the release.
2. Run the local checks and firmware build. Confirm all CI jobs pass for the exact commit being tagged.
3. Inspect author/committer/tag metadata and all Git history for private material. Scan an exported source archive as well as the working tree.
4. Update the changelog and compatibility record with actual evidence. State missing hardware or operating-system coverage.
5. Tag the reviewed commit and publish release notes that describe the delivered files. For future firmware binaries, include component notices, checksums, provenance, and revision-specific hardware results.
6. Download the published archive again, compare it with the reviewed tree, and verify public visibility and the default branch.

Do not attach an internal backup as a release asset. It can contain data from firmware that predates this project.
