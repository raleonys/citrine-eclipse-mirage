# Doğrulama sınıfları

| Sınıf | Neyi kanıtlar |
|---|---|
| SOURCE_QA | Kaynak hash/kapsam, metadata/komut sahipliği, düz paket kimliği ve negatif altyapı kontrolleri |
| PORTABLE_UNIT | Linux üzerinde taşınabilir uygulama ve host-Python altyapı testleri |
| WINDOWS_CROSS_BUILD | LLVM-MinGW ile Windows x64 nesneleri ve gerçek DLL linki |
| WINE_SOURCE_TEST | Windows executable'larının Wine altında kaynak/adapter testleri; gerçek Windows değildir |
| READ_ONLY_BINARY_INSPECTION | PE import/export/resource raporu ve kullanıcı avatarı byte eşitliği |
| WINDOWS_MSVC / WINDOWS_NATIVE | Yalnız gerçekten Windows/MSVC ile çalıştırıldığında kaydedilir |
| GOLDEN_TARGET_ACCEPTANCE | Gerçek ürün politikası ve exact DLL üzerinde zincir; fixture PASS yeterli değildir |
| CONSUMER_GAME | Adı/sürümü belli gerçek oyun ve Consumer akışı; şu anda NOT_RUN |

Komut: `python -B scripts/run_source_qa.py`. CTest altyapı testleri hem portable
konfigürasyona hem Windows test grubuna kayıtlıdır. CRT probe compiler'ın heap,
exception unwind, container/thread ve TLS destructor davranışını sınar; özel
CRT eşdeğerliği veya DLL'ler arası C++ ABI garantisi vermez.

Brownfield'den uyarlanan PE testleri bozuk optional header, raw extent/overlap,
resource recursion/fanout, ordinal-only export, forbidden CRT import, checksum
ve SecurityCookie hatalarını reddeder. Yeni testler ayrıca yanlış/private manifest,
source drift, duplicate/unsafe ZIP yolu, nested archive, symlink, write failure,
unowned metadata, kayıp komut argümanı/exit kodu ve Golden erken-durma yollarını sınar.

Güncel VDF uygulama/derleyici sonucu [VDF_PARSER_STATUS.json](evidence/VDF_PARSER_STATUS.json),
uyarlama kapsamı [BROWNFIELD_TRANSFER_MATRIX_V2.json](../metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json).
Önceki altyapı ve kaynak testlerinin ham kayıtları korunur; güncel raporlarla aynı iddia sayılmaz.

Güncel runtime bağı: [RUNTIME_BINDING.md](RUNTIME_BINDING.md). Buradaki önceki vendor/parser test sayıları tarihsel snapshot kanıtıdır; yeni app testleri [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json) içindedir.

Güncel uygulama C arayüzü ve kaynak DLL ayrımı: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Güncel doğrulama: [PUBLIC_ABI_STATUS.json](evidence/PUBLIC_ABI_STATUS.json).

Production/referans hedef ayrımı ve dosya başına kullanım denetimi:
[BUILD_PARTITION.md](BUILD_PARTITION.md). En son karşılaştırmalı doğrulama
[BUILD_PARTITION.json](evidence/BUILD_PARTITION.json) içindedir; önceki raporlar
tarihsel sonuçlarını korur.
