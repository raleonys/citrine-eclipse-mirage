Kaynak çalışması: 2026-11-03. Derleme/test/link yapılmadı.

REA6.8.0, Ghidra12.1.4 ile özgün anadius64.dll statik incelendi.
Hedef SHA256: 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

* Vtable VA180084928'de ilk üç QWORD: 180017a30,1800228c0,1800228c0.
* 024db0 içindeki024e28..024e33 payload+10'daki tablonun slot0'ını çağırıyor.
* 017a30 baytları C2 00 00, Ghidra assembly: RET0. Yalnız bu seçili tablonun
  temizliği için etkisiz dönüş kullanılabilir; başka executor tabloları reddedilir.
* 0228c0..0228d0 talimatları:
  MOV RAX,[RDX]; MOV [RCX],RAX; MOV EAX,[RDX+0C]; MOV [RCX+0C],EAX;
  MOV [RCX+18],RCX; RET. Context ve dword kopyalanır, hedef kendini gösterir.
  Kaynak sıfırlanmaz, diğer tablo alanları bu leaf içinde kopyalanmaz.

Ghidra0228c0'da procedure tanımı sunmadı. Kör decompile tekrarı yapılmadı;
adres bazında altı decoded instruction ve48byte okuması kullanıldı. Yeni
typed kaynak gövdesi `move_executor_0228c0`, gerçek native sınıf/return-register
veya genel ASIO ABI eşitliği iddiası taşımaz. Vtable sayıları çalıştırılabilir
özgün fonksiyon işaretçileri olarak kullanılmaz.

`select_recovered_socket_executor`, bu taşıma efektini Runtime'a ve yalnız
084928 cleanup sağlayıcısını Server'a bağlar. Mevcut sahipli Resources testinin
kaynağı artık önceki sahte Executor/move callback'ini kullanmıyor.

OwnedServer'ın özel CloseContext'i null motorla oluşuyor ve mevcut seçili
kaynakta motor kurulum API'si yok. Bu nedenle boş dalda sahte CancellationEffects
zorunluluğu kaldırıldı. Null sağlayıcı + canlı motor + geçerli socket birleşimi
gate/cancel/close/reset/owner-release işlemlerinden önce reddedilir. Mevcut
referanslı cancellation sağlayıcı yolu korunur. Bu değişiklik genel cancellation
kuyruğu algoritmasının veya harici kernel borrow/drain sözleşmesinin kapanışı
değildir; gerçek CancelIoEx SDK adaptörü de zorunlu olduğu yerlerde korunur.

Kanıt: selected-executor.native.json (17 kayıt), selected-executor.session.json.
Snapshot: selected-executor.snapshot.json. Provider oturumu kapatıldı.
İki üretim loader hook'u, senkron loader profili ve gerçek lifecycle/drain
bağlantısı hâlâ açık. Yeni test senaryoları yalnız kaynak olarak eklendi.
