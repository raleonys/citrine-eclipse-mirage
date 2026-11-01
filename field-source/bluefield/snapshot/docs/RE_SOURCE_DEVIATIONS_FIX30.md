# FIX30 — Geçici dosya adı tükenmesinde kaynak hata dalı

NON_RELEASE tam kaynak/build/QA devam paketi; exact FIX29 parent. Karşılaştırma kullanıcının RE kaynak paketidir; özgün DLL binary eşdeğerliği değildir. D14/D21 altında tek somut alt hata düzeltilir.7 ana inceleme açık kalır; sayı yapay olarak düşürülmez.

## Sapma nereden geldi?

Failure-atomic persistence kullanıcı talebi olduğu için doğrudan fstream yerine exclusive sibling staging eklenmişti.100 ad çakışması sonrası atomic temporary quota adlı genel runtime_error bizim entegrasyon seçimimizdi; upstream gerektirmiyordu. FIX27 initial system_erroru source failbit2ye çevirdi, fakat collision bütçesini non-system failure olarak özellikle dışarıda bıraktı. Bu önceki kapsam tercihi kullanıcı talimatı değildi ve kaynak initial-open sözleşmesini tam karşılamıyordu.

RE binary caller, output initial_state sıfır değilse payload/close çağırmadan false döndürür. Gerçek yazıcı routeu Application → LicenseFiles → write_binary → NativeOutputtur. Collision exhaustion stage constructorında, payload başlamadan olur; tüm exclusive create girişimleri dosya-var hatasıdır. Generic runtime_error NativeOutputun dar system_error catchini atlıyor, source false/dialog akışı yerine genel failure yoluna gidiyordu. Sorun100 sayısını ürün kotası diye otomatik silmekle çözülmedi.

## En küçük düzeltme

Yalnız exhausted-name throw ifadesi system_error(file_exists) oldu.100 deneme, isim şeması, global publication mutexi, cleanup/flush/publish ve failure-atomic davranışı aynı. NativeOutput, source binary/license callerlar ve Windows adapteri değiştirilmedi. Catch genişletilmedi. Allocation/recovery/owner ve post-open write/flush/commit hataları exception kalır. Bu bir initial setup error-class düzeltmesidir; process-vs-session/CRT veya unload politikası kararı değildir.

Önceki ticket testi collisionı non-IO error sayıp propagate bekliyordu. Bu app expectationı source davranışının kanıtı değildi. Collision şimdi ordinary open-error ile aynı source false/dialog matrisiyle sınanır; diğer exception testleri korunur. Beklenen sonuç literal bağımsız RE binary callerdan ve değişmemiş caller bodylarından gelir.

## Doğrulama kapsamı

Yeni test gerçek temp dosyaları oluşturur; syscall mocku gerektirmez. Üç100-collision setup dalı direct writer error condition, NativeOutput failbit2 ve source false/trace sonuçlarını kontrol eder.99 çakışma sonrası100üncü denemede başarı ve commit birlikte doğrulanır;399 preexisting dosyanın hiçbir byteı değişmez. Literal packet binary_io.hpp/cpp yalnız include/namespace uyarlamasıyla ayrı test types/algoritması olarak derlenir; source kodu production graphına girmez.

Runtime123 parent dosyasının122si aynı; tek existing atomic_io.cppde tek throw değişir. Vendor1060, bütün önceki CMD/PS1/Golden araçları, profil/CFG/Asio/XML algoritmaları ve diğer politikalar korunur. Önceki61 C++/header fixtureın60ı aynı; yalnız açıklanan ticket expectationı düzeltilir.

Full Debug/Production, exact ZIP replay, negatifler ve focused sanitizer sonuçları teslimdeki final QAya bağlanır. Windows/MSVC/native/CRT/game/actual DLL Golden, full-core sanitizer ve LSan NOT_RUN. D14 process/session/CRT; D21 diğer iç bütçeler; DEP05/D02/D06/D07/D13 incelemeleri hâlâ açıktır.
