> Tarihsel aşama kaydı. Güncel FIX13 davranışı ve geri alınan sınırlar için ORIGINAL_DEVIATION_AUDIT_FIX13.md geçerlidir.

# FIX08 — özgün bağımlılık kanıtı ve upstream hizalaması

## Kanıtın gösterdiği

Özgün pakette Asio/IOCP ve PEGTL tanımları zaten vardır. BLUEFIELD'ın blocking
socket/standart thread backend'i, eksik native bağlantılar yerine yazılan yeni
uygulama kararıdır; Anadius'un da blocking-only çalıştığı anlamına gelmez.
Anadius server README'si ve Windows type recovery, asio::detail::win_iocp_io_context
ile diğer Asio sınıflarını kaydeder. Paket, tam async backend/runtime eşdeğerliğinin
tamamlanmadığını da açıkça söyler. Library kimliğinin bilinmesi, tüm lifecycle'ın
çözüldüğü veya güncel upstream sürümünün aynı davranacağı anlamına gelmez.

Windows tao::pegtl::parse_error RTTI ve vdf::parser kural tanıları; Mac sembol
kayıtlarıyla birlikte PEGTL üzerinde kurulu VDF/Config2 gramerini destekler.
VDF grameri ve PEGTL iki alternatif parser değildir. Gramer/action normal gövdeleri
önemli ölçüde kurtarılmıştır; özgün sürüm, exception/unwind ve bütün native/container
semantiği tamamlanmış değildir. BLUEFIELD bu logical grameri kendi kurtarılmış
parser.cpp'siyle çalıştırır; PEGTL engine'i linklemez. Bu tercih uyumluluk sınırıdır.

Mac sembollerinde ghc::filesystem açıkça görülür. Windows type recovery içinde
std::filesystem türleri vardır. Bu kayıtlar, Windows'un hangi ghc/std yollarını
hangi sürümle kullandığını tek başına bütünüyle kapatmaz. gulrak/ghc::filesystem
BROWNFIELD'da vardır; BLUEFIELD'da std::filesystem ve kurtarılmış Windows lexical
path modeli kullanılır. Platform kanıtları birbirinin yerine sayılmaz.

## Bu sürümde yapılan hizalama

| Bağımlılık | FIX07 | FIX08 / HF19 pristine pin |
|---|---|---|
| MinHook | 1.3.4, c3fcafdc10146beb5919319d0683e44e3c30d537 kökenli input compatibility dosyaları | 8af6b4acae5a9388fd742b56fa79ece89d96f823; kullanılan kaynaklar HF19 upstream ağacıyla byte-identical |
| TinyXML2 | 11.0.0 tag tabanı 9148bdf719e997d1f474be6bcc7943881046dba1; yerel depth=100 değişikliği | 8224e427b655b83dae5e2298f1e6919523a78737; HF19 pristine cpp/h/license byte-identical |

TinyXML2 yeni kaynağı yine 11.0.0 raporlar; yeni 11.0.1/12.0.0 etiketi değildir.
MinHook/TinyXML2 dosyalarının commit ve SHA'ları metadata/THIRD_PARTY.json ile
DEPENDENCIES_FIX08.json'da bağlıdır. GitHub main'in bugünkü hali araştırılıp
kendiliğinden çekilmedi; kullanıcı tarafından istenen sabit BROWNFIELD pinleri
kullanıldı. Pristine vendor dosyaları değiştirilmedi. Build-time network yoktur.

BROWNFIELD SAFE-FREEZE, pristine MinHook dışında ayrı build-tree patch'tir.
Upstream eşitlemesi bu patch'i otomatik taşımış sayılmaz; bu sürümde uygulanmadı.

## XML derinlik politikası

HF19 pristine TinyXML2 limiti 500. FIX07 input kaynağında aynı header yerel olarak
100'e indirilmişti. Pristine dosyayı yeniden yamalamak yerine okuyucunun mevcut
başarılı-document depth=100 politikası owned XML adapter'ına taşındı. Visitor,
OPEN element'leri sayar, self-closing elementi ayrı ParseDeep girişi saymaz.
Başarılı XML için eski açık/kapalı element sınırları regresyondadır. Limit aşımında
18/has_error ve boş görünür document döner; diagnostic açıkça adapter sahibidir.

Bu post-parse kontrolü, özgün parser'ın erken durma allocation footprint'ini
kopyalamaz. Çok derin bozuk XML'in hata önceliği/metni ve tam orijinal native
semantiği iddia edilmez. Bu sınırlar yeni upstream'in entegrasyon kanıtında açık
kalır. Normal geçerli LSX/profil/achievement testleri ayrıca çalıştırılır.

## Ayrı kalması gereken geliştirmeler

Asio/IOCP, ghc filesystem veya PEGTL'yi sadece vendor klasörüne koymak entegrasyon
değildir. Taşıma için read/write completion, zero-progress, FIFO/event sırası,
cancellation, callback lifetime, stop/drain/unload; path için encoding/relative-path
ve error/status; parser için byte grammar, duplicate subtree replacement, BOM,
rewind/action ve hash traversal sözleşmeleri karşılaştırılmalıdır. Mevcut çalışan
FIX07 baseline'ı bu deneylerden ayrı korunur. Yeni config seçenekleri uydurulmaz.

Önceki karşılaştırmadaki CMP05/CMP06 allocation hataları ve persistence/deadline
adayları ayrı reliability işidir; dependency değişikliğine sessizce eklenmedi.
