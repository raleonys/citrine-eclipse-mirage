# `_recalloc_base` ve `_msize_base` kaynakları

`anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için korunmuş REA/Ghidra assembly ve pseudocode kanıtlarından bağımsız C++20 gövdeleri. Özgün kaynak kodu veya ikili eşdeğerlik iddiası değildir.

| Adres | Kaynak işlev | Davranış |
|---|---|---|
| `0x1800773bc` | `msize_base_0773bc` | NULL için hata 22, `05c954` invalid-parameter çağrısı ve `UINT64_MAX`; diğer durumda heap `0a5a10`, bayrak 0 ile HeapSize sağlayıcısı. |
| `0x180070434` | `recalloc_base_070434` | `count != 0 && size > 0xffffffffffffffe0 / count` taşma reddi, hata 12. NULL olmayan eski bloğun HeapSize sonucu önce alınır, ardından gerçek kurtarılmış `realloc_base_06e3a0` çağrılır. Başarılı büyümede yalnız eski **kapasiteden** yeni istenen boyuta kadar sıfırlama. |

`HeapSize` sonucu `UINT64_MAX` ise ayrıca hata eşleme yapılmaz; unsigned karşılaştırma sıfırlamayı atlar. Sıfır boyut ve NULL olmayan blokta HeapSize, free işleminden önce gelir. NULL ve sıfır boyut, malloc çekirdeğinin bir bayta normalizasyonunu kullanır; recalloc bu baytı sıfırlamaz. New-handler tekrarı gerçek realloc/heap kaynaklarına bağlıdır; eski kapasite döngüden önce yalnız bir kere sorgulanır.

`Effects`, realloc sağlayıcılarını genişletir. Yeni HeapSize, invalid-parameter ve sıfırlama sağlayıcıları zorunludur. Caller, yeniden tahsisin koruduğu içeriğe sahip canlı bir byte dizisi ve geçerli yaşam süresi sağlamalıdır. Büyümenin C++ byte dizisi boyutu `PTRDIFF_MAX` sınırını aşması açık `domain_error` kaynak sınırıdır; özgün assembly'deki bir kontrol olarak sunulmaz. Sağlayıcı istisnaları doğrudan yayılır; SEH veya ek rollback icat edilmemiştir. Gerçek HeapSize/HeapReAlloc, DLL, oyun, özgün adres çağrısı ve hedef çalıştırması yapılmaz.

`build.ps1`, üretim nesnesini ve gerçek kurtarılmış realloc+heap kaynaklarını kullanan sentetik testleri C++20 `-O2 -Wall -Wextra -Werror` ile derler. Kapasite/boyut matrisi, taşma sınırları, NULL/sıfır davranışı, kapasite başarısızlığı, sağlayıcı sırası, heap hücresi mutasyonu, new-handler tekrarı ve istisna yayılımı sınanır. Ayrıntılar `validation.json`, `evidence.json` ve sınırlandırılmış assembly/pseudocode dökümlerindedir. Test sayısı çalışma zamanı DLL eşdeğerliğinin ölçüsü değildir.
