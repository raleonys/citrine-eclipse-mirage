# FIX21 — Configuration path dönüşündeki ek normalizasyon

NON_RELEASE tam kaynak devamı; mühürlü FIX20 ebeveynidir. Karşılaştırma kullanıcının tam RE paketidir. Özgün DLL binary/runtime eşdeğerliği iddia edilmez.

## Tespit ve kaynak kanıtı

Aktif `ConfigurationEffects::resolve_path_0192d0`, UTF-8 dönüşümü ve isteğe bağlı executable-parent birleştirmesinden sonra ayrıca `lexically_normal()` çağırıyordu. Bu çağrı `.`/`..` bileşenlerini ve bazı separator yazımlarını dönüştürür. Kullanıcının bu ek normalizasyonu istediğine veya bir upstream bağımlılığın bunu zorunlu kıldığına dair kayıt yoktur. İlk eklenme gerekçesi bağımsız kaydedilmemiştir; sonradan gerekçe veya onay uydurulmaz.

Paketin reconstruct edilmiş C++ modüllerinde `resolve_path_0192d0` yalnız virtual effect sınırı olarak bulunur. Bu, paketin tamamında kanıt olmadığı anlamına gelmez: `anadius-all-functions.pseudo.c` ve `anadius-all-functions.asm.txt` içindeki `0x1800192d0` gövdesi okunmuştur. Inventory de bu adres için `no_definition_mapping`, provider için boş `definition_candidates` kaydeder.

| Paket dalı | Görülen işlem |
|---|---|
| `019305 → 0145d0` | Explicit-length UTF-8 → wide path dönüşümü |
| `01930d → 01935b` | Relative bayrağı kapalıysa dönüştürülmüş path doğrudan taşınır |
| Absolute/root-name dalı | Parent eklemeden aynı path taşınır |
| `0193b6 → 019820`, `0193c2 → 00a140`, `0193d7 → 016b50` | Executable parent kopyalanır, dönüştürülmüş path eklenir |
| Join sonrası | Geçici nesne temizliği ve return; ek lexical normalization yok |

Pseudo evidence `ev_f97924b6178fd1c17ef606a5e5153562cafc5f04e36eecb08e5cdb93f2a31286`; assembly evidence `ev_aeef0b48642561e9aa088e2e457d40ad95a8159f366b6578e122c959b494e3a0`. Tam paket ve iki ham dosya SHA değerleri CHANGE_AUTHORITY_FIX21.json içinde bağlıdır. Bu okuma, raw fonksiyonun tüm allocator/exception/ABI davranışının yeniden uygulanması değildir.

## Dar düzeltme ve karşı örnek

Tek runtime satırı değişti: `return model_path(p.lexically_normal());` yerine `return model_path(p);`. Mevcut conversion, native-path projection, relative classification ve parent/join adapterları değişmez. CFG varsayılanı, arama konumu, optional davranış, profil, avatar, persistence, provider, hook, thread veya vendor politikası değiştirilmez. Yeni path kabul şartı veya kota yoktur.

`a/./b/../target.txt` gibi pathler resolver dönüşünde yazımlarını korur. Bunun gerçek dosya etkisi de vardır: POSIX fixtureda `base/link`, `outside/child` dizinine bağlanır. `base/link/../picked.txt`, `outside/picked.txt` dosyasını açar; önceden lexical olarak sadeleştirilirse `base/picked.txt` açılır. Test iki mevcut dosyaya farklı içerik koyarak bu ayrımı doğrular. POSIX sonucu Windows reparse/link davranışının doğrulandığı anlamına gelmez.

Yeni fixture doğrudan actual ConfigurationEffects nesnesini çalıştırır: dokuz input × iki relative modu, absolute × iki mod, UTF-8, explicit-length/NUL ve dört gerçek filesystem karşı örneği. Windowsa özel drive/UNC/device spelling vakaları Windows dalındadır; burada NOT_RUNdır. Eski implementation aynı değişmemiş fixturea bağlanınca reddedilmelidir; hedefli negatif bu farkı ayrıca kanıtlar. Beklenen değerler uygulamayı geçirecek biçimde değiştirilmez.

## Kapsamın sınırı

D10 tamamen kapanmaz: Configuration/License/Content/OnlineSession parent/join/filename, std::filesystem root/relative classification, UTF-8 projection ve sonraki IO/ownership sınırları ayrı inceleme ister. Mevcut Linux fixture projectionı Windows lexical algoritmasıyla topluca değiştirilmez. Yazımın resolverda korunması, downstream her APIde aynı spelling veya özgün Windows hata davranışı garantisi değildir. Mevcut RuntimePaths yazma sahipliği/atomic persistence politikaları korunur.

121 diğer src dosyası ve1.060 vendor dosyası FIX20 ile aynıdır. Önceki C++ test dosyaları ve Golden araçları değiştirilmez. Yeni integration fixture graph/CMake hedefine eklenir; source policyye bir negatif eklenir. D10 statüsü partial olarak güncellenir;15 review/partial kayıt açık kalır. Bu sayı15 doğrulanmış bug sayısı değildir.

## QA ve mevcut uyarılar

Final QA sonuçları evidence/FIX21 ile dış FINAL_QA dosyasına source digest ve exact ZIP SHA ile bağlanır. Portable Debug/Production/ASan-UBSan, CMake Release/CTest ve exact ZIP temiz replay ayrı raporlanır. Synthetic Golden/PE kontrolleri native DLL çalıştırılması sayılmaz.

İlk FIX20 CMake/CTest araştırmasında41/42 çalıştı; bir generated test executableında execute biti eksik olduğu için test başlayamadı. Bu koşu PASS olarak sunulmaz, ham kayıt saklanır. FIX21 yeni temiz build çıktıları kendi sonuçlarıyla değerlendirilir; eski derleme receiptleri miras alınmaz.

Linux GCC derlemesinde mevcut licensing/license.cpp:54 ve:106 için iki `noreturn function does return` uyarısı görüldü. Bu dallar sonunda virtual `no_response`/`exit` effectlerine gider; somut uygulama bunları exception ile karşılar. Uyarıları susturmak için bu scoped düzeltmeye abort/terminate/exit eklenmedi. Bunlar eski source/effect sınırının derleyici uyarılarıdır; Windows C4702 düzeldi veya warning-free build elde edildi iddiası yapılmaz. Ayrı warning/effect incelemesi gerekir; gizlenmez.

Windows/MSVC/DLL/IOCP/native provider/hook/LastError/Lease/CRT/game ve gerçek DLL Golden NOT_RUNdır. CMake testleri Linux core/test executablelarını derler; Windows DLL üretmez. LSan leak-free iddiası yoktur. Güncel master version9 revision2026-10-25.1 doğrulandı; AGENTS/metadata/QA kendi başına davranış yetkisi üretmez. Canonical pointer ve diğer FIELD projeleri değişmedi.
