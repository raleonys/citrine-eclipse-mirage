# Derlenen version.dll için statik doğrulama

Sonuç: **PASS**. `version-rebuild/build-llvm/version.dll`, resmi LLVM-MinGW 20261006 dağıtımındaki Clang 23.1.3, llvm-ml ve llvm-rc ile CRT olmadan derlendi. DLL yüklenmedi veya çalıştırılmadı.

- 198 export adı ve ordinal özgün PE kanıtıyla eşleşiyor; ek export yok.
- 7 import özgün import kümesiyle eşleşiyor; tamamı `KERNEL32.dll` üzerinden, CRT importu yok.
- 197 sarmalayıcıdaki 6.556 derlenmiş komutun register'ları, operand genişlikleri, sabitleri ve sırası korundu.
- 424 işlev içi dal hedefi ve 636 çözümlenmiş RIP üzerinden bellek başvurusu karşılaştırıldı.
- 21.504 gömülü PE baytı derlenen dosyada da özgün kanıtla aynen eşleşiyor.
- PE giriş noktası ile `DllMain @3` aynı yeniden yazılmış girişe gidiyor.
- Metinsel `version.rc` ile üretilen `RT_VERSION` tür16/ad1/dil1033 kaynağının 560 baytlık payload'ı özgün kaynakla aynen eşleşiyor: SHA-256 `d61413c8dfa84eb63f499c0f3b5bc63e198538db28e9a793d86ac64403799f26`. Kaynak sayısı, ad, dil, codepage ve reserved alanları da doğrulandı. Fixed-file-info ve bütün sürüm metinleri bu tam payload karşılaştırmasına dahildir.

`compiled-pe-tables.json` derlenen PE tablolarını, `compiled-wrapper-comparison.json` her komutun özgün/yeni adres eşlemesini, `compiled-resource-comparison.json` sürüm kaynağının tam payload karşılaştırmasını, `verification-summary.json` sonucu ve sınırları içerir. Ham llvm-readobj ve llvm-objdump çıktıları proje içindeki `build-llvm/readobj.txt` ve `build-llvm/disassembly.txt` dosyalarındadır. Derleme komutları, araç sürümü, resmi arşiv kaynağı ve SHA-256 değerleri `build-llvm/build-record.json` içindedir.

Karşılaştırma, yeni yerleşimdeki adreslerin semboller üzerinden yeniden eşlenmesine ve `JE/JZ`, `JNE/JNZ` eşanlamlılarına izin verir. Çalışma zamanı davranışı gözlenmedi. C++ olarak yeniden yazılan dış `DllMain` için komut düzeyinde eşdeğerlik kanıtı verilmez; PE yerleşimi ve unwind metadatası farklıdır. Gömülü bileşen hâlâ **özgün makine kodudur**, geri kazanılmış C++ kaynak değildir. Tam kaynak kurtarımı tamamlanmış sayılmaz.

Derlenen DLL SHA-256: `c982e3ea13f0f8123899d4ab6175fe93925885f10a5c79417a39cecdcfbb5f7e`.
