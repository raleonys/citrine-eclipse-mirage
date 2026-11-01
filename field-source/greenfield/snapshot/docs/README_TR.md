# GF00 — Yeni preloader_l / SURGE hattı

**Durum:** GREENFIELD / NON_RELEASE_WORK / NOT_RELEASE. Kendi kod tabanımız; canonical HOTFIX19 ve Library `CURRENT_ARTIFACT` dokunulmamış.

## Amaç
MSVC x64 için mevcut shipping export adlarını/ordinal/çağrı sözleşmesini koruyan minimal DLL iskeleti. **Tam ABI veya runtime parity iddiası YOKTUR.** SURGE kasıtlı olarak boştur; `DllShutdown` sıfır sahiplikli GF00'da 0 döndürür. Ürün/oyun uyumluluğu, LSX endpoint, registry/hooks, activation, içerik, entitlement/auth yetenekleri henüz yoktur.

## Windows yerel derleme
Visual Studio x64 Developer Command Prompt içinde `scripts\build_windows_msvc.cmd`.
`cmake` ve NMake gereklidir. Test host `LoadLibraryW`, isim/ordinal GetProcAddress ve `DllShutdown(0)` + `FreeLibrary` denetler. Beklenen sonuç yerel test PASS'dır, release ya da fonksiyonel uyumluluk değildir. **Windows/MSVC testi henüz çalıştırılmadı.**

## Portable doğrulama
Linux/GCC ve Clang: `bash scripts/run_portable_qa.sh`. Kaynak imza/bütünlük kontrolü: `python3 scripts/verify_package.py`. Her test yalnız bağımsız C++ çekirdeğinin sonlanma-idempotence/concurrency davranışını kapsar.

## Tasarımın ilkeleri
- `DllMain` loader-lock altında yalnız no-op; statik CRT kullanan DLL’de DisableThreadLibraryCalls çağrılmaz; işlemler CRT varsayılan girişinden sonra dışarıda yapılır.
- `/ENTRY` override YOK. `DllMain` statik CRT startup tarafından çağrılır.
- Public export .def dosyası tek otoritedir; debug ve private export üretimde yok.
- Birinci aşamada MinHook, TinyXML2, PEGTL, Asio, JSON, registry ve LSX bağımlılığı YOK. Gerektiğinde belirli lisans/provenance dosyalarıyla kontrollü olarak eklenecek.
- Alt sistemler test edilebilir arabirimler üzerinden eklenir; arayüzler dondurulmadan implementasyon kabul edilmez.
- Şu an kanonik sürüm ile iddia edilen tek benzerlik export yüzeyinin şeklidir. CRT, loader, in-memory data layout ve Consumers için tam binary ABI/davranış paritesi **NOT_PROVEN**.

## Önemli sınır
Bu iskeleti gerçek oyun dağıtımı veya mevcut HOTFIX19 yerine kullanmayın. `/MT` ile bağımsız derleme sadece Windows QA sonrasında doğrulanabilir. Yeni hat için resmî semantik sürüm/codename tahsis edilmedi.

## GF02 current state

See docs/GF02_DIRECT_FRAMING_CODEC_TR.md. The exported DLL still has no active LSX runtime. HF19 receive/framing/codec and 6 original tests are adopted; the test ingress is distinct from shipping Asio.
