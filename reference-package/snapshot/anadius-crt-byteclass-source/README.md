# CRT byte sınıflandırması ve argv parser bağlantısı

`byteclass.cpp`, anadius64.dll `0x18006f934` ve `0x18006f9a8` gövdelerini C++20 olarak yeniden kurar. Girdi SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `evidence.json` iki fonksiyonun saklanan REA Evidence kimliklerini; `assembly-evidence.txt` adresli assembly kesitini tutar.

`classify_06f934`, zorunlu `_LocaleUpdate` sağlayıcısından context alır. Karakterin yalnız düşük8 bitiyle multibyte tablosunun `+0x19+byte` alanını ve maskenin yalnız DIL/düşük8 bitini kullanır. Bu test eşleşmezse ve full32bit ctype maskesi sıfır değilse locale alanından tablo pointer'ını okur; `table+byte*2` WORD'u full32bit maskeyle test eder. Gereksiz ctype/pointer okumaları yapılmaz. Sonuç full EAX0/1'dir. Context flag'i sıfır değilse normal dönüş öncesinde PTD `+0x3a8` DWORD'unda yalnız bit1 temizlenir.

`is_lead_06f9a8`, localeNULL/ctype-mask0/multibyte-mask4 argümanlarıyla yukarıdaki gövdeye tail aktarımı temsil eder. Yeni `ParserEffects` adapter'i, parserin sign-extended byte parametresini bu gerçek kaynak gövdesine bağlar. Native locale kurucusu, tablolar, PTD sahipliği ve eşzamanlılık sağlayıcıya aittir; opaque token'lar özgün image VA'ları olarak çalıştırılmaz. Provider tüm adres okumalarının canlı/sınırlı depolamada kaldığını sağlamalıdır. Exception yoluna yeni RAII/SEH cleanup eklenmedi.

`./build.ps1` **7.173 kontrolü** geçti. 256 byte üzerinde maskeler, düşük/yüksek bit ayrımı, kısa devreli okuma sırası, bit temizleme ve normal-flow exception sınırı kapsanır. `review.json` bağımsız statik incelemedir. Testlerin locale/table/PTD sağlayıcıları sentetiktir; gerçek locale API veya hedef DLL çalıştırılmadı. Tam locale/SEH/ABI eşdeğerliği tamamlanmış değildir.
