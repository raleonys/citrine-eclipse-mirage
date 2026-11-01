# GF13 Windows x64 native final-link gate — NOT_RELEASE

GF12 FIX01 parent exact SHA-256: `6be3fc0c6266684395047cf1a142032b94a993198b07314517306cbffa8243ab`.

Bu adım **yalnız test/derleme altyapısını** değiştirir. Mevcut `SURGE` no-op, `DllShutdown` zero-owner, `DllMain@1` / `SURGE@2` / `DllShutdown@3` aynen kalır. 232 özgün HF19 dosyası değişmeden korunmuştur. Canonical HF19 ve Library `CURRENT_ARTIFACT` değişmez.

## Windows x64 iş akışı

1. GF13 tam kaynak ZIP'ini temiz klasöre çıkarın.
2. Visual Studio x64 Native Tools Command Prompt içinde `scripts\RUN_WINDOWS_GF13.cmd` komutunu çalıştırın.
3. Çalıştırıcı, `Release /MT` ve `Debug /MTd` ile mevcut bağımsız DLL export host'u ve **özgün** `test_dispatch.cpp` + **özgün** Windows-only `avatar_path.cpp` final-link hedefini derleyip çalıştırır. Hata kodu sıfır değilse durur.
4. `gf13_original_dispatch_native` ayrı EXE/test hedefidir; shipping `gf_preloader` hedefinin kaynak veya link listesine eklenmez.
5. Native makbuz yalnız gerçek başarılı derleme, CTest, PE export ve host sonuçlarından sonra oluşturulur. Buradaki Linux hazırlığı Windows PASS değildir.

## Sınırlar

- Gerçek Win32/MSVC native toolchain bu ortamda mevcut değildir. Final-link, export, runtime IOCP, LSX, Consumer/game = `NOT_RUN`.
- GF09 özgün `GetConfig` malformed-entity testinin exit 14 uyumsuzluğu `OPEN` kalır.
- Ayrı orijinal unit-test final-link PASS sonucu production `SURGE` servis başlangıcına eşit değildir.
- ASan/UBSan Linux portable testleri Windows/IOCP testlerinin yerine geçmez.
