# GF01 — Bayt-identical modül aktarımı (NOT_RELEASE)

GF00 tam kaynak ZIP'i GF01'in immutable parent'ıdır. HF19 dosyaları gerçek kaynaklardan doğrudan, satır/byte değişikliği olmadan `adopted_hf19/` altına alınmıştır; `${SOURCE}` prefix'i dahil aynı kaynak yolu korunmuştur. Kodun ownership/ABI/lisans kökeni metadata/CODE_REUSE_LEDGER.json içindedir. `licenses/HF19_LICENSE` aynen korunur.

## Gerçek durum
- `hex_codec.cpp`, `fnv_hash.cpp`, `port_mapping.cpp` doğrudan derlenen `gf_adopted` kütüphanesinde bulunur; `checked_math`, `byte_semantics`, `winsock_version_policy`, Asio session timeout ve command-line tokenizer başlıkları taşındı.
- Bu kütüphane derlenir ama public `SURGE` ve `DllShutdown` henüz bu modülleri çağırmaz. Linker ölü kod elemesi nedeniyle GF01 DLL görüntüsünde bazı fonksiyonlar bulunmayabilir; **aktif servis değil, kaynak ağacına alınmış modüllerdir**.
- Port probe varsayılanı tüm port denemeleri başarısız olursa 3227 döndürür: bu **socket bağlandığı anlamına gelmez**; GF01'de port açılmaz.
- Komut satırı tokenizer'ı borrowed span döndürür. Windows CommandLineToArgvW ile aynı quote grammar'ını garanti etmez. Yaşam süresi kanıtlanmadan runtime'a bağlanmayacak.
- FNV signed-byte biçimi tarihsel model uyumu içindir, standart FNV'nin plain uint8 XOR sürümü olduğu iddia edilmez.
- Timeout policy yalnız saf chrono testinde çalışır; gerçek Asio IOCP session sahipliği entegre edilmedi.
- `ResourceOwners` GF01 yeni sahiplik *modelidir*: cleanup `false` olduğunda ticket korunur, yeniden drain edilebilir; lock callback çevresinde serbest bırakılır, concurrent reentrant drain/adopt yasaklanır. Henüz gerçek FreeLibrary/HANDLE cleanup, DllShutdown veya Asio'ya bağlanmaz.

## Kabul kapısı
GCC ve Clang ASan/UBSan üç portable test (GF00 lifecycle, adopted modules, owner retry), bir clean-extract koşusu; SHA-parent, binary exact source equality, manifest, tam CRC, tamper rejection, deterministic archive. Windows/MSVC `/MT`, LoadLibrary/exports, CRT, game/native acceptance ve real LSX NOT_RUN.

Bu yeni geliştirme hattı HF19 canonical'ı değiştirmez; yayın ve sürüm numarası üretmez.
