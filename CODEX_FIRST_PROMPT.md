# Yellowfield çalışma girişi

Önce `AGENTS.md`, `governance/CURRENT_SCOPE.md`, `README.md` ve
`metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json` dosyalarını oku.
Özgün hedefler `metadata/INTEGRATION_REQUIREMENTS.json` içinde korunur.
Aktif durum, ilk governance paketinin NOT_STARTED kayıtlarıyla karıştırılmaz.

Fonksiyonların kaynağı kullanıcı tarafından sağlanan Reverse Engineering ağacıdır.
Brownfield uygulama kodu alınmaz; dokümantasyon, Golden, root, metadata,
resources, scripts/build, uyumlu testler ve araç altyapısı temiz uyarlanır.
Bluefield son kullanıcı talimatıyla 11. hook ve gerekli native hook ömrü için
seçilmiş donor, diğer modüller için karşılaştırmadır. Greenfield/Redfield yalnız
inceleme/karşılaştırmadır. Tam Brownfield metadata profili de son talimatla seçildi;
docs/evidence/HOOK_ALIGNMENT_STATUS.json ve docs/REFERENCE_METADATA_AUDIT.md geçerlidir.
11. hook için daha önce verilen kullanıcı talimatı geçerlidir.

DLL otomatik Application başlangıcını ve sabit beş C export'unu sağlar;
geliştirme C++ API'leri ayrı DLL'dedir. Güncel arayüz PUBLIC_DLL_ABI.json içindedir.
548-byte ABI iddiası kanıt bulunamadığı için geri çekilmiştir. Gerçek Windows/MSVC
ve oyun kabulü yapılmadı; kaynak/Wine veya Golden fixture testi oyun kanıtı değildir.
Library senkronizasyonu/yayın ve frozen kaynaklarda değişiklik yapma.
