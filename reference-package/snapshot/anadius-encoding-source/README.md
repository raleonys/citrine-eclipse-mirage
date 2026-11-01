# Windows dosya yolu kod sayfası ve UTF dönüşümleri

`encoding.cpp`, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` içindeki altı dönüşüm yolunu C++20 olarak yeniden kurar. Sekiz dönüşüm/hata yardımcısının assembly ve pseudocode Evidence kimlikleri `evidence.json` içindedir. Yeni kaynak özgün MSVC nesne/exception ABI'si değildir.

| Özgün VA | Kaynaktaki davranış |
|---|---|
| `0x180048ac0` | CRT locale codepage65001 ise UTF8; değilse AreFileApisANSI true→0/ACP, false→1/OEMCP |
| `0x180048ae8` | MultiByteToWideChar flags8; yalnız count0 ise GetLastError |
| `0x180048b30` | UTF8/54936 için flags0x80; diğerlerinde flags0x400 ve used-default kontrolü; yalnız error0x3ec için flags0/null-default ile retry |
| `0x180006d70` | Önce dosya codepage seçimi, boşluk kontrolü, INT_MAX sınırı, query→resize→fill |
| `0x180006e60` | Verilen codepage ile uzunluk bilgili wide→narrow; aynı iki geçiş |
| `0x18001e3f0` | Path→UTF8; codepage65001 sabit, filesystem codepage'den bağımsız |

Gömülü NUL normal kod birimi olarak uzunluğa dahildir; C-string truncation eklenmez. Çıktı uzunluğu ilk query sonucudur; ikinci çağrının döndürdüğü farklı count ile yeniden boyutlandırılmaz. Used-default hata0x459, gerçek API count'undan önceliklidir. Diğer Win32 hatalarında ek fallback yoktur. Runtime uzunluk hatası22 ve filesystem hata sayısı zorunlu `Errors` arayüzüne gider.

`native_encoding.cpp` gerçek Win32 API'lerini ve bağlanan CRT'nin `___lc_codepage_func` sorgusunu kullanır. char16/wchar tamponları byte temsiliyle kopyalanır; geçici tahsis/yıkımların API giriş/çıkış LastError durumunu bozmaması sağlanır. Seçilen üst yollar nonnegative uzunluk ve ayrı giriş/çıkış tamponu kullanır. Genel low-level negatif uzunluk/aliasing, özgün allocation sırası, static CRT/thread-locale kimliği ve ABI eşitliği bu adapter'ın iddiası değildir.

`PortableErrors` sayısal hataları seçilmiş `std::system_error` generic/system kategorileriyle taşır. Özgün MSVC error mapping, mesaj metni, vftable ve EH yerleşimini aynen ürettiği iddia edilmez. Özgün hata temsili gerekiyorsa çağıran kendi Errors uygulamasını sağlamalıdır.

`build.ps1` yalnız bizim test programımızı çalıştırır. Mock API testleri öncelik/sıra/flags/query/fill/error ve uzunluk sınırlarını; native testler bilinen UTF8/UTF16 baytlarını, NUL, bozuk UTF8, yalnız surrogate, best-fit reddini ve direct-Win32 karşılaştırmasıyla tampon/LastError davranışını sınar. Güncel kesin sonuç `validation.json` veya ortak `../source-workspace/build-record.json` içindedir. Native işlemler sentetik string'lerle sınırlıdır; özgün DLL/EXE/CFG veya kullanıcı token'ı çalıştırılmaz/okunmaz/değiştirilmez. Tam runtime eşdeğerliği doğrulanmadı.
