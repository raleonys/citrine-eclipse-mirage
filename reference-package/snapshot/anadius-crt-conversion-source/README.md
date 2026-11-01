# Dönüşüm tamponları: 06004c / 06e430

`conversion.cpp`, dar metinden UTF-16'ya `0x18006004c` ve UTF-16'dan dar metne `0x18006e430` tampon yönetimini yeniden yazar. Yeni `Buffer` tipi özgün nesnenin `+10/+18/+20/+28` alanlarını temsil eder; tam nesne düzeni veya ABI eşitliği iddiası yoktur. Kullanılmayan üçüncü özgün parametre yeni kaynak arayüzünden çıkarıldı.

Kaynak önce bir kod birimi okuyarak boş dizgeyi ayırır. Boş olmayan dizgede gerekli boyut sorgusu, gerekirse eski sahip olunan tamponun bırakılması, malloc, ikinci dönüşüm ve uzunluk yazımı sırasını korur. UTF-16 hedefinde kapasite kod birimi, tahsis boyutu bayttır. Kapasitesi yeterli ödünç tampon kullanılır ve serbest bırakılmaz. Büyütme sırasında eski sahip olunan tampon yeni tahsis başarısız olsa da bırakılmıştır; önceki uzunluk bu hata yolunda korunur.

Sorgu ve doldurma sonuçları assembly'de işaretli 32 bitten 64 bite genişletilir. Sorgu sonucu kapasiteyle **işaretsiz** karşılaştırılır. Doldurmada kapasitenin alt DWORD'u geçirilir. Sıfır dışındaki negatif sonuçlar da sıfır testini geçtiğinden kaynak aynı bit davranışını korur. Gerçek Windows API'lerinin böyle sonuçlar vermesi beklenmez; sentetik sınır testleri bunu özgün çağrı kodunun davranışı olarak doğrular.

Hata yolu `GetLastError -> 05d698 -> 05d724 hücresini oku` sırasını kullanır. `05d724`, decompiler'ın yanıltıcı `__doserrno` adının aksine mevcut yeniden yazımdaki errno hücresidir. Tahsis hatası doğrudan 12 döndürür; yeni hata eşleme eklenmez. NULL kaynak dört alanı sıfırlar; boş kaynak hedefe bir NUL kod birimi yazar.

`Effects` saf arayüzdür. `06ccc8/06cd58` politika katmanları, malloc/free, hata eşleme ve geçerli sağlayıcı belleği üst bileşimde bağlanmalıdır. Burada varsayılan sahte üretim sağlayıcısı veya yerel API çağrısı yoktur. Bu modül, Windows kod sayfalarının tüm dönüşüm uygulamasını tek başına sağlamaz. SEH/unwind ve DLL eşitliği açık kalır.

`build.ps1`, mevcut llvm-mingw derleyicisiyle C++20, `-O2 -Wall -Wextra -Werror` altında bir üretim nesnesi ve yalnızca sentetik sağlayıcılarla test programı üretir. 5.370 kontrol: iki yön, NULL/boş kaynak, ödünç/sahip olunan tampon, yeterli/yetersiz kapasite, sorgu/doldurma/tahsis hataları, olay sırası, eski uzunluk ve DWORD/işaret sınırları. Orijinal DLL veya yerel dönüşüm/tahsis sağlayıcıları çalıştırılmaz.

`collect-evidence.ps1`, girdinin ve iki saklanan bütün dosyanın SHA-256 değerlerini doğrular; dört Evidence ID'nin saklanan REA kayıtlarında tekil olduğunu kontrol eder ve yalnız bu iki fonksiyonun assembly/pseudocode kayıtlarını çıkarır. Adresler, Evidence ID'ler, kesin hash'ler ve kapsam sınırları `evidence.json`; derleme ve test sonucu `validation.json` içindedir.
