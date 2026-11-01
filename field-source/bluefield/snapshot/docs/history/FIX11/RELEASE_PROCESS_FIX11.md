# Release ve Golden

Bu bir tam uygulama kaynak snapshot'ıdır; kimliği BF02 FIX11, durumu NON_RELEASE_WORK /
NOT_RELEASE'dir. BF01 yanlış kapsamlı ve kullanıcı tarafından reddedilmiş olarak
kalır; bu çalışma doğrudan aynı exact kullanıcı kod paketinden türetilmiştir.

Yerel Production profili beş Golden kapısı sonrası LOCAL_BUILD üretir; resmî release kabulü
ilan etmez. BROWNFIELD'ın production gereksinim sırası korunur:

MSVC → URSYNEX Golden Finalizer → ARYRA Golden Layout → MUSTAVEX Golden Metadata
→ CORVEXON Golden Trail → FALCION Golden Cleaning.

Bu snapshot'ta beş gerçek Golden aracı production yoluna bağlanmıştır. Sentetik
bağlantı testi ve araç selftestleri native DLL kabulünden ayrıdır. Yeni Windows
MSVC/NMake derlemesi ve o DLL'in beş aşamalı sonucu bu ortamda NOT_RUN'dır.

Resmî release için ayrıca identity-specific yetki, güncel normatif mirror ve
sınıfa göre G0–G4 uygulanır. Yerel compile/debug/test bundan bağımsızdır.
Zorunlu oyun kabulü yalnız FINAL_RELEASE için geçerlidir. Bu çalışma hiçbir
BROWNFIELD/current release işaretçisini değiştirmez.

FIX01 tam, kümülatif kaynak snapshot'ıdır. Exact BF02 parent ZIP SHA-256:
`cc2b660f0b6640c36c858b41059cc09d86760d99a73ac8cfbe44930eabf0b65a`.
Kullanıcının MSVC logu build başarısı değildir: core/MinHook static library
oluşmuş, native DLL translation unit'i `sockaddr_in6` eksikliğinde durmuştur.
FIX01 için MSVC compile/link ve Native/CRT gate'leri tekrar çalıştırılmalıdır.

FIX02 snapshot parent: BF02 FIX01 ZIP
`7976882bc6f32b1573181641d64dc0985a635a08ba92d38d0a16b71fabf52b56`.
Kullanıcı FIX01 ile oyunun açılmadığını bildirdi. Bu bildirim Windows runtime
logu veya crash dump'ıyla henüz eşleştirilmedi; FIX02 hook eklenmesi oyun PASS
anlamına gelmez. Native/MSVC ve Consumer sonucu ayrıca bekler.

FIX03 parent: BF02 FIX02 ZIP `8cd4aab5e9402ec9613d2545c7d062094ed432a865b131d9c9706c9801fb43a0`.
Bu sürüm kurulum kilit sırası riskini düzeltir ve kanıt toplama boşluğunu kapatır;
kullanıcının oyun açılmama problemi Windows'ta kapanmış sayılmaz.

FIX04 parent: BF02 FIX03 ZIP `4bab6dc304a2e0fea1e6b073ec972b71b859293afbf89d0ed8f18cea27322943`.
Config/profil/asset düzeltmesi yerel kaynak doğrulamasıdır; Windows ve oyun kabulü
ayrı kalır. Tam Golden araçları bu değişiklikle tamamlanmış sayılmaz.

FIX05 exact parent: BF02 FIX04 ZIP `fa72de59df22f40ee010929caa434f5b84069d53c25d1ae963c7413919678904`.
Scoped test/Golden-tool selftests are not new-DLL Golden or complete HF19 acceptance.

## FIX06 üretim bağı

Exact parent FIX05 ZIP SHA-256:
`c5e846d03994a27f870013dfbdb1446a8514933c25f4d81589386a3eb4d8b5f9`.
Production girişindeki MSVC sonrası run_golden.py gerçek beş aracı çalıştırır.
Finalizer staging kopyası dışında yazmaz; sonraki dört gate read-only'dir.
Her gate sonucu ve hash build/reports/golden_production.json'a kaydolur.
Hata yeni DLL publication'ını durdurur. FALCION yeni source layout'una LOCAL_BUILD
kapsamında uyarlanmıştır; source manifestini değiştirmez. Resmî release G0–G4,
oyun kabulü ve canonical release pointer bu yerel kaynak çalışmasının dışındadır.

## FIX07 güncel durum

Kullanıcı son sürümde surge.cfg olmadan başlangıç ve profilin çalıştığını bildirdi.
Önceki no-config OPEN bulgusu USER_CONFIRMED düzeyinde kapanır; exact DLL SHA ve
Golden receipt sağlanmadı. FIX07 varsayılan adı SURGE olur, sayısal ID'ler korunur.
Eski CFG'nin exact etkisi ve düzeltmenin 14 alanlı profile eşitliği yeni entegrasyon
testindedir. Kapsam 26 C++ + 3 tool grubudur. FIX07 native/Windows/DLL Golden
NOT_RUN; eski CFG ile tüketicide anadius görülmesinin exact nedeni açık kalır.
Ayrıntı PROFILE_CONFIG_FIX07.md içindedir.

## FIX08 dependency notu

FIX07 kullanıcı tarafından çalışır olarak doğrulandı. FIX08 exact HF19 pristine
MinHook/TinyXML2 pinlerine geçer. Asio/IOCP ve PEGTL/VDF özgün kanıtları ile
BLUEFIELD backend seçimleri ayrı şeylerdir; DEPENDENCIES_FIX08.md güncel açıklamadır.
Önceki XML depth=100 vendor uyarlaması owned reader'a taşınır. Derin bozuk XML
hata sırası/footprint ve Windows native eşdeğerliği açık sınırdır.

## FIX09 Asio

Aktif gerçek TCP yolu Asio'ya taşındı; Windows IOCP source guard'ı vardır. Yeni
Asio integration grubu ile toplam 27 C++ + 3 tool grubu bulunur. Frame/profile/
crypto/persistence regresyonları aynı uygulama kodunu kullanır. Allocation failure,
large-write, FIFO, cap ve stop/lifetime testleri gerçek loopback üzerinden çalışır.
Portable sonuç native IOCP veya gerçek DLL Golden sonucu değildir. PEGTL ve
filesystem sonraki ayrı kapsamdır. ASIO_TRANSPORT_FIX09.md güncel tasarımdır.

## FIX10 Config2/PEGTL

Cumulative kaynak paketi gerçek pinned PEGTL'yi içerir. Profil/config application
sırası ve Golden MSVC zinciri değişmez. Frozen FIX09 corpus ve portable/sanitizer
sonuçları native kabul yerine geçmez. Exact sealed ZIP replay dış FINAL_QA
raporuna bağlanır; Windows IOCP/Native/CRT ve gerçek Golden DLL NOT_RUN kalır.
Filesystem hâlâ ayrı iş kapsamıdır. CONFIG2_PEGTL_FIX10.md güncel parser tasarımıdır.

## FIX11

Cumulative source includes gulrak module-side path adapter and an Asio terminal
dispatch-failure repair found by QA. Exact sealed ZIP replay binds to external
FINAL_QA; initial failure records are preserved with their evidence level. No
official release pointer changes. Windows Native/CRT/real DLL Golden remain
separate. FILESYSTEM_FIX11.md defines scope and limits.
