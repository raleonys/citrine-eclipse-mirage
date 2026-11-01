# Locale nesnelerinin serbest bırakılması

`locale_free.cpp`, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` içindeki altı normal akış gövdesinin okunabilir C++ yeniden yazımıdır. Özgün DLL çalıştırılmaz. Ghidra'nın kütüphane adlandırması, özgün kaynak veya derleyici kimliği kanıtı değildir.

| Giriş VA | Yeniden yazılan davranış |
| --- | --- |
| `0x1800706c0` | Locale lconv/ctype/time ve altı kategori sahiplik dalları, en son locale nesnesinin free çağrısı |
| `0x180070a40` | Lconv monetary alanlarında 13 ayrı başlangıç pointer karşılaştırması |
| `0x180071068` | Lconv numeric alanlarında 5 ayrı başlangıç pointer karşılaştırması |
| `0x180070860` | NULL/başlangıç time nesnesi koruması ve `+0x15c` sıfır refcount şartı |
| `0x180071730` | Time nesnesindeki 87 pointer alanının özgün sırada serbest bırakılması |
| `0x1800713bc` | Pointer dizisinin 64 bit modüler bitiş adresine kadar ilerlenmesi |

`Effects` yalnızca adres okuma, DWORD refcount okuma ve `free_0668c0` sınırlarını sağlar. İç içe monetary/numeric/time yardımcıları gerçek yeniden yazılan kaynak fonksiyonlarına bağlanır. Adresler sağlayıcıya ait depolama kimlikleridir; C++ bunları gerçek pointer gibi dereference etmez. Native heap, DLL yükleme veya özgün fonksiyon pointer çalıştırması yoktur.

`Defaults` statik nesne kimliklerini ve lconv başlangıç pointer değerlerini taşır. Değerler topluca kopyalanmaz; her karşılaştırmada ilgili slot yeniden okunur. Free etkisinden sonraki locale alanları da makine kodundaki sırayla yeniden okunur. `+0x108/-0xfe`, `+0x110/-0x80`, `+0x118/-0x80` hesapları işaretsiz 64 bit modüler aritmetiktir. Leaf helper alanı NULL ise ve başlangıç değeri farklıysa free(NULL) etkisi korunur; `free_locale` kendisinde NULL koruması bulunmaz.

Time pointer alanlarında `+0x158` slotu atlanır; `+0x15c` refcount yalnızca if-unreferenced girişinde okunur. `free_time` iç pointerları bırakır; nesnenin kendisini wrapper bırakır. Hiçbir free işlemi alanları temizlemez, refcount azaltmaz veya yinelenen pointerları tekilleştirmez.

Helper sayısı Ghidra'daki signed görünümden daraltılmaz: `LEA [RCX+RDX*8]` tam 64 bit RDX kullanır. `count=2^61` sıfır iterasyon, `2^61+1` bir iterasyon üretir. Olağan pozitif 7/12/2 çağrılarının yanında bu endpoint davranışı da test edilmiştir. Çok uzun geçerli dolaşımlar için yapay bir üst sınır eklenmez.

`build.ps1`, sabit yerel LLVM-MinGW derleyicisiyle bir üretim nesnesi ve yalnızca sentetik sağlayıcı kullanan test EXE'si üretir. Testler 8.192 monetary ve 32 numeric alias kombinasyonunu, 81 lconv sahiplik bileşimini, altı kategoride 729 dallanma bileşimini, 87 time pointerının tam sırasını, NULL/nonzero refcount korumalarını ve free sırasında değişen alan/başlangıç değerlerini kapsar. Sonuç `validation.json` içindedir. Özgün erişim hataları, SEH temizliği ve ABI eşitliği doğrulanmış değildir.

`collect-evidence.ps1`, yalnızca özgün DLL ve saklanan assembly/pseudocode SHA değerleri eşleştiğinde altı fonksiyonu sınırlarıyla çıkarır. Her iki REA Evidence kimliği ve seçilen dosyaların SHA değerleri `evidence.json` içindedir. Kaynak kanıtlar `selected-functions.asm.txt` ve `selected-functions.pseudo.c` dosyalarındadır.
