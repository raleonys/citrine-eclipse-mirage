# Güncel profil avatarı

Sahip, mevcut gömme yöntemiyle devam etmeyi açıkça seçti. Son gönderilen
PNG208x208 piksel,3840 bayt; SHA256:
`df06a4efdf98a12f430753ac8d2629e10445e926725bc0620026c7e299636e0e`.
`resources/branding/surge.png` bu dosyanın byte-byte kopyasıdır.

Mevcut generator dosyanın tamamını private read-only C++ dizisine dönüştürür.
Pikselden yeniden çizim, yeniden PNG encode etme veya PE PNG resource ekleme
yoktur. Normal başlangıçta profil PNG yerel kullanıcı veri klasörüne yazılır;
CFG User/Avatar seçimi bu profil yazımını önceki gibi atlar. Özgün3882 baytlık
fallback ve ayrı default.png yazımı korunur. Haricî avatar zorunlu değildir.

Uygulama, test, generator, vendor ve export kaynak kodlarında değişiklik yoktur.
Yalnız görsel girdisi ve buna ait güncel kayıtlar yenilenir. Önceki TLS/temizlik
ve tahsis-enjeksiyonu test düzeltmeleri korunur. VirusTotal Files Dropped
farkının nedeni bu değişiklikle açıklanmış veya giderilmiş sayılmaz.

[Doğrulama](evidence/avatar-update/VALIDATION.json) yerel çapraz derleme/Wine
sonuçlarını kaydeder. Yeni görseli içeren binary için native Windows/MSVC, Golden
ve oyun kabulü bu yerel testlerden çıkarılamaz. Derleme kaynak paketiyle yapılır;
bu çalışma dist çıktısını güncellemez veya yayımlamaz.

Yerel sonuç: ilgili Release4/4 ve Debug2/2 test geçti. Gerçek DLL üç kez
yüklenip başlatıldı, LSX profil yanıtı alındı, kapatılıp unload edildi. DLL’in
diske yazdığı profil PNG yeni girdiye, fallback özgün girdiye byte-byte eşit.
Raw DLL’de yeni PNG mevcut; önceki profil PNG yok ve PNG PE resource yoktur.
Release uygulama testinin temiz veri klasöründe log oluşmadı; global eski
Debug logunun zaman damgası DLL çalıştırmasından önceki değerinde kaldı.

Kaynak QA17/17 ve vendor regresyonları19/19 geçti; atlanan vendor testi yok.
