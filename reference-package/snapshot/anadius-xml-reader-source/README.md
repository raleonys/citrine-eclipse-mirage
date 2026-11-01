# Çalışan XML okuyucu uyumluluk bağlayıcısı

`reader.cpp`, `messages::XmlBackend` ve `dispatch::NodeAccess` arayüzlerini gerçek XML ayrıştırmasıyla karşılar. Hazır ağaç döndüren test fixture'ına ihtiyaç duymaz. Kullandığı kaynak, resmi [tinyxml2 11.0.0](https://github.com/leethomason/tinyxml2/tree/9148bdf719e997d1f474be6bcc7943881046dba1), commit `9148bdf719e997d1f474be6bcc7943881046dba1` sürümüdür. Bu seçim bir **uyumluluk bağımlılığıdır**; özgün DLL'deki tinyxml2 sürümü/commit'i kanıtlanmadı. Upstream kaynak kodu tersine mühendislikle kurtarılmış özgün kaynak gibi sunulmaz.

`vendor/tinyxml2` içinde resmi header, uygulama ve lisans vardır. Header **değiştirilmiştir**: derinlik sabiti 500 yerine 100'dür. Değişiklik yerinde işaretlidir; özgün/yerel hash'ler ve indirme URL'leri `upstream-provenance.json` içindedir. Kaynağın zlib lisans bildirimi korunur.

## Özgün kanıtla eşlenen davranış

Özgün anadius64.dll hash'i `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için ctor `0x180001830`, `processEntities=true`, whitespace=0 kullanır. Parse `0x180004280`, uzunluk=-1 ile ilk NUL'a kadar kopyalar; erişim yolları `Attribute 0x180002cc0`, `FirstChildElement 0x180003300`, `NextSiblingElement 0x180003fe0`, `GetText 0x180003860` ve `ErrorStr 0x1800032d0` olarak eşlendi. `0x18000499e` derinliği artırır; `0x1800049a4` tam 100 ile karşılaştırır. Document seviyesini de saydığı için 98 iç içe element başarılı, 99 element hata18 üretir.

Detaylı 22 fonksiyon kanıtı, 16 instruction noktası ve 19 ham hata kodu, `../anadius-xml-source/reader-evidence.md` ve JSON dosyasındadır. Bu karşılaştırma seçilen yolları destekler; tüm parser komutlarının eşitliği değildir.

Bağlayıcı entity işleme ve whitespace korumayı seçer; attribute sırasını korur; eksik ve boş attribute'u ayırır. `GetText` comment'leri atlar, ilk uygun text/CDATA düğümünü döndürür; iç içe elementleri veya metin parçalarını birleştirmez. Null metni boş string'e dönüştürmez. Arayüzün tarihsel `error_name()` adı altında **ayrıntılı ErrorStr** döner; yalnız enum ErrorName değildir.

Her Parse çağrısı kendi document'ini ve tamponunu tutar. Wrapper adresleri document ömrü boyunca sabittir; sonraki ayrı Parse çağrıları önceki belgeyi bozmaz. Düğüm/string view/pointer'lar ait oldukları ParseResult.document yok edildikten sonra kullanılmamalıdır. Başka backend'den düğüm NodeAccess'e verilirse entegrasyon hatasıdır. Özgün retry aynı document üzerinde Clear kullanır; yeni document sahipliği bağlayıcının tasarımıdır, özgün ABI iddiası değildir.

`printer()` yeniden yazılan varsayılan `MemoryPrinter` döndürür. Upstream XMLPrinter çıktısına geçilmez. Böylece gerçek okuyucu, kurtarılmış dispatcher/işleyiciler, özgün çıktıya göre yazılmış serializer ve AES aynı akışta bağlanır.

## Derleme ve doğrulama

`build.ps1`, mevcut LLVM-MinGW ile C++20 kaynaklarını derler ve yalnız bizim test EXE'mizi çalıştırır. Testler BOM, whitespace/LFCR/CRLF normalizasyonu, entity'ler, attribute sırası, nullable/boş metin, CDATA/comment, tampon sahipliği, NUL sınırı, parser hataları ve derinlik eşiğini sınar. Güncel sonuç `validation.json` ve ortak `../source-workspace/build-record.json` içindedir.

Ortak yönlendirici testi gerçek XML metni→okuyucu→dispatcher→işleyici→serializer→AES yolunu, şifreli girişin gerçek error8 üzerinden algılanmasını ve error7 log/retry yolunu da çalıştırır. Kullanılan uygulama durumu ve OS/provider/dosya sonuçları sentetiktir. Soket açılmadı, özgün DLL/EXE veya yeniden derlenmiş DLL yüklenmedi/çalıştırılmadı.

Özgün sürüm/vendor değişiklikleri, tüm hatalı Unicode/entity/declaration uç durumları, allocator/EH/SEH, MSVC nesne/SSO yerleşimi ve parser'ın tam çalışma zamanı eşdeğerliği açık kalır. Bu klasörün çalışması tek başına tüm DLL'yi tamamlamaz.
