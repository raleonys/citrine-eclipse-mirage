# Üretim bağlantısı ve tanılama temizliği

Sahip, doğrulanan dead-code audit kararlarının TLS tanılama düzeltmesiyle birlikte
uygulanmasını istedi. Dört kaynak dosyası silinmeden üretim bağlantısından
korunan referans/kaynak API grubuna ayrıldı. Güncel bölünme60 üretim ve78 referans
CPP; toplam138. Public header, dört vendor, kaynak API imzaları, üç üretim
export'u ve Golden derleyici/linker politikası korunur.

| Audit maddesi | Uygulama |
|---|---|
| DC-001–004 | config/order.cpp, config/config_startup.cpp, net/session_routing.cpp ve platform/windows_effects.cpp yalnız referans/API/test grubuna taşındı. |
| DC-005 | Etkin CLI'nin kullanmadığı eski load_codemodel Python okuyucusu kaldırıldı; generation-time graph kapısı ve CMake file-API query korundu. |
| DC-006 | Altı dosyada yalnız yedi kullanılmayan import bağlaması kaldırıldı. |
| DC-007 | Avatar testindeki sonucu okunmayan SHA-1 hesabı kaldırıldı; PNG/fallback byte assertion'ları ve hash API korunur. |
| DC-008–011 | Token'ın kanıtlanmış gereksiz boyut alt koşulu, farklı kök dalındaki alias guard, ClassicIso tekrar EOF kontrolü ve QPC özdeş ternary sadeleştirildi. |
| Crypt tanılama | crypt_observed gövdesi NDEBUG dışına alındı; Debug reentry guard ve mevcut debug_log çağrısı duruyor. Virtual method, hook seçimi/orijinal çağrı/LastError ve yaşam süresi sözleşmesi korunur. |

Eski config/startup.cpp dosyası, üretimdeki runtime/startup.cpp ile MSVC MAP'te
aynı startup.obj adına dönüşmemesi için config/config_startup.cpp olarak
adlandırıldı. Dosya gövdesi byte düzeyinde aynı; header ve C++ sembol adları
değişmedi. Bir kaynak gövdesi silinmedi;138 CPP sayısı korunur.

config_hash'ın eski modelle1440 karşılaştırması ayrı referans testine, achievement
storage'ın eski bootstrap bileşimi kendi model testine taşındı. Asıl hash/storage
testleri production-only core üzerinde kaldı. config_order, session integration
ve WindowsEffects model testleri referans olarak sınıflandırıldı; gövdeleri ve
assertion'ları korunur. Portable hash testinden de model bağımlılığı ayrıldı;
ayrı portable differential hedefi eklendi. Üretim testi referans kütüphanesine
bağlanarak geçerli gösterilmez.

DC-012–019 kapsamındaki public temsil/metadata/ownership veya karma TU tasarımları
bu dar temizliğe dahil edilmedi. Özellikle leases_ rezervasyonu, başarısız native
release sonrası retry, belirsiz socket sonucu karantinası, pending IO sayaçları,
buffer/receipt capture'ları ve image lease korunur. Genel unload helper'ı kaynak
API/test köklerinde kalır; checked close onunla değiştirilmez.

Bu TLS koruması önceki kaynakta da vardı. Temizlik, VirusTotal etiketinin nedeni
bulundu veya etiket kalkacak anlamına gelmez. Aynı toolchain ile ölçülen bölüm,
TLS, ham DLL ve geliştirme export farkları son doğrulamada ayrı kaydedilir;
cross-build farkı native MSVC kazancı olarak sunulmaz.

Güncel makine sözleşmesi [BUILD_CLEANUP_REVIEW.json](../metadata/BUILD_CLEANUP_REVIEW.json),
doğrulama [VALIDATION.json](../metadata/evidence/build-cleanup/VALIDATION.json).
Native Windows/MSVC/Golden ve oyun kabulü değişen kaynak için ayrıca gerekir;
eski kabul bu değişikliğe aktarılmaz. Donor emekliliği incelemesi açık kalır.

## Yerel sonuç ve sınırlar

Release205/205, etkilenen Debug23/23 ve portable21/21 test başarılıdır. Kaynak QA
17/17 gruptur; portable/altyapı alt kontrolleri çakıştığından sayılar toplanmaz.
Production/referans MAP sınırı başarılı; kaynak API'nin3346 export adı
önce/sonra birebir aynı. Yeni20 partition regresyonu, dört model nesnesinin
üretim adlı arşivden geri gelmesini ve startup dosya adlarının ayrımını denetler.

Aynı LLVM-MinGW Release ham DLL1.083.392→1.070.080 bayt,13.312 bayt küçüldü.
Farkın6144 baytı PE raw bölümler,7168 baytı bölüm dışı COFF tabloları/dolgudur;
13KiB makine kodu veya MSVC kazancı değildir. .text VirtualSize3600, TLS şablonu8
bayt azaldı; TLS raw bölümü aynı512 baytta kaldı. Tek tek dar değişikliklere
bağımsız hız/boyut kazanımı atanmadı. Gerçek hedefin Release OBJ'sinde tanılama
TLS'i yok; Debug callback talimatları önceki kaynakla aynıdır.

Bu sonuçlar LLVM-MinGW/Wine ve Linux portable kanıtıdır. MSVC/Golden veya oyun
testi çalıştırılmış sayılmaz. Tam ZIP'in temiz açılımından test/API kapalı build
ve üç gerçek DLL döngüsü için paket dışındaki `surge-build-cleanup-delivery-verification.json`
son kaydı kullanılır; derleme/log kanıtları bu kaynak snapshot'ına bağlanır.
