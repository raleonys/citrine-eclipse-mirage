# İlk multibyte verisi

Bu modül, SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan anadius64.dll dosyasından salt okunur PE veri çıkarımıdır. Özgün DLL çalıştırılmaz veya değiştirilmez. Yeni özgün fonksiyon gövdesi sayısı **0**; veri ve mevcut kaynak reset fonksiyonu arasındaki bağlantı doğrulanır.

`extract.cjs` PE32+ bölüm tablosunu ve base relocation dizinini okuyup `extracted-data.json` ile `initial_mb.cpp` üretir. C++ arabirimi `anadius::crt_initial_multibyte` isim alanındadır. `record_bytes()`, `initial_type_table()`, `initial_case_table()` sabit dizileri; `image_globals()` dört başlangıç DWORD değerini döndürür. Özgün tercih edilen adresler çalışma zamanı C++ pointer'ı yapılmaz.

| Veri | VA | Dosya ofseti | Boyut | Relocation |
|---|---|---|---|---|
| İlk multibyte kaydı | 0x1800a1c70 | 0xa0270 | 0x228 | Yok |
| Ayrı type tablosu | 0x1800a1ea0 | 0xa04a0 | 257 | Yok |
| Ayrı case tablosu | 0x1800a1fb0 | 0xa05b0 | 256 | Yok |

Kayıt +0x18'deki 257 byte ile ayrı type tablosu; +0x119'daki 256 byte ile ayrı case tablosu birebir aynıdır. ASCII A-Z için type flag 0x10, a-z için 0x20'dir; case haritası karşı harfi verir. Her tabloda 52 sıfır olmayan byte vardır. Kayıtta bunların dışında tüm byte'lar sıfırdır: refcount +0, codepage +4, mode +8, altı WORD +0xc, padding +0x219..0x21f, son pointer +0x220. Sıfır refcount, canlı kayıt sahiplik sayacı olarak varsayılmamalıdır.

`0x1800a1c6c` locale mask değeri dosyada **0xfffffffe**'dir. Current-locale 0x1800a52e0, current-multibyte 0x1800a59d0, type/case pointer 0x1800a59c0/0x1800a59c8 ile locale-changed 0x1800a52d8, system-codepage flag 0x1800a59d8 ve initialized 0x1800a59dc loader zero-fill alanındadır. Bu sıfırlar DLL dosyasında fiziksel DWORD/QWORD sıfırları olarak bulunmaz. `extracted-data.json` her alanda raw-backed ve zero-fill byte sayısını ayırır.

İlk pointer çifti 0x1800a1938/0x1800a1940, tercih edilen imajda 0x1800a17e0/0x1800a1c70 değerindedir ve ikisinde DIR64 relocation vardır. Bunlar JSON metaverisidir. Tüm locale nesne grafiği veya canlı pointer sahipliği bu modülün kapsamı değildir.

Retained assembly `reset_sbcs_06f050` fonksiyonunun kaydın +4..0xb, +0xc..0x17 ve +0x220 alanlarını sıfırlayıp iki tabloyu kopyaladığını gösterir; +0 refcount ve +0x219..0x21f padding korunur. Test 0xa5 ile doldurulmuş bağımsız bellekte bu korunmayı da denetler. `initialize_multibyte_06f5f8` image zero-fill pointer'larını daha sonra ilk kayıt ve tablolarla doldurur; bu modül çalışan süreci gözlemlemez.

Derleme ve doğrulama: `./build.ps1`. Kurulu yerel LLVM-MinGW kullanılır; sadece kendi ürettiğimiz fixture çalıştırılır. 2908 C++ kontrolü ve bağımsız PE okuyucusuyla derlenen dizilerin 1065 byte'ı + locale mask için 1066 kontrol geçer. `validation.json` ve `byte-validation.json` sonuçları saklar. Bu testler özgün DLL ile ABI veya bütün davranış eşitliğini kanıtlamaz. `evidence.json` veri ile retained REA çağrı yeri kanıtını ayrı tutar; `provenance.json` kaynak hash'lerini kaydeder.
