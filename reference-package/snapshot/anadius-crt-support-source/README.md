# CRT SList, stdio seçenekleri ve TLS slotu

`support.cpp`, anadius64.dll içindeki yedi küçük başlangıç/temizlik gövdesini C++20 olarak kurar. Kaynak SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Bu kısmi kaynak modülüdür; tam DLL değildir.

| Özgün VA | Kaynak davranışı |
|---|---|
| `0x1800067e0` | Printf qword seçenek hücresinin adresi |
| `0x1800067f0` | Scanf qword seçenek hücresinin adresi |
| `0x18004c1fc` | Printf seçeneklerine `0x24`, sonra scanf seçeneklerine2 ekleyen qword OR |
| `0x18004c218` | TLS callback içeriğinin bulunduğu slotun kararlı adresi |
| `0x18004c1e0` | Ortak SList başlığını initialize eden native importa tail aktarım |
| `0x18004c1f0` | Aynı başlıkla aşağıdaki liste cleanup gövdesine tail aktarım |
| `0x18004d2f8` | Bir kere flush; her node için Next'i free öncesinde okur ve zinciri serbest bırakır |

`State` yeni caller-owned C++ hücreleri sağlar. Printf `0x1800a37d0`, scanf `0x1800a37d8` ve TLS slotu `0x1800a5a70` image başlangıcında sıfırdır; `initial-globals.json` raw/virtual mapping ayrımını korur. SList başlığı `0x1800a4d70` virtual zero-fill alanındadır; kaynakta bu VA pointer'a çevrilmez. `Head` token'ı doğru hizalı ve canlı provider storage'ına bağlanmalıdır. Aynı token bütün çağrılarda aynı başlığı temsil eder.

Flush sonrası eklenen yeni düğümler aynı cleanup çağrısında tekrar flush edilmez. Next, free etkisinin node storage'ını bozmasından önce yakalanır. Native atomic/interlocked semantiği zorunlu provider'a aittir; sentetik fixture bu concurrency semantiğini kanıtlamaz. Liste geçerli, sonlu ve acyclic olmalıdır; geçersiz pointer/zincir için özgünde bulunmayan toparlanma eklenmedi. Provider exception'ı sırasında yeni RAII/unwind cleanup çıkarılmadı.

`./build.ps1` 4.438 sentetik kontrolü geçti. Boş/1–64 node, sırayla Next/free, cleanup sırasında yeni head ekleme, provider hata sırası, yüksek32bit seçeneklerin korunması ve kararlı slot kimliği kapsanır. `review.json` bağımsız assembly incelemesidir; `evidence.json` yedi fonksiyonun REA Evidence kimliklerini tutar. `collect-evidence.ps1` hash kontrolüyle yalnız dosya okur ve saklanan kanıtları keser. Özgün/yeniden derlenen DLL, oyun ve native SList API'leri çalıştırılmadı.
