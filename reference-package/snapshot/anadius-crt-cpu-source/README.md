# CRT CPU seçim kaynağı

`cpu.cpp`, anadius64.dll içindeki `0x18004b26c` yordamının CPUID/XGETBV kararlarını C++20 olarak yeniden kurar. Dosya SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Bu bağımsız derlenebilir bir kaynak modülüdür; tam DLL veya özgün kaynak değildir.

`assembly-evidence.txt` adresli özgün REA assembly dökümünün bu fonksiyona ait kesitidir. Assembly Evidence `ev_4c8b01f035bc41c396a4aa4aca013d8f009f82716778db198ba44b422b71da6d`; pseudocode Evidence `ev_f3773edcf62992a2d8fa4ced76cc2440c73d7627be11988e074de50fc5db9f86`. `evidence.json` iki tam dökümün hashlerini korur. `collect-evidence.ps1`, özgün dosyanın SHA kontrolünden sonra yalnız bounded PE okumaları ve bu saklanan kanıtların kesitini üretir; hedefi çalıştırmaz.

Akış CPUID0 ve koşulsuz CPUID1 ile başlar. Intel vendor eşleşmesinde iki qword yazılır ve maskelenmiş model kümesi flags bit0'ı belirler. CPUID7 için maksimum leaf karşılaştırması signed CMP/JL'dir. EBX bit9, daha önce alınan flags snapshot'ına bit1 ekler. Leaf1 ECX bit20/27/28 ve XCR0 maskeleri sırasıyla seviye1/2/3/5/6 ile enabled2/6/0xe/0x2e/0x6e değerlerini seçer. XGETBV yalnız bu guard'lar sağlandığında sorulur. Fonksiyon EAX0 döndürür.

Image başlangıç değerleri `initial-globals.json` içinde, çalışma sırasındaki yazılardan ayrı kaydedildi:

| Alan VA | Genişlik | PE başlangıcı | Kanıt |
|---|---|---|---|
| `0x1800a1550` | DWORD | 1 | raw `0x9fb50` |
| `0x1800a1554` | DWORD | 2 | raw `0x9fb54` |
| `0x1800a1558` | QWORD | `0x80000` | raw `0x9fb58` |
| `0x1800a1560` | QWORD | `0x2000000` | raw `0x9fb60` |
| `0x1800a4788` | DWORD | 0 | `.data` sanal zero-fill; dosyada raw baytı yok |

`State` caller-owned yeni bir C++ modelidir; varsayılanları sıfırdır ve özgün image başlangıcını otomatik uygulamaz. Image durumuyla başlamak için açıkça `State state{1, 2, 0x80000, 0x2000000, 0};` verilir. Intel eşleşmesi olmadan qword alanları korunur; bu nedenle doğru başlangıç, son DLL global deposu bağlanırken gereklidir. PE başlangıcı ile Intel dalının `0x8000`/tüm-bir runtime yazıları farklıdır.

`./build.ps1` mevcut araç zinciriyle derler ve 10.373 sentetik kontrolü çalıştırır. Vendor/model sınırları, signed leaf, XCR0 düşük8bit matrisi, ayrı guard'lar, yüksek32bit ve snapshot sırası kapsanır. `review.json` bağımsız assembly incelemesini, `common-validation.json` ortak 68 nesne/53 grup derlemesindeki sonucu tutar. Üretim gövdesi yalnız zorunlu register sağlayıcılarını çağırır; testlerde gerçek CPUID/XGETBV sağlayıcısı yoktur. Özgün DLL/EXE çalıştırılmadı. Native CPU keşfi, packed global/ABI ve SEH eşdeğerliği hâlâ açıktır.
