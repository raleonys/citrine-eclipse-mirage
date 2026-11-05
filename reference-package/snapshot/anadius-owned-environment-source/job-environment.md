# Çocuk iş için sınırlı Environment erişimi

Bu adım yalnız kaynak kodudur. Derleyici, test, linker ve DLL çalıştırılmadı;
eski ZIP güncellenmedi. Önceki README doğrulama sayıları bu değişikliği kapsamaz.

`Environment::enter_job`, tam oturum nesli/owner/control kimliği taşıyan kayıtlı
bir kaynak projection ile canlı `SessionMemoryDomain::JobPermit` ister. İzin
aynı Parent Operation, çocuk thread ve fiber içinde geçerlidir. Başka oturum,
kapsam öncesinde üretilmiş aynı oturum kopyası veya eski capability kabul edilmez.
Kaynak projection ve SessionOwner ömrünü çağıran korur; paylaşılan Parent lease
tek başına bu nesnelerin ömrünü kanıtlamaz.

Query, type karşılaştırması, mevcut factory/prepare/copy/move/destroy yolları
bu kapsamda kullanılabilir. Kopyalar tam kaynak kimliğini ve kapsamın benzersiz
epoch değerini taşır. Kaynağı move/destroy yasaktır; sadece kapsam içinde
oluşturulan geçici projection'lar taşınabilir ve yok edilebilir. Session/accept
oluşturma, public release, handler/response scheduling ve Environment kapanışı
asıl worker yetkisini korur. Task/functor slotları bu retlerden önce tüketilmez.

`JobHandoff::clone_environment`, canlı kaynak claim'i ve Parent Operation altında
mevcut `023dd0` constructor yolunu çağırır, kopyayı sorgular, table üzerinden yok
eder ve kapsamı kapatır. İş kaydını tamamlamaz, closure çalıştırmaz ve strong
referansını bırakmaz. Bu, native mesaj yürütmesinin tamamlandığı anlamına gelmez.

Kapsam kapanırken izin önce geçersizleşir. Aktif çağrı veya geçici projection
kalması başarı sayılmaz. Kaynak projection kapanış öncesi/sonrası tam alanlarıyla
yeniden doğrulanır. Aktif callback varsa hedef callable erken yok edilmez;
çalışan çağrı paylaşılan state'i tutar. Hata halinde provider poisoned kalır;
native kısmi adımlar için tahmini cleanup yapılmaz. Control kilidi Parent izin
kontrolü veya dış checkpoint çalışırken tutulmaz.

Eklenen `multi_session_projection_lineage` test kaynağı, asıl worker üzerinde
iki oturum ile copy/factory/move capability akışını tarif eder. Çalıştırılmadı.
Gerçek hazırlanmış Profile + JobHandoff + çocuk Environment kapsamını kapsayan
pozitif uçtan uca test henüz yoktur; callback retirement, kapsam kaybı ve hata
prefix'leri de çalışma zamanı doğrulaması bekler.

Sıradaki uygulama işi Policy/Graph ile Profile mesaj yürütmesinin aynı tam iş
yetkisine bağlanmasıdır. Scheduler/closure/gerçek thread çıkışı ve iki loader
hook'u ayrıca açıktır. Kaynak dosya sayısı toplam DLL tamamlanma yüzdesi değildir.
