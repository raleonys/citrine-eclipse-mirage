# B26 — Online factory, destroy ve cookie geçici ömrü

Resources.native_objects=true seçimi, mevcut **aynı online_version::Binding** üzerinden create_online factory ve vptr slot0 destroy çağrısını bağlar. Ayrı module/object gate yoktur. Seçili toplam **3/7 typed source binding**: version, factory, destroy. Status ve üç getter hâlâ mandatory lower sağlayıcıdadır; bu dört synchronous çağrı da exact adopted-object ve ortak module reservation ile korunur. B25-only ve default legacy yollar korunur. Resources, Binding ve Snapshot düzenleri değiştiği için bütün tüketiciler yeniden derlenmelidir.

## Factory, destroy ve sahiplik

Factory, gözlenen Windows x64 RCX=UTF16 cookie / RAX=object çağrı şekliyle çalışır. Cookie pointer’ı yalnız çağrı boyunca ödünçtür. Exact loaded module üzerindeki nonzero factory çözümü ve açık CodeLease gerekir. Normal sonuç, null dâhil, rezervasyon bitmeden kaydedilir. Null sonuçta core mevcut OutOfEvidenceInput hatasını üretir; module yaşar ve ayrı cleanup yapılabilir. Başka object/token adopt edilmez.

Destroy yalnız exact benimsenmiş nonzero object için çalışır. İlk8 byte vptr, vptr’ın ilk8 byte’ı destroy adresi olarak memcpy ile okunur; typed C++ class/vtable aliasing yapılmaz. Object/table okunabilirliği ve kodun callability’si provider sözleşmesidir; arbitrary pointer validity veya foreign C++ class ABI’si kanıtlanmış değildir. Gözlenen çağrı **void(this)** şeklindedir; EDX/deleting-destructor flags eklenmez.

Canlı/bilinmeyen object, version/factory rebind ve module free isteğini lower başlamadan engeller. Factory/destroy ownership mutation öncesi unknown claim alır. Factory exception’ı, destroy exception’ı veya null vptr/slot object/module/code’u tutan ortak poison üretir; kör retry, onarım, ikinci vptr okuması veya free yoktur. Son owner canlı/bilinmeyen object varken kilidi bıraktıktan sonra, code owner’ları bırakmadan fail-stop yapar. Otomatik foreign destroy/raw module free eklenmedi.

Normal destroy, adopted object’i temizler; core ardından global object’i sıfırlar ve module free’ye geçer. Normal TRUE/FALSE free iki callable kaydı kapatıp iki lease referansını reservation açıkken kilit dışında bırakır; FALSE unload başarısı sayılmaz. Free exception’ı, önceki object=0 prefix’i ve nonzero module/code sahiplerini tutar.

## Bulunan cookie-scope farkı ve düzeltme

Retained assembly sırası: **35fac factory →35fb1 global object ataması →35ff3 cookie buffer free →36002 status**. Önceki C++ load_online gövdesinde cookie_copy fonksiyon sonuna kadar yaşıyordu. Şimdi dar block içinde copy ve object ataması var; block null kontrolü/status öncesinde biter. Copy constructor hatası factory/atamayı başlatmaz; factory exception’ı copy’yi unwind eder ve global object atamasını yapmaz.

Own EXE allocator observer, yalnız factory’nin aldığı gerçek2048UTF16 buffer adresini izler. İlk delete’te CAS ile adresi disarm eder; freed bytes okunmaz, canlı coreobject slotu ve allocation lifetime kaydedilir, WinLastError korunur. Eski source fresh derlenerek normal destroy/free sonrasında **beklenen temiz exit1** verdi: buffer status sırasında henüz bırakılmamıştı. Bu negatif kontrol PASS sayılmaz. Scope düzeltmesinden sonra buffer tek kez, global object atamasından sonra ve ilk status öncesinde bırakıldı; status factory error5353’ü gördü. cookie-negative-control.json eski source hash’i, compile/run ve başarısız predicate’i saklar.

