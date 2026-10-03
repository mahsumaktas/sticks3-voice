# Third-party notices

The root MIT license covers original project code, documentation, and artwork. It does not replace the licenses of dependencies or bundled fonts. Product names are descriptive; no affiliation or endorsement is claimed.

| Material | Version / source | License |
| --- | --- | --- |
| Nunito font | Bundled `assets/fonts/Nunito.ttf` | SIL Open Font License 1.1; complete notice in [OFL.txt](assets/fonts/OFL.txt) |
| M5Unified | 0.2.25; [upstream](https://github.com/m5stack/M5Unified) | MIT; Copyright (c) 2021 M5Stack |
| M5GFX | 0.2.32; [upstream](https://github.com/m5stack/M5GFX) | MIT; Copyright (c) 2021 M5Stack; internal third-party files retain their notices |
| Espressif USB Device UAC | 1.3.1; [upstream](https://github.com/espressif/esp-iot-solution/tree/master/components/usb/usb_device_uac) | Apache-2.0; SPDX copyright 2023–2026 Espressif Systems (Shanghai) CO LTD |
| TinyUSB | 0.19.0~3; [upstream](https://github.com/hathach/tinyusb) | MIT; Copyright (c) 2018, hathach (tinyusb.org); internal third-party files retain their notices |
| ESP-IDF | 5.4 series; [upstream](https://github.com/espressif/esp-idf/tree/release/v5.4) | Apache-2.0 unless otherwise marked; bundled components may use other licenses |

Dependency versions are recorded in [the lockfile](firmware/dependencies.lock). ESP-IDF downloads managed components during the build; they are not original project code. Keep their license files and per-file notices when redistributing them. Binary distributors must preserve the licenses and notices applicable to the components included in their build, including the complete Apache-2.0 license and any required upstream NOTICE text.

The complete [Apache-2.0 license](LICENSES/Apache-2.0.txt) is included for reference. The initial release distributes project source and artwork; it does not redistribute a toolchain, managed component tree, or prebuilt firmware.

No third-party product mascot artwork is included in the public project. The original project mascots are separate from the names of the host applications.

## MIT dependency license text

The M5Stack and TinyUSB copyright notices above accompany this permission:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
