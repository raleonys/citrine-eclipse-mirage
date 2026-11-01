# B27 — Online status, getter çağrıları ve metin kopyasının yaşam süresi

`Resources.native_queries=true`, `native_objects=true` ve aynı `online_version::Binding` gerektirir. Status +08 ve username/persona/user-ID +10/+18/+20 çağrıları, factory tarafından benimsenmiş exact object üzerinden bağlanır. Böylece seçili online kaynak yolundaki **7/7 gözlenen çağrı şekli** kullanılabilir. Tam foreign class/header ABI veya DLL eşdeğerliği iddiası değildir.

## Adres ve davranış

Status `0x180036002` vptr+08 çağrısının EAX düşük32 bitini korur; `0x180036005` sıfır testi ve `0x18003645a` değer1 karşılaştırması BOOL'a indirgenmez. Getter çağrıları `0x180036039`, `0x18003606c`, `0x180036098`: RCX object; RAX NUL ile sonlanan byte-string. Username ataması `0x18003605d`, persona `0x180036089`, user-ID `0x1800360b9`. Online flag ve authentication log bu üç çağrıdan öncedir. Sonraki getter/null/copy allocation hatası, önceki alan atamalarını geri almaz. B26 object commit → cookie geçicisinin bırakılması → status sırası korunur.

`source-evidence.json`, retained ASM/pseudo function Evidence ID, satır, dosya/hash ve bu adreslerdeki talimatları içerir. B22 REA6.5.0/Ghidra12.1.4 oturumu retained kullanılır; B27 yeni original byte/decompile sorgusu yapmaz.

## Kopya boyunca koruma

Yalnız getter çağrısını korumak yeterli değildi: dönen borrowed pointer çağrı sonrasında kopyalanıyordu. `copy_getter`, ortak reservation'ı gerçek NUL taraması, std::string allocation ve byte kopyası tamamlanana kadar tutar. `Effects` üzerinde üç default owned-copy metodu eklenmiştir; legacy default, önceki raw getter/null-error davranışını kullanır. **OnlineExecution ve RecoveredEffects üç metodu açıkça forward eder**; gerçek worker bu korumayı atlamaz. Native-query seçimi kapalı B26 yolu da lower owned-copy'yi ortak object reservation içinde çağırır; legacy özel owned-copy override'ları korunur.

Raw getter API'si normal null sonucu dâhil exact borrowed pointer döndürür; döndükten sonraki kullanım için caller serialization gerekir. Owned-copy null sonucu eski `OutOfEvidenceInput` hatasını üretir. Normal/query exception/null/bad_alloc object sahipliğini değiştirmez; bilinen object ayrı normal destroy ile temizlenebilir. String allocation/metadata native getter'ın çıkış LastError değerini değiştirmez. Hiçbir otomatik foreign retry/destroy/module free eklenmedi.

Vptr ve slot adresi yalnız exact adopted-object doğrulandıktan ve ortak reservation alındıktan sonra memcpy ile çözülür. Null vptr/slot, code/object/module sahiplerini tutan conservative poison+unknown profiline girer; tekrar okuma ve free/destroy retry yapılmaz. Son çözümsüz owner'ın fail-stop davranışı B26'dan sürer. Gerçek SDK query exception'ında object'in bilinen kalması, ownership değiştirmeyen synchronous query sözleşmesine dayanır.

## Sınırlar ve doğrulama

Getter'ın nonnull sonucu kopya boyunca okunabilir ve NUL ile sonlanmış olmalıdır. Reservation yalnız bu Binding üzerinden managed destroy/free/rebind işlemlerini engeller; pointer okunabilirliğini veya dış sağlayıcının mutation/unload yapmayacağını kanıtlamaz. Object/table okunabilirliği, callable code, gerçek module pin, complete foreign formal prototype/class ABI, external ABA ve asynchronous borrowed-string kullanımı açık sözleşmelerdir. Effects vtable ve Resources değiştiği için bütün tüketiciler yeniden derlenmelidir.

35 core kontrolü dört typed çağrı şekli, signed low32 sonuç, exact this/pointer/error, NUL/empty/null, query exception ve açık cleanup, overlap/reentry reddini sınar. Own child191/192 invalid vptr/slot sonrası benimsenmiş object/table sayfasını PAGE_NOACCESS yapar; sonraki query/destroy/free istekleri yeni object okuması olmadan metadata'da reddedilir. Exact state predicate gerekir; generic abort başarı değildir.

Entegrasyon, gerçek SourceBackend worker ve RecoveredEffects forwarding yollarını, gerçek uzun string allocation sırasında reservation'ı, önceki alan prefix'lerini ve sentetik provider'ları sınar. Ayrı negatif kontrol, yalnız work kopyasından OnlineExecution'ın üç copy override'ı kaldırılınca, normal explicit cleanup ardından beklenen temiz exit1 ile gerçek allocation koruma predicate'inde başarısız olur. Bu negatif sonuç PASS sayılmaz; pozitif kaynak/test hash'leri ve kaldırılan satırlar copy-forward-negative-control.json içinde saklanır. Kesin kontrol sayıları ve ortak build sonuçları aşağıdaki frozen doğrulama kaydındadır. Hedef/original/rebuilt DLL veya game çalıştırılmaz; gerçek user CFG/token/profile/achievement içeriği okunmaz; AV engelli girdiler yeniden denenmez.

Yeni native gövde/kapanış **0**; 487 mapped/491 eligible/124 normal claim/52 tarihsel native kapanış korunur. Tam DLL hazırlığı BLOCKED kalır; bu kaynak bağlantılarından toplam DLL tamamlanma yüzdesi türetilmez. Sonraki B28, kalan uygulama fonksiyonlarını adres ve bağımlılık kanıtıyla seçer; öncelik1→2→3→5→4, genel CRT ve numeric0033c0 bekler.

## B27 ortak doğrulama

**35 çekirdek + 38 entegrasyon = 73 kontrol PASS.** Ortak build 245 nesne (241 C++ / 4 C), 285 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 822 kaynak aynı; 8 önceki dosya açık hash kaydıyla güncellendi; toplam 834 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
