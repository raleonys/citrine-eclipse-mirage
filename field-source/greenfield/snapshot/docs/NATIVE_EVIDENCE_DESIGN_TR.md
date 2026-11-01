# GF14 — Windows native test kanıtının kaynağa bağlanması

**Tür:** `NON_RELEASE_WORK / NOT_RELEASE`, test/araç geliştirme. Kaynak ebeveyn: GF13, SHA-256 `e67305952326ef7ecd4db77cf503f9f38cc0cd381d92362a0dadcfff8380e17d`. Resmî HF19 SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`.

## Gerekçe

GF13 `gf13_record_native.py` makbuzundaki `source_sha256` alanı kaynak hash'i değil, `VERIFY_IN_SOURCE_ZIP_SIDECAR` yer tutucusuydu. Önceden oluşturulmuş JUnit dosyaları, tek başına çalıştırıcıyla fiziksel bağ taşımıyordu. GF14 bu eski betiği veya shipping kodunu değiştirmek yerine *ayrı* bir native çalıştırıcı sağlar. Önce GF14 kaynak manifestini doğrular, sonra aynı Python işleminde gerçek `cmake configure`, Release `/MT` ve Debug `/MTd` derleme, özgün HF19 dispatcher EXE final-link, `ctest -V` çalışması ve `dumpbin /directives` ile CRT nesne kimliğini kaydeder. `GF00_WINDOWS_EXPORT_SHAPE_PASS_NOT_RUNTIME_PARITY` çıktısı ve gerçek JUnit'deki iki test zorunludur. Her iki konfigürasyon başarılı olmadan başarılı sonuç ZIP'i yayımlamaz.

## Windows x64 çalışma

1. Tam GF14 ZIP'ini **temiz bir klasöre** çıkarın.
2. **Visual Studio x64 Native Tools Command Prompt** açın. `cl`, `cmake`, `ctest`, `dumpbin`, `python` erişilebilir olmalı.
3. `scripts\build\BUILD_NATIVE_ACCEPTANCE.cmd` çalıştırın.
4. İki konfigürasyon başarıyla biterse, kaynak klasörüyle aynı üst klasörde oluşturulan `*_GF14_WINDOWS_RESULTS_FULL_EVIDENCE.zip` paketini geri gönderin. İşlem `NOT_RELEASE` olarak kalır.

Bu akış **sahte native PASS üretmez**. GF13 orijinal dispatcher Windows final-link başarısız olursa sonuç ZIP'i `PASS` olarak oluşturulmaz; komut ve hata mesajı görünür konsola akıtılır. İşlem sırasında kaynak ağacına build çıktısı yazılmaz. Çalıştırıcının kanıt paketi SHA kontrolleri, PE32+ AMD64 export tablosu, ikili dosya hash'leri, özgün JUnit, `ctest -V` çıkışı, MSVC `/MT` ve `/MTd` COFF direktiflerini doğrular. Bu korumalar bilerek tahrif edilmiş kayıtları ayıklamaya yöneliktir; çalışan testin bütün davranışlarının formal ispatı değildir.

Linux/GCC/Clang portatif testleri Windows kabulü yerine geçmez. `SURGE` no-op, `DllShutdown` zero-owner, Win32 IOCP/LSX/Consumer/game `NOT_RUN`; GF09 `GetConfig` entity uyumsuzluğu `OPEN_EXIT14`. `gf_preloader` kaynak/link listesi, mevcut HF19 dosyaları, canonical HF19 ve `CURRENT_ARTIFACT` **değişmedi**.
