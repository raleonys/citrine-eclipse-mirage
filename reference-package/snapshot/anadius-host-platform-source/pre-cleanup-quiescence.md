Kaynak güncellemesi: 2026-11-03. Derleme/test/link yapılmadı.

Seçilmiş OwnedEngine yolu zaten cleanup sonrası engine-empty + gerçek dış
quiescence kontrolü içeriyor. Bu kayıt onu tekrar uygulamaz. Bulunan boşluk,
source cleanup içindeki library unload/online destroy/MinHook uninitialize
çalışmadan ÖNCE unmanaged kullanıcıların durdurulduğu kanıtının bağlanmamasıydı.
OwnedEngine'in mevcut kontratı da trampoline kullanıcılarının uninitialize
öncesinde durdurulmasını istiyor. Sonradan empty görülmesi önceki erişimleri
güvenli yapmaz.

SourceBackend yeni bağımsız CallbackCleanupPrerequisite arayüzünü tanır.
CallbackAttachment'ın mevcut virtual slotları değişmez. Sıra:

1. Gerçek source stop ve admission pause.
2. Gerçek native/FLS join, source route lease drain, Worker.finish.
3. Varsa attachment'ın quiesce_before_source_cleanup kontrolü.
4. Ancak true sonrasında Runtime'ın mevcut exit callback sırası:
   library -> online -> hook cleanup.
5. Mevcut engine-empty/dış proof, native Binding.close ve registry/provider kapanışı.

install_host için pre_cleanup_quiescence zorunludur ve mevcut son proof'tan
ayrıdır. İmzası bir bool callback olsa da true'nun anlamı kalıcı bir dış
taahhüttür: yeni/cached native girişleri ve trampoline kullanıcıları engellenir;
entry epilogue dahil bütün unmanaged kullanıcılar döner; bu engel attachment
kapanana kadar tutulur. Sıfır active_calls veya nonnull code token bunu üretmez.
Kaynak değişikliği yalnız gerçek host kanıtını doğru sıraya yerleştirir; üretim
host'unun böyle bir kanıtı sağladığını iddia etmez.

Matched attachment proof'u swap ile sahiplenir. Proof metadata kilidi olmadan
çağrılır, önce/sonra delegate admission/native lease kontrol edilir. False,
stopped/joined kökleri ve binding'i tutar; kaynak exit cleanup henüz çağrılmaz.
Retry gerçek stopper veya tamamlanan Worker.finish'i yeniden çalıştırmaz.
True, kalıcı engel taahhüdü olduğu için memoize edilir. Unknown exception veya
yutulmuş wrapper reentry poison/retention gerektirir. Hiç yayınlanmamış
attachment mevcut unpublished kapanış yolunu kullanır; dış proof çağrılmaz.

Yeni AssemblyInputs alanı HOST yerleşimini değiştirir. Bir sonraki açık derleme
talebinde bütün tüketiciler yeniden derlenmelidir. Yeni attachment'ın ikinci
arayüzü özel source adapter'a aittir; eski CallbackAttachment vtable'ı korunur.

Mevcut gerçek owning Runtime/Resources test kaynağına eksik pre-proof passive
ret, true post-proof varken iki false pre-proof denemesinde sıfır source cleanup,
ardından true ile gerçek cleanup ve başarısız engine uninitialize sonrası
tekrar etmeme senaryoları eklendi. Kontrollü SDK fixture'ıdır; çalıştırılmadı.

İki production loader hook'u hâlâ tanımsızdır. Gerçek pre/post dış kanıtlar,
per-image sahipler, failed-attach politikası ve desteklenen özgün senkron loader
başlangıcı hâlâ gereklidir. Bu adımın sonucu çalışan/derlenmiş DLL değildir.
