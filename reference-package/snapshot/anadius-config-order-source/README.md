# Config2: sınırlı hash/list ve ilk kopya sırası

`order.cpp`, mevcut parser'ın logical child kayıtlarını özgün Windows hash/list sırasına bağlayan **sınırlı** kaynak modülüdür. Her container için en fazla **8 benzersiz çocuk** desteklenir. Daha büyük container `UnsupportedOrder` ile açıkça reddedilir; bu exception özgün DLL'nin davranışı değil, tamamlanmamış büyüme algoritması için kaynak sınırıdır.

`fnv1a64`, özgün `012820/011ba0/017570` içerisindeki byte hash'ini kullanır: seed `CBF29CE484222325`, unsigned byte XOR ve `100000001B3` çarpanı, uint64 taşması. `014a90` fresh container'da mask7/bucket8/load1 kurar. `011970` ve `012820`, boş bucket için sona; dolu bucket'ta yeni benzersiz anahtar için bucket'ın ilk kaydının önüne ekler. Tekrarlanan assignment değeri ve subtree'yi değiştirir; mevcut key kaydı taşınmaz.

`parsed_order(node)` parser'ın first-created unique slot sırasından parse-final list'i üretir. `copy_order(list)` bir **fresh sekiz-bucket** kopyanın rebuild'ini üretir. `SelectedCopyOrder`, ikisini bileştirir. Caller önce parser root'tan `Config2` seçmeli, yalnız seçilen subtree'yi `to_configuration_node` ile çevirmelidir. Path parametresi stage değiştirmez; seçili subtree'nin her derinliğinde bir copy uygulanır. Lexer/source sırası veya standart kütüphane `unordered_map` sırası kullanılmaz.

İlk Config2 kopyasının `019d52 -> 0165c0 -> 0162f0` yolu, bucket sayısı değişmese de `017570` çağırır. Unique collision grubunun sırası ters döner. Yeni descendant record'ların `010bc0 -> 0132a0/014970 -> 011ba0` yolu da source list'ten yeniden ekleme yapar. Bu iki yol desteklenen sekiz-bucket sınırında aynı dolaşım sonucunu verir. Native allocator, ABI, allocation/exception timing veya node adresleri modellenmez; kısmi algoritmalar **tam özgün yordam gövdesi sayılmaz**.

**Son koleksiyon tüketim sırası henüz çözülmedi.** `019ad0` sonraki bölüm kopyalarında local destination'ı Game → Emulator → User → Content → Entitlements → Achievements boyunca yeniden kullanır. Sabit alan lookup'ları missing key ekleyerek bu destination'ın bucket kapasitesini büyütebilir. Bu nedenle Content/Entitlements/Achievements için “ilk kopya = son tüketim sırası” denemez. Bu provider'ı doğrudan tam koleksiyon eşdeğerliği iddiasıyla kullanmayın; pipeline scalar uygulama sınırı bu ayrımı korumalıdır.

`build.ps1`, LLVM-MinGW C++20 ile üretim nesnesini ve sentetik testleri derler. Testler sabit collision örneği, 720 anahtar permütasyonu, unsigned/NUL hash, duplicate replacement, nested copy, owned yaşam süresi ve 8/9 sınırını kapsar. Özgün DLL/oyun yüklenmez veya çalıştırılmaz; kullanıcının CFG/token dosyası okunmaz. `CMakeLists.txt` sağlandı; doğrulanmış komut PowerShell build'idir.

Adres, REA Evidence ID, listing SHA256 ve tam seçilmiş statik pasajlar `evidence.json`, `evidence.pseudo.c.txt`, `evidence.asm.txt` içindedir. Bunlar mevcut hash'e bağlı retained REA listing'lerinden türetilmiştir; yeni runtime kanıtı değildir.
