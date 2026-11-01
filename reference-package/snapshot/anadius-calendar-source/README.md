# 041ff0 takvim / epoch aritmetiği

`epoch_041ff0`, DLL içindeki saf takvim hesabının C++20 kaynağıdır. Girdi, year-since1900 ve zero-based month içeren dokuz mantıksal32bit `tm` alanıdır. Yalnız second/minute/hour/day/month/year okunur; weekday, yearday ve daylight kullanılmaz. `BrokenDown`, özgün CRT bellek düzenine cast edilecek native yapı değildir.

Assembly'de year+1900, ay normalleştirmesi, era/ay/gün hesapları ve day eklemesi32bit register'larda yürür. `0420f1 CDQE` sonrasında imzalı gün sayısı64bite genişler; saat/dakika/saniye de MOVSXD ile eklenir. Kaynak signed overflow UB kullanmaz: add/sub/mul açıkça uint32 bitleriyle wrap olur. Bölmeler signed truncation-toward-zero olarak kalır. Bu, bazı negatif yıllarda standart floor-era proleptik Gregorian algoritmasından farklı sonuç verir; alışılmış civil-date algoritmasıyla değiştirilmedi.

Özellikle `month=INT_MIN` durumunda `11-month` önce32bit wrap olur, sonra12'ye bölünür. Geçersiz ay/gün veya saat alanları reddedilmez; kayıttaki aritmetik uygulanır. Gün hesabı32bit wrap olduktan sonra sign-extend edilir. Son64bit clock çarpımları bütün32bit girdiler için taşmaz. Zaman dilimi, DST, localtime, mktime, timegm veya OS çağrısı yoktur.

`build.ps1` ile **31.760 kontrol** geçti. Kapsam: Unix epoch/pre-epoch,2000/leap/century,2038sonrası,day0 ve düzensiz h/m/s,pozitif/negatif ay normalleştirmesi,negatif yıl golden sonucu;25pozitif yılın aybaşı/ortası/sonu için bağımsız C++chrono karşılaştırması;30.000 tam32bit deterministik örnek ve sınırlardaki signed multiply-high/shift instruction-oracle karşılaştırması. Oracle, kaynak bölmelerini kullanmak yerine assembly'nin register programını ve magic multiplier'larını uygular. Özgün DLL çalıştırılmadı; runtime binary differential iddiası yoktur.

API `calendar.hpp`, üretim `calendar.cpp`, test `calendar_tests.cpp` içindedir. Derleme C++20/LLVM-MinGW `-O2 -Wall -Wextra -Werror`; CMake tanımı da sunulur, doğrulanan yol PowerShell'dir. Adresli tam retained pasajlar `evidence.asm.txt` / `evidence.pseudo.c.txt`; Evidence kimlikleri ve hash'ler `evidence.json`; sonuçlar `validation.json` içindedir.

Bu modül yalnız041ff0aritmetiğini karşılar.041e10string/get_time ayrıştırması ve native STL/locale/allocator/SEH/ABI eşitliği burada çözülmüş sayılmaz. CRT kaynakları ve önceki üretim kaynakları değiştirilmedi; kullanıcı CFG/token veya achievement dosyaları okunmadı.
