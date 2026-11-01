# B22 — Sahipli callback compiler sağlayıcıları

Seçili callback kaynağı artık karşılaştırma locale’ini ve invalid-call davranışını bir **sahipli CrtProvider** ile tutabilir. `_create_locale` ile açık ad üzerinden kurulan nesne, `_stricmp_l` ve `_wcsicmp_l` çağrılarına bağlanır; son sahibi bırakıldığında `_free_locale` çalışır. Bu compiler locale’i, kurtarılmış PTD initial-locale nesnesi değildir. Eski dört işaretçili CrtApi yolu korunur; yeni sağlayıcı açık seçimdir.

`select(resources, "C", handler, {Filesystem::Utf8, 1252, 437})` factory/yayınlama öncesi çağrılır. Handler zorunludur; dönebilir veya exception atabilir. Kurulumun ikinci parçası başarısız olursa önceki resources korunur. Live provider değiştirme veya arbitrary provider reentry desteklenmez.

## Copy ve hata sözleşmesi

Owned copy, okunabilir NUL-terminated girdi, nonoverlap ve INT_MAX sınırındaki capacity için seçili bir kaynak politikasıdır. Başarı yalnız metin ve NUL’u yazar. Geçerli hedefte null kaynak/çok küçük kapasite, handler çağrılmadan önce ilk elemanı temizler. Handler dönerse EINVAL/ERANGE ve compiler errno sağlanır; atarsa aynı exception ve görünür prefix korunur. Null karşılaştırma girdileri ortamın CRT invalid handler’ına iletilmez. Orijinal CRT debug-fill, invalid-handler veya EH/ABI eşdeğerliği iddia edilmez.

Foreign factory, hem owned hem legacy CRT sağlayıcısının dört alt çağrısını gerçek FLS/PTD pre/post izniyle sarar. Inherited wide route da aynı alt kontrolü kullanır. **ByteWide staging ve invalid-output preclear, alt CRT kontrolünden önce gerçekleşir**; tüm wide bridge’in etkisiz reddedildiği iddia edilmez. Geçersiz postcondition lower exception’a önceliklidir. ByteWide normal dönüşte ve exception’da sağlayıcının çıkış WinLastError’ını korur; çıktı geri alınmaz ve çağrı tekrarlanmaz.

## Sayısal codepage

NumericEncoding, compiler locale ve process File-API mode okumadan sabit synthetic filesystem seçicisi kullanır: UTF8→65001, ANSI→0, OEM→1. Kurtarılmış048ac0/048b30 algoritması flags ve used-default dalını bu seçime göre belirler. Yalnız alt conversion sınırında0/1, açık sayısal ANSI/OEM sayfalarına çözülür; query/fill aynı immutable değerlere bakar. MAC/thread takma kodları2/3 reddedilir.

Bu sıra özellikle korunur: ANSI takma kodunun sayısal65001’e çözülmesi, flags0x400 ve used-default parametresini kendiliğinden UTF8 flags0x80’e dönüştürmez. Alt API’nin reddettiği flags ve hata0x3ec retry davranışı korunur. IsValidCodePage, her sayfanın her flags birleşiminde çalışacağını garanti etmez. Literal65001/54936 yolları kurtarılmış özel flags dalını kullanmaya devam eder. NUL/count/query-fill length ve yazılmış hata prefixleri korunur.

## Kanıt ve sınırlar

source-evidence.json retained Ghidra VA/Evidence ID/line/hash bağlarını ve own synthetic test kayıtlarını ayırır. B22 binary_session REA6.5.0/Ghidra12.1.4 kullanılabilirliğini doğrular; target açılmadı, yeni original byte/decompile sorgusu yapılmadı. Original/rebuilt DLL/game çalıştırılmadı veya değiştirilmedi. Gerçek user CFG/token/profile/achievement içeriği okunmadı; AV engelli öğeler yeniden denenmedi.

Sahipli seçim, tüm uygulamanın compiler/global provider davranışını sabitlemez. B18–B21 single-active PTD/admission, aynı Execution, shared stream ve ayrı external quiescence şartları sürer. 7 online foreign çağrı, configuration/license/legacy document, B10 tarihsel review borcu ve native CRT/EH/ABI/unload/PE kapıları açık kalır. Yeni native gövde/kapanış0;487mapped/491eligible/124normal claim/52tarihsel kapanış değişmez. Tam DLL hazırlığı BLOCKED; toplam DLL yüzdesi bu bileşimden türetilmez.

Sonraki B23: seçili native callback original sağlayıcılarının sahipliği ve yayınlama kanıtı. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B22 ortak doğrulama

**60 çekirdek + 20 entegrasyon = 80 kontrol PASS.** Ortak build 240 nesne (236 C++ / 4 C), 275 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 807 kaynak aynı; 3 önceki dosya açık hash kaydıyla güncellendi; toplam 814 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
