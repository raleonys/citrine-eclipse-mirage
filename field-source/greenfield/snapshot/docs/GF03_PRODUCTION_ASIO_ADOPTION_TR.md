# GF03 — Production Asio owner kaynaklarının doğrudan alınması

Durum: **SOURCE_LINKED_REAL_LOOPBACK_TCP_TEST_ONLY / NOT_RELEASE**.

- GF02 immutable parent üzerinden tam kaynak snapshot.
- HF19 `AsioListenerOwner`, `AsioSessionOwner`, `AsioServiceOwner` ve `PreparedAsyncTransaction` **byte-identical** taşındı.
- HF19 unit testlerinde gerçek Asio TCP loopback accept, greeting, stop, quiescence vardır; protocol classifier/dispatcher bilinçli stub'tır.
- Asio upstream başlık seti + Boost Software License v1.0 eksiksiz taşındı.
- Asio OS socket owner için yeni bir wrapper/owner icat edilmedi.
- Export `SURGE`/`DllShutdown` bağlantısı **yok**; tam LSX, native IOCP, Consumer/game/WinMSVC kabulü **yok**.
- GF01 `ResourceOwners` ile OS socket lifetime'ını paralel sahipliğe almak YASAK; gerçek Asio sınıfları sahibi.
- Üretim koda geçiş için protocol dispatch ve `ReleaseServer` bridge; geçerli model-source test yeterli değildir.
