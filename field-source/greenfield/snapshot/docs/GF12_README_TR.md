# GF12 — Handshake / wire / transaction kaynak kapanışı

**NON_RELEASE_WORK / NOT_RELEASE.** Parent GF11 SHA-256 `962d4d636f8dd666e84486336d3b378346c05136f78e1879c07fa09f78b1659e`, resmi HF19 SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`.

HF19 kaynaklarından dört gerçek `.cpp` (`challenge_handshake`, `socket_exchange_frame_step`, `socket_exchange_wire`, `socket_exchange_transaction`) ve iki özgün test (`test_async_wire_scanner`, `test_handshake`) **baytları değiştirilmeden** eklenmiştir. GF01–GF12 kümülatif orijinal HF19 kaynak/test dosyası: 232.

`gf12_exchange_transitive_units` STATIC hedefi yalnız araştırma/derleme içindir. Windows'a özgü `socket_exchange_transaction.cpp` içindeki kullanılmayan yardımcı fonksiyonların Linux uyarıları görünür tutulmuş, **yalnız `-Wno-error=unused-function`** uygulanmış; orijinal dosyalar değiştirilmemiştir. `gf_preloader` ve export ABI değişmemiştir.

Gerçek GCC/Clang ASan+UBSan kaynak-bağlantılı özgün testleri: 2/2 her derleyicide PASS (wire scanner + handshake). GF11'dan gelen 12 transitive sembolün 12'si GF12 kaynaklarında tanımlıdır. Gerçek GNU ld ile özgün `test_dispatch` ve mevcut bütün ELF arşivleri link denemesi `normalized_avatar_path` sembolü nedeniyle FAIL; kaynak Windows-only `avatar_path.cpp` korunmuş, Win32 mock yok. Tam DLL / live LSX / IOCP / MSVC native **NOT_RUN**. `GetConfig` original exit14 OPEN.

Windows operatör komutu: `scripts\RUN_WINDOWS_GF12.cmd` (x64 VS Native Tools, yerel kaynak/test denemesi, NOT_RELEASE).

Canonical HF19, `CURRENT_ARTIFACT` ve önceki GF11 arşivi değiştirilmedi.
