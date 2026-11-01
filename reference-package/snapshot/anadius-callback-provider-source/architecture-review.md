# B22 bağımsız mimari inceleme

Sonuç: **PASS**. İnceleyen: `/root/b22_provider_audit`. Üretim, core ve integration testi yazarlarından bağımsızdır.

Sahipli compiler locale ve invalid-call sağlayıcısı, sayısal ANSI/OEM çözümü ve gerçek foreign factory üzerinden dört alt CRT yolunun PTD giriş/çıkış kontrolü incelendi. Locale, derleyici errno alanı ve kurtarılmış PTD locale alanı ayrı tutulur. Legacy tablo yolu korunur.

Core **60**, integration **20** kontrol için root (core) ve `/root/b22_integration` (integration) tarafından üretilen PASS kayıtlarının kaynak ve EXE SHA-256 değerleri bağımsız doğrulandı; testler yeniden çalıştırılmadı. B21 ZIP kimliği ve 810 önceki kaynak doğrulandı: **807 değişmeden**, **3 açık değişiklik**, **4 yeni kaynak**, toplam **814**. Ortak build bu inceleme sırasında sürüyordu; 240 nesne / 236 CPP / 275 grup + 19 terminal senaryosu henüz bu raporda PASS sayılmamıştır.

`ByteWide` artık sahipli sağlayıcı varken kullanılmayan legacy tabloyu zorunlu tutmaz. Invalid-copy handler öncesi gerçek UTF-16 çıktı prefix’i ve throw sonrasındaki WinLastError korunur. Wide staging/preclear alt CRT izninden önce gerçekleşir; tüm bridge girişinin etkisiz reddi iddia edilmez. `select` foreign factory ve publication öncesinde çağrılmalıdır.

Sayısal sağlayıcı kurtarılmış 65001/0/1 seçimini ve flags/used-default kararını değiştirmeden alt çağrıda 0/1 alias’larını çözer. UTF-8’e çözülen alias için flags sessizce değiştirilmez; uygun olmayan kombinasyon dönüşüm hatası verebilir. Süreç genelindeki locale/handler dondurulmamıştır.

Native callback testleri kendi kaynak shimleri, sentetik veriler ve kendi heap/FLS alanlarıyla çalışır. İki terminal çocuk; aynı exception ve PTD/binding drain için 221, handler FLS değişimi sonrası tutulan poison/prefix ve sonraki alt çağrının reddi için 222 bekler. Genel abort 239 ve beklenmeyen dönüş 230 başarı yerine geçemez.

**Yeni native gövde/kapanış: 0. DLL bütünü hazır değildir.** Generic CRT, native ABI/PE, hooks/unload, dış quiescence, yedi online foreign call, configuration/license/legacy document ve B10 inceleme borcu açık kalır. Asıl/rebuilt DLL veya oyun çalıştırılmadı; kullanıcı içeriği okunmadı. Ayrıntılı kanıtlar ve hash’ler `architecture-review.json` içindedir.
