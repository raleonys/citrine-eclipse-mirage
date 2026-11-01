# Multibyte başlangıç denetimi

Kapsam: özgün SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için retained REA assembly/pseudocode ve mevcut yeniden yazılmış kaynaklar. Bu denetimde özgün DLL çalıştırılmadı, yüklenmedi veya değiştirilmedi; üretim koduna dokunulmadı. Aşağıdaki akış normal dönüş yoludur; SEH/abort alanı ayrıca çözülmüş sayılmaz. JSON dosyası kaynak hash'lerini ve Evidence ID'lerini verir.

## Kesin veri ve erişim sınırları

İlk MB kaydı VA `0x1800a1c70`, boyut **0x228 / 552 byte**; refcount +0, CP +4, mode +8, altı WORD +0xc, type +0x18/257 byte, case +0x119/256 byte, padding +0x219/7 byte, son pointer +0x220/8 byte. Refcount dahil tablolar dışındaki alanlar sıfırdır. Type/case dizilerinin 52'şer ASCII harf byte'ı sıfır değildir. Kayıtta base relocation yoktur. Ayrı ilk tablolar A1EA0/257 ve A1FB0/256 aynı slice'ları içerir.

Current-locale A52E0, current-MB A59D0, type/case pointer A59C0/A59C8, changed A52D8, system-CP flag A59D8 ve initialized A59DC loader zero-fill sıfırlarıdır. Locale mask A1C6C ham dosyada **0xfffffffe**'dir. Initial pair A1938/A1940, DIR64 relocation ile ilk locale A17E0 ve ilk MB A1C70'e gider. `initialized` fonksiyonlarda **byte** olarak okunur/yazılır.

PTD en az **0x3c8 byte** canlı sıfırlanmış storage gerektirir; MB pointer +0x88, locale pointer +0x90, flags DWORD +0x3a8. Normal constructor0648fc önce sabit alanları ve flags=1'i yazar, MB=initial kaydeder, sonra lock5 altında MB ref+1, lock4 altında mevcut locale'i yükleyip yerine koyar. Buradaki “64” adres öneki veya x64 modelidir; **64 PTD nesnesi oluşturulduğu çıkarımı yoktur**. Mevcut model state-index0 için tek PTD'dir.

## Assembly ile başlangıç sırası

`initialize_multibyte_06f5f8` assembly ev_e90764f05824434e1aeb88a2b5fee00f692636a92849517a7c91ea719da84931:

1. F5FC: initialized byte !=0 ise F651'e atlar; AL=1 döner.
2. F60C: case pointer=ilk case tablosu; F621: current MB=ilk MB; F628: type pointer=ilk type tablosu. **Bu current-MB yazımı ref artırmaz.**
3. F62F: required_raw_064c64. FLS index/geçerli thread PTD alanı, reentrancy sentinel, calloc(1,0x3c8), FLS'e publication ve constructor bağımlılıkları canlı olmalıdır. FLS publication constructor'dan öncedir; başarısız acquire abort eder. Hazır PTD varsa constructor tekrarlanmaz.
4. F645: set_multibyte_06f2d0(-3,1,ptd,&currentMB).
5. F64A: setter'ın normal dönen status'üne bakmadan initialized=1. F651: AL=1. Üst RAX bitleri boolean ABI taahhüdü değildir. Başarısız builder/allocation da normal dönüşse initializer true döner ve yeniden deneme kapısı kapanır.

`set_multibyte_06f2d0` assembly ev_85ea52b71c9b1d78e4844e8c6a0d02a4b390325ac23bac449f0e8c968b21073c:

- F300: update_thread_MB06f540 önce; sonra F307: system_cp06efd0(-3). CP helper önce locale_update058b04(0), flag=0, ardından flag=1 ve GetACP yapar. Changed=0 ise locale_update ilk pair'i kullanır; current-locale'ın sıfır olması bu hızlı yolun ilk-locale pointer'ını sıfır yapmaz.
- F31C: PTD MB+4 ile CP eşitse return0; allocation, builder ve publication yoktur. Native ACP değeri burada gözlemlenmedi.
- F329..F42D: yeni **0x228** allocation; önce eski PTD MB'nin tamamı stack snapshot'a, sonra yeni storage'a kopyalanır. Kopya sırasında yeniye yazmak eski okumalarla iç içe geçirilmez. F433 refcount'u0 yapar; F438 builder06f674 çağırır.
- Gerçek builder başlangıçta system_cp06efd0(resolved CP)'yi **yeniden çağırır**. Pozitif CP için ikinci locale_update ve system flag=0 oluşur. Son system-CP flag'inin her başarılı startup'ta1 kaldığı varsayımı hatalıdır. RecordNative, canlı support Effects'e bağlanmış olmalıdır.
- Allocation null: free(null), return FFFFFFFF. Builder FFFFFFFF: error hücresi05d724=22, yalnız yeni allocation free, return FFFFFFFF. “errno/doserrno” decompiler adı yerine gerçek05d724 Effects kimliği korunmalıdır.
- Başarılı builder: initializing!=0 olduğundan changed işaretleme06532c atlanır. F47A eski PTD MB ref-1; önceki1 ve kimlik initial değilse free. F4A1 fresh ref=1, F4AE PTD MB=fresh.
- F4C2 mask&PTDflags sıfırsa publish06ee00 lock5 ile type257/case256 kopyalar, eski current-MB ref-1, current=fresh, fresh ref+1, unlock5. F517 initializing!=0 iken ilk-pair MB pointer'ını current ile değiştirir. MaskFE ve constructor flags1 için TEST sonucu0; publication açıktır. Sonra free(null), builder status döner.

