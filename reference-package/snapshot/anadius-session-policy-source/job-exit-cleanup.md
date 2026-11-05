# Çocuk iş için04a47c temizleme bağlantısı

Durum: **yalnız kaynak kodu; derlenmedi, test/link/DLL çalıştırılmadı**.
Yeni test kaynağı eklenmedi. Pozitif prepared Profile + JobHandoff + Policy
çocuk temizliği entegrasyon senaryosu henüz yoktur; statik inceleme bu kanıtın
yerine geçmez. Önceki PASS ve ZIP kayıtları tarihsel sürümlerine aittir.

`Policy::cleanup_job_exit_waits(permit, checkpoint)` mevcut
`ExitWaitRegistry::cleanup_current_thread()` yolunu çocuk worker'a açar. Yapıcı,
bind, close, default cleanup, log, guard ve worker/lifecycle işlemleri kendi
asıl worker/fiber koşullarını korur. Yeni giriş nonvirtual'dır; public temel
sınıf vtable'ı veya nesne yerleşimi değiştirilmedi.

Seçilen Policy, kendisini ve Parent lease'ini çağrı boyunca tutar. İzin aynı
Parent ve canlı çocuk Operation'a ait olmalıdır. Yanlış Parent/actor veya
süresi bitmiş izin, aktif rezervasyon ve kayıt temizliği öncesinde reddedilir.
Call önce boş callback ile hazırlanır; aktif rezervasyon bundan sonra kurulur,
callback bu rezervasyon altında ve Policy kilidi dışında swap ile alınır.
Yetkili yeniden giriş Policy'yi poisoned yapar; yutulan hata dış çağrının kayıt
temizliğine ilerlemesine izin vermez.

İzin, kaynak durumu ve açık iş checkpoint'i opaque çağrı öncesi/sonrası yeniden
kontrol edilir. Callback hedefi aktif rezervasyon sürerken emekliye ayrılır;
son izin/kaynak kontrolü bundan sonra yapılır. Hata halinde tahmini ikinci
temizleme yapılmaz. Sayaç, dönen gerçek registry çağrısını kaydeder; postcheck
hatası bilinen temizleme prefix'ini geri almaz.

Alt gövde değişmedi: `outputs/anadius-source/bootstrap.cpp` içindeki mevcut
`0x18004a47c`, native thread kimliğini alır, yalnız eşleşen thread_id kayıtları
için ready/unlock/notify/entry-clear adımlarını uygular. Policy kilidi registry,
Parent kontrolü veya dış checkpoint çalışırken tutulmaz. Bu işlem compiler
CRT'nin başka kayıtlarını temizlemez ve fiziksel thread/FLS çıkışını kanıtlamaz.

`JobHandoff::cleanup_exit_waits()` yeni somut yolu dispatch tombstone, canlı
Profile çağrısı ve çocuk Parent Operation ile çağırır. Tam iş/oturum nesli,
closure binding, owner/function tokenı, control ve claim eşleşmesi kontrol edilir.
Yalnız seçilmiş somut Policy kabul edilir; legacy sağlayıcıya fallback yoktur.
Kontrol, callback boyunca aynı heap backing'i tutar; stack Operation/Call
adresleri checkpoint'e yakalanmaz.

Bu açık kaynak işlemi job'u Pending durumundan çıkarmaz; message request'i
tüketmez, strong/job sayısını azaltmaz, closure'ı serbest bırakmaz. Mevcut
`021770` köprüsü hâlâ default Policy çağrısını kullanır. Tam çocuk mesaj yolu
kurulurken bu sınırlı çağrının o köprüde seçilmesi ayrıca gereklidir.

Sıradaki iş Graph ve Profile mesaj yürütmesini tam iş yetkisine bağlamak;
hazırlanmış Profile fixture'ında Deque, Environment ve bu temizleme yolunu
kapsayan pozitif/negatif senaryoları eklemektir. Çalıştırma için kullanıcının
açık yeniden derleme talebi gerekir. İki loader hook'u ayrıca açıktır.
