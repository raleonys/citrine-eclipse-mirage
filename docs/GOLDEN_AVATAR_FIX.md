> Tarihsel snapshot: hook11/lifecycle ve metadata için son düzeltme
> [Bluefield uyumu](HOOK_ALIGNMENT.md) ve [Brownfield metadata denetimi](REFERENCE_METADATA_AUDIT.md) içindedir.

# Avatar, import ve Golden düzeltmesi

Görev: YF-GOLDEN-AVATAR-20261029. Önceki paket
`yellowfield-msvc-native-lookup-fix-source.zip` değiştirilmeden korunur.

Kullanıcının açık seçimi: “Yalnız PE resource olmasın; mevcut Anadius avatar
verisinin yerine geçsin”. PNG RCDATA101 kaldırıldı. Aynı 3653-byte SURGE
PNG build sırasında private read-only C++ diziye dönüştürülür ve RE'deki
`0x1800809d0` profil-avatar verisinin yerini alır. Runtime için haricî PNG
gerekmez; mevcut config avatarı veya provider görseli ezilmez. Kaynakta PNG
bulunması build girdisi içindir.

Ayrı `0x180082710` varsayılan PNG aynı görselle değiştirilmez. 3882-byte özgün
fallback doğrudan verilen `anadius-rea-evidence.json` bellek verisinden
çıkarılmıştır; bütün PNG chunk CRC'leri doğrulanmıştır. Brownfield/Bluefield
uygulama kodu alınmamıştır. Hash/adres/kanıt kaydı:
[AVATAR_DATA_CONTRACT.json](../metadata/AVATAR_DATA_CONTRACT.json).
`preloader_version.rc` byte'ları ve VERSIONINFO profili değişmedi.

`OLE32.dll` export değildir; önceki adaptörün `SHGetKnownFolderPath` ve
`CoTaskMemFree` çağrılarından gelen importtu. RE `FUN_180039bc0`, 260 UTF16
birimlik artışlarla `GetEnvironmentVariableW` çağırır. `LocalAppData` ve
`ProgramData` bu özgün davranışa döndürüldü; `ole32` link girdisi kaldırıldı.
DLL'nin dışarı açtığı beş C fonksiyonunun adı/ordinal'i değişmedi.

Son Native kullanıcı logu önceki `unexpected()` hatasını geçmiş, encoding
birim testinde `std::min` için C2589 vermiştir. Windows `min` makrosunu önleyen
NOMINMAX, object library'de vardı fakat `$<TARGET_OBJECTS>` üzerinden core
consumer'larına aktarılmıyordu. `preloader_l_core PUBLIC NOMINMAX` bu yayılımı
düzeltir. Vendor kaynaklarına müdahale edilmez; testler atılmaz.

Kullanıcının önceki onayıyla dört eksik Golden hedefi de tamamlanmıştır.
Derleyici kimliği korunur; yalnız checksum kopyada tamamlanır. Sabit
fixture timestamp/linker değerleri ürün kuralı yapılmaz.
[Golden ürün hedefleri](GOLDEN_PROTOCOLS.md).

Bu turdaki test ve gerçek binary gözlemleri
[GOLDEN_AVATAR_FIX.json](../metadata/evidence/GOLDEN_AVATAR_FIX.json) içinde tutulur.
LLVM/Wine testleri, gerçek Windows/MSVC/oyun kabulü yerine geçmez. Yeni
kaynakla Native ve Golden profilleri Windows'ta yeniden çalıştırılmalıdır.
Yeni kaynak ZIP'i yanında standart `hash  dosya-adı` formatında SHA256
kontrol dosyası otomatik üretilir.

Yerel sonuç: Release173/173, Debug etkilenen5/5 test geçti. Kaynak QA15/15
geçti. Her iki gerçek LLVM DLL'de yalnız VERSIONINFO resource'u var; iki
avatar read-only `.rdata` içinde ve OLE32 importu yok. Beş C export korundu.
Bu sonuçlar gerçek MSVC Golden veya oyun kabulü değildir.