06f540 mask&flags!=0 ve PTD locale!=0 ise PTD MB'yi korur. Aksi halde lock5 altında PTD MB/current karşılaştırılır; farklıysa eski ref release/free, current yeniden okunur, PTD'ye yazılır ve ref+1 yapılır. Null sonuç abort eder; null current üzerine gerekli atomic erişim abort'tan önce oluşabilir. Image-zero global current'ı initializer atlayarak doğrudan bu yola vermek geçerli startup değildir.

## Refcount çıkarımı: varsayım ile kanıtın ayrımı

Statik ilk kayıt ref0'dır; bunu1'e düzeltmek için kanıt yoktur. Constructor başına MB retain+1 vardır. Basit örnek **tek PTD**, initial ref=1, current=initial, flags1, ACP!=0, builder başarılı ve side effect mutasyonu yok varsayılsın: update_thread eşitlikte ref değiştirmez; setter eski PTD ref1->0; publication eski global ref0->FFFFFFFF; static kimlik free'yi engeller. Fresh önce1 sonra global retain ile2 olur. İlk-pair pointer yazımı ek ref artırmaz. Genel olarak bu iki release öncesi initial ref=k ise sonuç k-2 modulo2^32'dir. Bu örnek gerçek bütün DLL initializer sırası veya çalışan süreç refcount ölçümü değildir; diğer PTD retain'leri ve önceki işlemler sonucu değiştirir.

Başarılı publish sonrası fresh PTD ve global sahiplikleri ayrı iki ref'tir; PTD destructor release64_89c bunlardan birini azaltır. Heapten alınan her fresh refcount için atomik erişimden önce gerçek hizalı LONG ömrü ve kayıt defteri kaydı gerekir. Counter kayıt defteri free/unbind sırası ve lock4/5 bütünlüğü heap alanıyla ortak olmalıdır. C++ unsigned modulo davranışı, static ref FFFFFFFF sonucunu kaybetmemelidir.

## Entegrasyon boşlukları

- `ImageState` doğru image snapshot'ı sağlar; `globals()` const'tur, current alanları0 kalır. Multibyte/locale bridge ayrı değişebilir Globals ister. İlk pair, initial identities, maskFE ve mevcut0 değerlerini ortak tek domaine aktaracak composition henüz bu API içinde yoktur. Current alanlarını önceden initialized gibi doldurmak snapshot kapsamını değiştirir.
- `ImageState` yalnız read_address ve kayıtlı üç LONG için atomic Effects sunar. Bridge Memory'nin read/write8/16/32/address işlemleri; slot adresleri, PTD, tablo yazımı, heap kayıtları ve okunabilir tüm alanlar için bir bounded adapter gerekir. Type/case Address accessor'ları canlı owned dizilerdir; immutable data modülünün const tablolarını publication destination yapmak uygun değildir.
- Resource bridge Resources, PTD/ref/free gövdelerini birleştirir ama lock/unlock ve native atomik provider hâlâ caller'a aittir. ImageState counters registry'si fresh heap MB sayaçlarını eklemeye kapalıdır. Üç başlangıç LONG'u ile dinamik heap LONG'larının aynı native adres domainine dispatch edilmesi gerekir; ImageState atomic'sini tek başına tüm kaynak provider'ı sanmak doğru değildir.
- FLS State/index, callback0649cc, gerçek sıfırlanmış Ptd allocation/ömrü, construct/destruct routing, exception-table relocated identity (08A930), heap/errno/abort provider'ları gerekir. Mevcut ImageState exception table veya FLS sağlayıcısı değildir.
- RecordNative+multibyte Effects için döngüsel binding açık: iki nesne kurulmalı, `bind_system_cp(effects)` sonra initialize çağrılmalı. Native ACP/OEMCP, CP validation/info, iki alt CRT map/type wrapper'ının defined-output provider'ları henüz zorunlu bağımlılıktır.
- Native publication locks 4/5, changed atomic exchange ve initialized byte için dış initializer serialization gerekir. 06f5f8 kendi içinde once/lock/CAS uygulamaz. Tek thread normal startup dışında eşzamanlı iki initializer için çıkarılmış güvence yoktur.

## Açık kalanlar

DLL'nin tüm initializer dizisinin bu çağrılara göre sırası; çalışılan Windows ACP/OEMCP'si; başarı/failure yolu; FLS/heap/exception-table native deployment; tam SEH ve fault footprint; ABI/bütün-DLL eşitliği bilinmiyor. Bu denetim yeni REA evidence veya yeni orijinal gövde üretmez. Önerilen bir sonraki doğrulama yalnız statik kanıt ve kendi owned fixture'ı ile image-zero -> PTD -> setter -> publish zinciridir; özgün DLL execution gerekmez.
