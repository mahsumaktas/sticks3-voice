# StickS3 Voice

**M5Stack StickS3** için cep boyutunda USB mikrofon ve bas-konuş kontrolü. Claude Code ve Codex CLI ses akışları için tasarlanmıştır.

[English](README.md) · [Kurulum](docs/getting-started.md) · [Uyumluluk](docs/compatibility.md) · [Gizlilik](docs/privacy.md)

![İki modun yazılımla oluşturulmuş ekran önizlemesi](assets/previews/hero.png)

Öndeki düğmeye basılı tutarak mikrofon sesini iletin. Yan düğmeyle Claude diktesi ve Codex sesli sohbeti arasında geçin. Ekranda seçili mod, mikrofon seviyesi, basılı tutma süresi ve özgün karakterler görünür. Arayüz İngilizce veya Türkçe derlenebilir.

- Standart USB ses: **48 kHz, tek kanal, 16 bit PCM**.
- USB klavye kısayolları; sürekli çalışan yardımcı uygulama veya özel sunucu gerekmez.
- Ön düğme bırakıldığında sıfır ses örnekleri gönderilir; varsayılan 90 saniyelik sınır yapılandırılabilir.
- ESP-IDF 5.4 tabanlı firmware; bileşen sürümleri sabittir.
- Yükleme aracı varsayılan olarak yedek alır ve doğrulama yapar.

Cihazda yapay zekâ modeli çalışmaz. Claude Code veya Codex CLI'nin ses özelliği ve gerekli hesap/hizmet erişimi gerekir. Cihaza API anahtarı girilmez; bilgisayardaki uygulamanın oturum ve hizmet gereksinimleri devam eder.

## Düğmeler

| Kontrol | Claude modu | Codex modu |
| --- | --- | --- |
| Öndeki **A** düğmesini basılı tut | Space basılı tutulur, mikrofon sesi iletilir | Mikrofon sesi iletilir |
| A'yı bırak | Space bırakılır, sessizlik iletilir | Sessizlik iletilir; sohbeti bilgisayar yönetir |
| Yandaki **B** düğmesine bas | F8 gönderilir, Codex modu seçilir | F8 gönderilir, Claude moduna dönülür |

Açılış modu Claude'dur. Konuşurken mod değişmez. B, güç/reset düğmesi değildir. Önce doğru terminali ve sohbeti seçin: tuşlar ön plandaki uygulamaya gider. Cihaz açık oturumları bulamaz, uygulamayı öne getiremez veya ses modunun açıldığını doğrulayamaz.

Claude'da önce basılı tutarak dikte modunu etkinleştirin. Metni göndermeden önce incelemek istiyorsanız uygulamanın otomatik gönderim ayarını kapatın. Codex için F8 eşlemesi 0.159.2 sürümüne dayanır; kurulu sürümünüzü kontrol edin. F8'i ayrıca elle kullanırsanız cihazdaki mod ile bilgisayardaki ses oturumu farklılaşabilir. Ayrıntılar: [uyumluluk](docs/compatibility.md).

## Derleme ve yükleme

StickS3, veri taşıyan USB kablosu, yardımcı araçlar için Python 3.12 ve ESP-IDF 5.4 gerekir. `python3` Python 3.12'yi gösterecek şekilde depo kökünden:

```sh
. "$IDF_PATH/export.sh"
idf.py -C firmware set-target esp32s3
idf.py -C firmware build
python3 -m venv .venv
.venv/bin/python -m pip install --require-hashes -r requirements.txt
.venv/bin/python tools/device.py list
# Cihazı elle yükleme moduna alın, ardından listelenen portu kullanın:
.venv/bin/python tools/device.py flash --port PORT
```

Komutlar POSIX kabuğu içindir; Windows yolları kurulum belgesindedir. Yükleme mevcut firmware'i değiştirir. Tam flash yedeğini özel tutun ve geri dönüş için saklayın. İlk yüklemeden önce [ayrıntılı kurulum ve geri yükleme adımlarını](docs/getting-started.md) okuyun.

## Durum

**v0.1.0 ilk kaynak kod sürümüdür.** Önceki dahili firmware'de macOS mikrofon girişi ve iki ses akışı denenmiştir. Herkese açık sürümün yenilenen arayüzü, USB kimliği ve yapılandırma değişiklikleri için fiziksel cihaz kabulü henüz yapılmamıştır. Windows ve Linux doğrulanmamıştır. Başarılı derleme veya CI sonucu donanım ve uygulama uyumluluğu kanıtı değildir.

[Katkı](CONTRIBUTING.md) · [Mimari](docs/architecture.md) · [Sorun giderme](docs/troubleshooting.md) · [Güvenlik](SECURITY.md) · [Değişiklikler](CHANGELOG.md)

Proje kodu ve özgün çizimler MIT lisanslıdır. Nunito ve üçüncü taraf bileşenlerin kendi lisansları korunur: [LICENSE](LICENSE), [font lisansı](assets/fonts/OFL.txt), [üçüncü taraf bildirimleri](THIRD_PARTY_NOTICES.md).

Bağımsız bir projedir; M5Stack, Anthropic veya OpenAI ile bağlantılı değildir ve bu kuruluşlarca onaylanmamıştır. Ürün adları donanımı ve hedef uygulamaları belirtir.
