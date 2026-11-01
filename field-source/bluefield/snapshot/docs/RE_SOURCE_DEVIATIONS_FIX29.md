# FIX29 — XML parse-time derinlik ve hata önceliği düzeltmesi

Karşılaştırma hedefi tam kullanıcı RE kaynak paketidir; özgün DLL runtime eşdeğerliği değildir. NON_RELEASE, exact FIX28 parent. D15 düzeltildi; açık ana inceleme8→7.

## Neden sapmıştı?

RE paketinin seçtiği compatibility TinyXML2 headerında depth100, parser PushDepth içinde uygulanır. FIX08de pristine güncel upstreami korumak için depth500 ile parse edilip yalnız başarılı document sonrasında visitor ile100 denetlenmişti. Bu, önceki entegrasyon seçimimizdi; upstream güncellemesinin zorunlu kıldığı bir davranış değildi. Kaynakta bu teknik amaç açık olsa da kullanıcı100 kontrolünün zamanını/hata davranışını değiştirmemizi istemedi. Sonradan bir kullanıcı onayı veya bağımlılık zorunluluğu varmış gibi sunulmuyor.

Geçerli belgelerin limit testleri bu farkı saklıyordu.99 açık R elementi içinde <bad a=>, source parserda18 depth error üretirken parent7 attribute error üretebiliyor. Parent ayrıca source ErrorStr yerine invented adapter diagnostic dönüyordu. Daha ilerideki syntax/allocation çalışması da gereksizce yapılabiliyordu.

## Ne değişti?

Readerın post-success visitorı, source_depth_error flagi ve özel diagnostic stringi kaldırıldı. Reader yeniden doğrudan Parse/Error/ErrorStr kullanır. Current pinned upstream cpp, src/xml/upstream_parser.cpp içinden tam bir kez derlenir. Bu TUda tinyxml2 namespaceindeki constexpr100, PushDepthin standart unqualified name lookupiyle source limitini seçer. Public global header500 sabiti değişmez. Hiçbir vendor dosyası değiştirilmedi; generated vendor fork/private-field hack yoktur. Bu adapter açıkça kendi build entegrasyonumuz olarak kaydedildi; upstreamin varsayılan davranışı diye gösterilmiyor. Gelecek explicit dependency updateinde lookup/body guard yeniden incelenmelidir.

Dependency pinleri ve current upstreamin Unicode/entity/destructor/printer/security fixleri geri alınmadı. Header/cpp referansı, paket içindeki seçilmiş compatibility dependencydir; recovered original DLL sourceu veya binary sürüm kanıtı değildir.

## Kanıt

Test-only packet h/cpp/license arşivle byte-identicaldır. Ayrı TUda namespace/depth symbol private rename yapılır; production graphına girmez. Canlı karşılaştırma4284 örnekte17138 kontrol: return code/error flag/full diagnostic/tree; derinlik98/99 sınırı, selfclose, bozuk attribute/comment/CDATA/mismatch, CRLF line, multi-root ve NUL davranışı. Input alanı sonludur; upstreamin farklı numeric entity güvenlik davranışlarını kaynak adına geri almaz.

Eski56 C++ fixture/headerın55i aynı; XML testinin invented adapter string assertionı, source ErrorStr ID18/line expectationına güçlendirildi. PASS almak için kaynak beklenen sonucu değiştirilmedi; yeni bağımsız dependency reference bekleneni üretir. Runtime122 parent dosyasının121i aynı; yalnız reader.cpp değişti ve tek yeni compile TU eklendi. Vendor1060, tüm CMD/PS1/Golden araçları ve diğer runtime politikaları aynı.

Full Debug/Production/clean ZIP CTest, negatifler ve focused sanitizer sonuçları teslimdeki final QAya bağlanır. Windows/MSVC/DLL/native/game/actual DLL Golden, full-core sanitizer ve LSan NOT_RUN. Diğer açık kayıtlar DEP05, D02, D06, D07, D13, D14, D21; D15 kapanışı bunları kapatmaz.
