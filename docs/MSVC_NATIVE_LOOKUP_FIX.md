# MSVC Native test ad çakışması

29 Ekim 2026 kullanıcı logları önceki düzeltmeleri doğrular:

| Profil | Kullanıcının gerçek Windows/MSVC sonucu |
|---|---|
| Compile | PASS; gerçek DLL ile üç yükleme/socket/profil/kapatma/unload döngüsü |
| Debug | PASS; aynı public DLL runtime kontrolü |
| CRT | PASS;6/6 seçili CRT/public/source-DLL testi |
| Native | Derleme C2668 nedeniyle async receive integration testinde durdu; tam CTest henüz çalışmadı |
| Golden | Derleme ve export/resource kontrolleri tamamlandı; ürün hedefleri eksik, BLOCKED/exit3 |

Kalan Native derleme hatası, MSVC `eh.h` içindeki global `unexpected(void)`
bildirimi ile `receive_test::unexpected()` test yardımcısının aynı anda görünür
olmasından kaynaklanır. `using namespace receive_test` işlevin on iki niteliksiz
çağrısını belirsiz bırakıyordu. Çağrılar açıkça `receive_test::unexpected()`
olarak yazıldı. Fixture'ın exception davranışı ve tüm test senaryoları korundu.

İlgili testte MSVC dışındaki derleyicilere aynı global bildirim eklenir; bunun
tanımı ve çağrısı yoktur. Böylece ileride niteliksiz bir çağrı eklenirse derleme
kontrolü onu MSVC dışında da yakalar. MSVC kendi gerçek CRT bildirimini kullanır.
Bu bildirim yalnız test executable'ındadır; ürün DLL'ine veya exportlarına eklenmez.

Benzer niteliksiz çağrı/deklarasyon içeren18 test dosyası, aynı global CRT
bildirimi zorlanarak derleme kontrolünden geçirilir. Yardımcıyı kendi sınıf veya
namespace'inde tanımlayan diğer dosyalarda çakışma gözlenmemiştir. Eski hatanın
yeniden üretimi, düzeltme ve tarama sonucu ayrı kanıtlardır; gerçek MSVC tekrarının
yerine geçmezler. Ham sonuçlar [MSVC_NATIVE_LOOKUP_FIX.json](../metadata/evidence/MSVC_NATIVE_LOOKUP_FIX.json) içindedir.

Yeni kaynakta yalnız Native profilini tekrar çalıştırmak gerekir:
`scripts/build_orchestration/build_native_acceptance.cmd`.
Uygulama, vendor ve resource byte'ları ile beş export değişmemiştir. Önceki
Compile/Debug/CRT başarıları kullanıcı loglarına aittir; düzeltilmiş tam Native
grubunun Windows sonucu henüz alınmamıştır. Golden ürün hedefleri bu görevde açılmaz.
