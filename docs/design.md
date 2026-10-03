# LCD design and asset provenance

The 240 × 135 RGB565 screen uses one C++17 renderer on the device and in the host preview tool. The cream Claude mode and dark teal Codex mode share the same layout: mode and USB status at the top, character and capture status in the center, physical-button hints at the bottom. Mode names describe the intended host workflow; the screen does not verify login, model activity, or transcription success.

Sprig, the seedling, and Orbit, the floating capsule, are original characters constructed from ellipses and rounded rectangles in `firmware/main/voice_ui.cpp`. No vendor mascot, terminal silhouette, pet sprite sheet, or other vendor artwork is redistributed. Product names identify integrations and do not imply affiliation or endorsement.

Nunito is the only bundled font. Its source is `assets/fonts/Nunito.ttf`, with the accompanying SIL Open Font License in `assets/fonts/OFL.txt`. The generator rasterizes fixed labels and timer digits into grayscale coverage masks. It performs no downloads. Only the selected language is compiled into firmware; no runtime font engine or heap allocation is needed per frame.

English is the default. Enable `CONFIG_STICKS3_UI_TURKISH` in firmware configuration to select Turkish. Host tools use `-DSTICKS3_UI_TURKISH=1`. The checked-in generated header holds conditional masks for both languages.

## What the screen means

| State | Meaning |
| --- | --- |
| Ready / Hello there | USB is mounted and the device is waiting for a button press. |
| Listening / Mic streaming | The capture button is held and the host is requesting microphone data. This is not a claim that an AI service received it. |
| Open voice | The button is held but host microphone streaming has not started. |
| All done / Mic muted | The button was released. This confirms local capture gating, not successful transcription or delivery. |
| Let's connect | USB is not mounted. |
| Take a breath | The capture time limit was reached. Release the button before trying again. |
| Audio stopped | The firmware detected an audio failure. |

The waveform uses measured PCM peak levels, clamped to 24,000 and square-root scaled. Sixteen samples form a short scrolling history, with an envelope decay of 0.76 per rendered frame. Silence produces baseline bars after the previous envelope/history decays; there is no synthetic audio waveform. Leaving the speaking state resets the history. The timer caps at 90 seconds.

“Mic muted” describes the USB output gate: outside active capture the firmware supplies silence. It does not promise an electrically powered-off microphone, an operating-system privacy permission change, or that the host closed its audio stream. The USB pill reports mount state only. Character bobbing and blinking are decorative and never indicate network or AI activity.

## Reproducing and checking previews

With the pinned development dependencies installed:

```sh
.venv/bin/python tools/build_assets.py
.venv/bin/python tools/render_previews.py
```

The second command compiles `tools/preview.cpp` together with the actual firmware renderer in both languages. It renders all seven states for both themes, converts temporary PPM framebuffers to PNGs, and builds contact sheets plus `assets/previews/hero.png`. The hero is two real ready-state framebuffers enlarged by nearest-neighbor scaling, not a hardware photograph or a fictional mockup. PNGs contain no EXIF, user paths, serial numbers, or embedded personal metadata. Temporary executables and PPMs stay outside the repository.

Font rasterization explicitly selects Pillow's BASIC layout engine. Optional RAQM availability must not change kerning or the generated masks between operating systems.

Use `tools/render_previews.py --check` to compare regenerated pixels without rewriting the committed PNGs. PNG compression libraries may encode identical pixels differently across operating systems; verification compares dimensions and every decoded RGB pixel.

The asset generator also checks label dimensions against their layout regions and checks seven text/background pairs in each actual source palette against a 4.5:1 contrast threshold. Measurements are recorded in `assets/previews/label-metrics.json`. These are source-color checks, not a claim of whole-device accessibility certification; antialiasing, RGB565 quantization, panel brightness, and physical viewing conditions affect the result.

`tests/test_ui.cpp` exercises all states and both themes under host sanitizers. It checks sentinel bounds, zero-input stability, different measured levels, reset on leaving capture, peak saturation, timer saturation, and USB status changes. These checks validate renderer behavior; hardware and host audio compatibility need separate testing.
