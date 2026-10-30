> Sonraki açık kullanıcı kararı: sınırsız/heap kullanan yol okuması artık
> [HOOK_PATH_SCAN.md](HOOK_PATH_SCAN.md) sözleşmesiyle değiştirilmiştir.
> 32.768 UTF-16 birimi sınırı, sabit suffix tamponu ve korumalı okuma geçerlidir;
> aşağıdaki eski okuyucu/body-identity ifadeleri bu kapsamda tarihsel kayıttır.

# Bluefield 11. hook uyumu

Kullanıcı Bluefield implementation’ını doğru referans seçti. Önceki RE UTF8/UD2
selector tercihi ve bağımsız unload politikası bu yetkiye uygun değildi; kaldırıldı.
Aktarım kapsamı `CryptQueryObject` ve gerekli ortak native hook ömür yönetimidir.
Diğer uygulama modülleri Bluefield’den kopyalanmadı.

| Konu | Güncel davranış / kaynak |
|---|---|
| Selector | Bluefield `crypt_query.cpp`; yalnız include/namespace uyarlaması, normalized body aynı |
| Dosya adı okuma | `ReadProcessMemory` ile UTF16 unit okuma; pointer overflow kontrolü, sabit yol sınırı yok |
| Okunamayan/geçersiz UTF16 | Original nesneye dönüş; UD2/heap UTF8 dönüşümü yok |
| Eşleşme | Case-sensitive `preloader_l.dll` suffix; companion `EAAntiCheat.GameServiceLauncher.dll`; Bluefield null/non-file kuralları |
| Original çağrı | Tüm SDK argümanları/output pointer’ları ve sonuç aynen aktarılır |
| LastError / gözlem | Bluefield sırasıyla incoming/outgoing LastError korunur; thread-local phase gözlemi |
| Eksik trampoline | Bluefield failure log + fail-fast; sessiz uydurma dönüş yok |
| Hedef | Doğrudan SDK `&CryptQueryObject` ve zorunlu statik CRYPT32 importu |
| Aktivasyon | Tüm native hook’lar queue edilir, config kilidi bırakılır, tek apply yapılır |
| Quiescence | Disable, bounded admitted-call bekleme, external thread/register/stack guard, uninitialize |
| Unload guard | Bluefield gövdesi yalnız include adı değiştirilerek; header byte-identical |
| Shutdown caller | Public kapatma thread’i guard taramasından Bluefield gibi hariç tutulur |

Somut uygulama adapterı mevcut Hooks/NativeApi/Binding sınırlarını kullanır;
Bluefield’in tüm application class’ı aynen kopyalanmış değildir. Protected reader
ayrı host sınırına çıkarıldı, ömür temizliği mevcut worker’a bağlandı. Kaynak
hashleri ve tam uyarlamalar `metadata/evidence/HOOK_ALIGNMENT_STATUS.json` içindedir.
SPDX GPL-3.0-only bildirimleri korunur.

Doğrulama gerçek MinHook queue/apply, disable/uninitialize, original-call argümanları,
LastError, geçersiz UTF16, erişilemeyen ve page-boundary yol okuma, canlı external
thread guard ve DLL startup/shutdown/unload testlerini içerir. Tam test kayıtları
aynı status dosyasına bağlıdır. Wine testleri gerçek Windows/MSVC/oyun kabulü
yerine geçmez; oyun içinden çağrı izi sunulmamıştır.

Metadata düzeltmesi ayrı kayıttır: [Brownfield alan tablosu](REFERENCE_METADATA_AUDIT.md).
