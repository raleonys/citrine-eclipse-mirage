# REDFIELD RF33 — Registry byte çıktısı refactoru

21 Ekim 2026 · LOCAL_BUILD / NOT_RELEASE.

Mevcut registry policy içinde REG_SZ yazımını bağımsız ve allocation-free bir
yardımcıya ayırdım. Yapılandırma seçimi, UTF-8 dönüşümü, geçici belleğin sahibi
ve serbest bırakma sırası mevcut policy içinde kaldı. Ortak yazıcı yalnızca bir
çağrı boyunca ödünç aldığı kaynak ve byte tamponuyla çalışır.

Eski write_w byte tamponunu uint16_t* olarak kullanıyordu. Exact RF32 fonksiyonunu
hizasız bir byte tamponuyla çağıran UBSan denemesi misaligned store üretti ve
exit1 verdi. Yeni yazıcı UTF-16LE byte'larını tek tek yazar; hedefte uint16_t
nesnesi veya hizalama varsaymaz. Bu, eski tanımsız davranışı giderir; desteklenen
hizalı çıktının ve hata sözleşmesinin değiştirilmesi amaçlanmadı.

Kaynak/test kapsamı: boş metin, embedded NUL, yüksek byte, Unicode/surrogate
birimleri, sekiz tampon offset'i, 0–13 kapasite, size-only sorgu, eksik size
pointer'ı, null kaynak ve boyut taşması. Kısa tampon veriye dokunmaz; gerekli
boyut ve REG_SZ türü eski sırada yayımlanır. Kapasite önceki API çağrısından önce
yakalanmış değerdir; size hücresinin güncel içeriğinden yeniden okunmaz.

Odaklı yardımcı ve gerçek policy fallback çağrısı ASan/UBSan altında geçti.
Windows dönüşüm dalı sentetik API/heap modeliyle ayrıca sınandı: ilk/ikinci
dönüşüm hatası, allocation failure, boş configured metin, eksik size pointer,
kısa tampon ve sorgu. Model gerçek Windows/SDK ABI testi değildir.
Üç yanlış uygulama varyantını regresyon reddetti: yanlış endian, tam kapasiteyi
reddetme ve kısa tamponda başarı bildirme.

98 temiz GCC regresyonu geçti; RF32'nin 97 testinin stdout ve exit değerleri
korundu. Eski GetConfig exit14, FAIL_OBSERVED olarak ayrı kaldı. İlk workspace
denemesi, son kontrolde iki çıktının executable bit'i kaybolduğu için başarısız
oldu; bu kayıt korundu ve PASS yapılmadı. Son derleme izole kaynak kopyasında,
nesne/çıktı/tool/input son kontrolleriyle tamamlandı.

Bir mevcut uygulama dosyası değişti; iki yeni dosya eklendi. Eski 265 uygulama
dosyası ve 58 portable core kaynağı aynı. Build runner ve CMD'de yalnız RF33
gösterim/receipt kimliği güncellendi; launcher düzeni, argüman, exit ve pause
akışı korundu. Üç public export, yedi hook, 35 binding, surge.cfg, preloader_l,
GPL-3.0-only, vendor pinleri ve resource korundu.

Yeni tersine mühendislik paketindeki ABI hücreleri, bellek temsili incelemesine
referans oldu; ScalarCell gövdesi kopyalanmadı. İki yeni portable dosyanın sahibi,
ömür/hata/bağımlılık sınırı ve güncel source/test kanıtı qualification kaydına
bağlandı. Bu iki dosyanın qualification'ı tüm registry modülünü kapatmıyor.
DWORD/HKEY/trampoline SDK-tip sınırı ayrı açık inceleme adayıdır.

Gerçek Windows/MSVC, Debug/native/CRT ve oyun testi NOT_RUN. CMake mevcut ortamda
yok; yeniden çalıştırılmadı. LSan çalıştırılmadı; odaklı ASan detect_leaks=0 kullanır.
Bu teslim yeni DLL veya official release değildir. Daha önce teslim edilmeden
kalan RF33 Debug işi tamamlanmış sayılmıyor; bu kayıt exact RF32'den türeyen
registry düzenlemesidir. 16 tüm-modül incelemesi, 298 code ve dört vendor
qualification açık; M20 taşıma kapsamı ve RFCL-005 profil kök nedeni açık.

Yeniden çalıştırılabilir normal regresyon, mevcut build_verify girişidir. Odaklı
test tests/test_registry_value_output.cpp içindedir. İzole fallback ve sentetik
conversion deneylerinin kaynak/komut/çıktıları evidence/rf33_registry_* altında;
parent alignment stderr'i ve ilk başarısız derleme de evidence içinde saklıdır.
Normal regression ve model test sonuçları native/game kabulü yerine geçmez.
