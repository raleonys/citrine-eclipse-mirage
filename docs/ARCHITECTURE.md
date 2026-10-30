# Uygulama ve araç sahipliği

Bu görev runtime mimarisini yeniden tasarlamaz. Mevcut RE uygulama kaynakları
`src/{config,core,runtime,net,platform,protocol,router,handlers,codec,hash,xml}`
altında; başlıklar `include/preloader_l/` içindedir. CMake kaynakları açık listeler,
araştırma/frozen köklere glob veya include yolu açmaz.

`tools/` ve `scripts/` operatör/QA katmanıdır; uygulama DLL'ine Golden Python,
PowerShell veya test fixture'ı bağlanmaz. Testlere ait sahte OS sağlayıcıları
yalnız test hedeflerinde kalır. First-party namespace `preloader_l` korunur.

Özgün hedef mimarisinin temel sınırları korunur: ABI ingress, application/domain,
dar ports, Windows/transport/storage adapter'ları, ayrı LSX/XML framing ve
bootstrap/teardown owner'ı. Mevcut RE fonksiyon koleksiyonunun bu hedeflerin
tamamını gerçekleştirdiği iddia edilmez.

Otomatik Application/Service owner, Asio/filesystem üretim bağları ve koşullu
11-hook rollback/drain yolu uygulanmıştır. DllMain hazırlığı dışında ağ,
config, hook ve beklemeli teardown worker üzerinde çalışır. Uygulamanın altı
C export'u ile C++ geliştirme DLL'i ayrıdır; [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md)
güncel sözleşmeyi açıklar. Gerçek Windows/native quiescence ve oyun kabulü
ayrı kanıt ister; Wine testleri bunları tamamlamış sayılmaz.

Mevcut Config2/VDF byte grameri PEGTL ile `preloader_l_vdf_objects` sahibi altında
derlenir; gerçek DLL ve DocumentPipeline bu parser'ı kullanır. Kaynak/action ve
regresyon sınırları [VDF_PARSER.md](VDF_PARSER.md) içindedir.
