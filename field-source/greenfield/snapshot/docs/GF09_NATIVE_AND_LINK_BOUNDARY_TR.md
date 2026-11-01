# GF09 — Orijinal kaynak test sınırı ve native teslimat

Durum: **NON_RELEASE_WORK / NOT_RELEASE**. Parent **GF08**, canonical kaynak **HF19**, `CURRENT_ARTIFACT` değişmez.

## Uygulama

- HF19'dan **10 yeni birebir dosya**: net `session_state` + `socket_transport` ve başlığı, 7 özgün HF19 birim testi. Kümülatif **216** HF19 dosyası.
- **5 özgün test** (`network_session`, `attribute`, `config_command_line`, `config_key_limit`, `asio_lifecycle`) gerçek kaynakla GCC/Clang portable koşullarında derlenir. Asio lifecycle testinin loopback TCP kullanması tam LSX/Windows IOCP kanıtı değildir.
- `test_handler_registry.cpp` tüm 42 handler fonksiyon işaretçisini gerektirir; default taşınabilir CTest'e alınmaz, yalnız `GF07_ENABLE_WINDOWS_DEEP_LINK_PROBE=ON` ve Windows/MSVC altında açık opt-in native full dependency probe. Sahte handler implementasyonu eklenmez.
- `test_get_config.cpp` özgün test, gerçek HF19 kaynaklarıyla izole GCC derlemesinde çalıştırıldığında **EXIT=14**, son `bad&bogus;` malformed-entity alanında `invalid_request` beklentisi uyuşmuyor. Bu gerçek açıklık sürerken default CTest PASS'a dahil edilmez. Açık araştırma için `GF09_ENABLE_GET_CONFIG_KNOWN_OPEN_PROBE=ON` etkinleştirilebilir; başarısızlık beklenir, `WILL_FAIL` ile maskelenmez. Test ve üretim kaynağı değiştirilmedi.
- `gf_preloader` hâlâ GF00 no-op `SURGE`, zero-owner `DllShutdown`, export `DllMain@1, SURGE@2, DllShutdown@3`; hiçbir GF09 statik kütüphanesi shipping yola bağlanmaz. LSX, Windows IOCP, Consumer/game kabulü yok.

## Windows kaynak kapısı

`scripts\RUN_WINDOWS_GF09.cmd` Visual Studio x64 Native Tools Prompt'ta Release `/MT` ve Debug `/MTd`, DLL `LoadLibraryW` + export/ordinal testini ve mevcut HF19 Win32 owner OBJECT derlemesini denemek için hazırlanmıştır; **Linux ortamında çalıştırılmadı**. Çıkış 0 yalnız gerçek çalıştırmada test geçerse döner. Tam dispatcher/real listener/games bu batch'in kapsamı dışındadır.

## Kanıt politikası

Kaynak ZIP SHA-256 ve GF08/HF19 parent SHA-256 sabittir. Gerçek kaynak manifesti, birebir HF19 kıyası, olumsuz testler, temiz çıkarım derlemesi ve kanıt ZIP CRC ayrı kayıtlarla izlenir. Başarısız test gizlenmez; canonical HF19 kaynak ve release pointer'ı değişmez.
