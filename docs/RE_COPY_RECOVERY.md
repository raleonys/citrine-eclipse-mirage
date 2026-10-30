# Orijinal genel dosya kopyalama yardımcısının kurtarılması

Kullanıcı talimatı: “orijinal dll den o kismi çözemiyormusun”. Hazırlanmış
RE content_handlers kaynağı yalnız copy_overwrite(mode2) etkisini tanımlıyordu.
Orijinal DLL dökümü ise048d20 genel gövdesini,04895c dosya kimliği yardımcısını
ve0489f4 son yazma zamanı sorgusunu içerir. Bu turda bu üç gövdenin gözlenen
kararları doğrudan pseudocode ve assembly üzerinden yeniden oluşturuldu.

| Seçenek (options & 0xF) | Gözlenen davranış |
|---|---|
| 0 | Önce CopyFileW(fail_if_exists=TRUE); sonucu ve hata kodunu döndürür. |
| 1 | Hedef mevcutsa iki handle açar (share=READ + WRITE), kimlikleri karşılaştırır. Farklı dosya skip sonucu false/0; aynı dosya false/32. |
| 2 | CopyFileW(fail_if_exists=FALSE); diğer seçenek dallarına girmez. |
| 4 | Hedef mevcutsa share=READ ile açar; signed64-bit son yazma zamanları karşılaştırılır. Kaynak daha yeniyse önce hedef ve kaynak handle’ları kapanır, ardından overwrite yapılır. Aksi halde kimlik kontrolü yürür. |
| Diğer düşük nibble değerleri | Orijinaldeki gibi reddedilmez; share=READ ile kimlik dalına gider. Yüksek option bitleri yok sayılır. |

Dosya kimliği helper’ı önce GetFileInformationByHandleEx(FileIdInfo=0x12,24byte)
çağırır. Yalnız hata50/87 için GetFileInformationByHandle fallback’i çalışır.
Legacy sonuçta volume32->64 genişletilir; indexHigh dört bayt, indexLow dört
bayt olarak yerleştirilir ve kalan sekiz bayt sıfırlanır. Diğer hatalar aynen
döner. Zaman helper’ı FileBasicInfo sorgusunun 0x28 baytlık sonucundan LastWriteTime okur.

Handle’lar hedef -> kaynak sırasıyla kapatılır. CloseHandle başarısızlığı
orijinaldeki gibi abort ile biter. Kaynak/target açma, zaman ve kimlik sorgusu
hatalarında kısa devre sırası ve temizlik korunur. INVALID_HANDLE_VALUE ile
GetLastError=0 uç durumu yeni bir hata olarak normalize edilmez.

Uygulama kodu [file_copy.cpp](../src/platform/file_copy.cpp) ve gerçek SDK sınırı
[file_copy_windows.cpp](../src/platform/file_copy_windows.cpp) içindedir.
[Kaynak arayüzü](../include/preloader_l/platform/file_copy.hpp) bool/error alanlarını
açıkça taşır; orijinal derleyicinin tanımsız üç padding byte’ını veya eski C++
binary ABI’sini yeniden üretme iddiası yoktur. Özel CRT/heap kaynakları ürüne
alınmadı; diğer std::filesystem/fstream yolları ve dört vendor korunur.

Avatar adaptörü artık CopyFileW’ye doğrudan kısayol yerine bu genel gövdeyi
mode2 ile çağırır. Normal avatar davranışı hâlâ orijinal mode2 dalıdır. Diğer
seçeneklerin tamamı kaynakta mevcuttur; legacy dosya kimliği çağrısı gerçek
helper’a bağlıdır. Güncel Release geliştirme DLL’inde GetFileInformationByHandle,
GetFileInformationByHandleEx ve CopyFileW statik importları doğrulandı.
Bu, orijinal davranıştan farklı bir kimlik sorgusunu her avatar kopyasına
zorlamak veya sahte import üretmek değildir.

Testler: seçenek maskesi, bilinmeyen bit kombinasyonları, signed tarih uçları,
kimlik byte’larının tamamı,50/87 fallback ve diğer hata kodları, kısa devre ve
fatal kapatma sıraları kontrollü testlerle; Unicode/binary dosyalar, skip/overwrite/
update, aynı dosya, hard link ve gerçek metadata sorguları Windows test EXE’siyle
sınanır. Sonuçlar docs/evidence/RE_COPY_RECOVERY.json içindedir. Wine/cross-build,
gerçek Windows/MSVC Golden ve kullanıcı oyunu kabulü ayrı kanıt sınıflarıdır.