İlk ortak build bu fark bulununca kesildi; o koşu authoritative PASS değildir. Son ortak doğrulama yalnız düzeltilmiş frozen source için yürütülür. Cookie temporary lifetime düzeltmesi dışında recovered online algoritma sırası korunur.

## Test ve kanıt

**39 core +52 integration =91 yeni kontrol PASS.** Core unaligned object/vptr bytecopy, exact cookie pointer/result/error, canlı object rebind/free reddi, normal destroy sonrası vptr’ın hemen silinmesi, shared-gate overlap ve iki lease retirement sırasında sekiz managed reentry reddini sınar. Own child181, canlı son-owner fail-stop’ta metadata kilidinin açık ve sahiplerin tutulmuş olduğunu doğrular;182–185 factory/destroy/nullvptr/nullslot için exact state/exception/error/retention predicate’leri kullanır. Generic abort239/unexpected230 başarı değildir.

Entegrasyon gerçek recovered worker/load/cleanup, uzun ve embedded-NUL cookie, own typed factory ve slot0 void(this) destroy ile çalışır. Dört lower query’nin exact object/active koşulu ve sırası, null/missing factory, ikinci lease throw/null, version mismatch, getter exception prefix’i, normal FALSE ve unknown free exception durumları sınanır. Unknown child221–224, hata sonrasında kendi object sayfasını PAGE_NOACCESS yapar; retry/cleanup/rebind/query metadata’da reddedilir ve yeni object read/SDK/lower etki olmaz.

Hedef DLL/game, actual online DLL, gerçek LoadLibrary/FreeLibrary veya user profile I/O çalıştırılmaz; sentetik loader/export/UI ve memory-files kullanılır. Original/rebuilt DLL/game değiştirilmedi; gerçek CFG/token/profile/achievement içeriği okunmadı, AV engelli öğeler yeniden denenmedi.

## Açık kalan kapsam

source-evidence.json VA/Evidence ID/line/hash ile retained caller kayıtlarını, cookie sırasını ve negatif/pozitif kanıtı bağlar. B22 REA6.5.0/Ghidra12.1.4 oturumu retained kullanılır; B26’de yeni original byte/decompile sorgusu yoktur. B18–B25 callback PTD/admission/Execution/sharedstream/owned-code sözleşmeleri ve bağımsız external quiescence şartları sürer. Guarded lower getter’ların borrowed string’i kopyalanana kadar caller serialization gerekir; bütün foreign reentry veya asynchronous string tüketimi güvence altına alınmış değildir.

Yeni native gövde/kapanış0;487mapped/491eligible/124normal claim/52tarihsel kapanış korunur. Seçili typed source bağlantısı, foreign formal prototype/class ABI/readability/module pin veya whole DLL eşdeğerliği değildir. Tam DLL hazırlığı BLOCKED; toplam DLL yüzdesi bundan türetilmez. Configuration/license/legacy document, B10 review borcu ve nativeCRT/EH/unload/PE açık kalır.

Sonraki B27: status+08 ve getter+10/+18/+20 yolları. İstenen sıra1→2→3→5→4; genel CRT kuyruğu bekler.

## Foreign sınırlar

| Sınır | Çağrı VA | Seçili durum |
|---|---|---|
| version_check | `0x180035e8c`, `0x180035e8e` | Typed source binding |
| create_online | `0x180035fac` | Typed source binding |
| status_virtual_08 | `0x180036002` | Mandatory lower + lifetime guard |
| username_virtual_10 | `0x180036039` | Mandatory lower + lifetime guard |
| persona_virtual_18 | `0x18003606c` | Mandatory lower + lifetime guard |
| user_id_virtual_20 | `0x180036098` | Mandatory lower + lifetime guard |
| destroy_virtual_00 | `0x180035b8c`, `0x180036703` | Typed source binding |

## B26 ortak doğrulama

**39 çekirdek + 52 entegrasyon = 91 kontrol PASS.** Ortak build 244 nesne (240 C++ / 4 C), 283 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 821 kaynak aynı; 5 önceki dosya açık hash kaydıyla güncellendi; toplam 830 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
