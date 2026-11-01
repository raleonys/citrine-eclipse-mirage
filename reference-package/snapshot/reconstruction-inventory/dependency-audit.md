# Bağımlılık denetimi — 23 Ekim 2026

Öncelik sırası **1 → 2 → 3 → 5 → 4**. İlk envanterden sonra R2 `021d10`, R1 `010040/008350` ve R3 **`025c40`** normal gövdeleri kaynakta eklendi. Ayrıntılar, 49 seçili çağrı ilişkisi ve güncel 30 kanıt dosyasının SHA-256 değerleri [dependencies.json](dependencies.json) içindedir. [R3 kanıt ve test kapsamı](../anadius-socket-source/composed-read-flow-audit.md). Sırada R4 native frame köprüsü vardır.

## Mevcut kaynakla kapatılan eski sağlayıcılar

| Native adres | Kaynak gövdesi | Mevcut bağlantı |
|---|---|---|
| `0x1800261d0` | `composed_write_0261d0` | `RecoveredComposedWriteEffects::send_body_0261d0` |
| `0x180023dd0` | `construct_io_environment_023dd0` | `RecoveredConstructedWriteEffects::construct_environment_023dd0` |
| `0x180025a70` | `finish_write_025a70` | `RecoveredFinishedWriteEffects::finish_write_025a70` |
| `0x18002a500` | `start_queued_write_02a500` | `RecoveredQueuedWriteApi::next_write_02a500` |
| `0x180022c60` | `schedule_receive_handler_022c60` | `RecoveredHandlerDispatchEffects::schedule_receive_022c60` |
| `0x1800229d0` | `schedule_send_handler_0229d0` | `RecoveredHandlerDispatchEffects::schedule_send_0229d0` |
| `0x180025c40` | `composed_receive_025c40` | `RecoveredComposedReadEffects::receive_body_025c40` |

Bu bağlantılar kaynak düzeyindedir. Native sanal tablo, allocator, nesne ömrü, eşzamanlılık ve ABI eşdeğerliğini tek başına kanıtlamaz. Eski README kayıtlarındaki zorunlu sağlayıcılar, sonraki derived sınıflar kontrol edilmeden yeniden açık sayılmaz.

## 2. aşama: veri alma

Mevcut kaynak zinciri worker `0298b0` → completion dispatcher `027c10` → receive completion `027710` → environment move `023cb0` → schedule `022c60` → handler `022670/022640` → invoke `021a40` → **gerçek receive `025c40`** → zorunlu native frame `025480` biçimindedir. R3'ün yeni entegrasyon testi IO routing/completion/scheduling zincirini çalıştırır; dış worker döngüsü mevcut ayrı regresyonlarda izlenir.

| İş sırası | Paket | Açık iş ve bitiş ölçütü |
|---|---|---|
| 1 | R2: `0x180021d10` | Normal gövde ve SDK adapter'i kaynakta; allocation `0269b0`, constructor `023dd0`, publication `028b50/028c00` ve cleanup `0293f0` kullanılıyor. Önceki1.961 kontrol; artık gerçek R3 doğrudan çağırıyor. Native ABI açık. |
| 2 | R1: `0x180010040 / 0x180008350` | İki normal gövde kaynakta; önceki4.661 kontrol. Inline/heap, büyüme, hizalama/header/sized-free ve failure prefix'leri sınandı. Gerçek R3 çağırıcı bağlantısı eklendi; genel CRT byte gövdeleri/nativelik iddiası yok. |
| 3 | R3: `0x180025c40` | Normal read-until ve final receive provider köprüsü kaynakta; 5.386 yeni kontrol. Delimiter/sentinel/request/commit/EOF/error, R1/R2 ve borrowed/owned scheduling bağlı. R4 frame ve native ABI/exception/category sağlayıcıları açık. |
| 4 | R4: `0x180025480 / 0x1800280d0` | Mevcut `Session::frame_completed` ve `Session::do_read` modelleri native receive handler sahipliğine bağlanacak. Parçalı ve birden çok frame, embedded NUL, dispatch/erase/rearm, hata ve callback ömrü sınanacak. |

R1 ve R2 bağımsız hazırlanabilir; R3 ikisine, R4 bütün receive zincirine bağlıdır. Ek açık sınırlar `0285b0` misc category, `024390/023930` length_error, `026860` bad_weak_ptr ve genel amaçlı byte-fill sözleşmesidir.

Örnek statik çağrı kanıtları: `025c40` içindeki `0x18002612b` çağrısı `010040` adresine, `0x1800261a0` çağrısı `021d10` adresine gider. `0280d0` içindeki `0x1800281b7` çağrısı `025c40` adresine gider. Tüm adresler anadius64 görüntüsüne aittir; ayrıntılı kanıt kimlikleri JSON'da korunur.

## 3. aşama: native bağlantılar ve DLL entegrasyonu

Açık işler: handler destroy/move/query ve executor/factory sanal slotları; owned/borrowed scheduling; reference dispose/delete; dispatcher/service bağlantıları; posted/receive/send nesnelerinin gerçek depolama ve ömürleri; category/RTTI/logger sınırları; tek global durum grafiği; pending worker kapanışı; export ve DllMain bağlantıları; MSVC STL/EH/SEH/unwind sözleşmeleri.

Çıkışta özgün anadius makine koduna ihtiyaç duymayan linklenmiş hedef, statik PE/import/export/unwind denetimleri ve izinli sentetik startup/shutdown/rollback doğrulaması bulunmalı. CRT'ye bağlı kalan somut engeller 5. aşamaya aktarılır.

## 5. aşama: CRT'ye dönüş

CRT üretimi bu aşamaya kadar beklemededir. Başlıca engeller lower lowio `0732a8/067698/067788`, 16 ACRT çiftinin aynı native durum alanına bağlanması, initializer/flush sağlayıcıları, cookie ve `A5178` encoded-handler alias'ları, sağlıklı multibyte startup ve runtime/global/stdio/environment/TLS/SEH kapanışıdır. Mevcut kontrollü fixture başarıları bütün CRT grafiğinin eşdeğerliğini kanıtlamaz.

## 4. aşama: version.dll gömülü preloader

Dış katmanda 197 assembly wrapper ve 1 loader girişi bulunuyor; 198 export/ordinal için önceki statik build doğrulaması var. Gömülü görüntü 107 giriş içeriyor: 91 non-thunk ve 16 thunk. Mevcut version build hâlâ **21.504 bayt özgün gömülü makine koduna bağımlı**.

Son aşamada bağımsız kaynaklı embedded build kurulmalı; entry `3338` → dispatcher `3210` → application `1000`, Host/import/global/once/hook/ABI ve loader ömrü tamamlanmalı. Ardından özgün byte include bağımlılığı kaldırılıp dış export/resource sözleşmesi tekrar doğrulanmalı. Seçilmiş upstream hook motoru, özgün motorun kurtarılmış kaynak kodu sayılmaz.

## Kapsam ve doğrulama sınırı

`anadius64online.dll` iç uygulaması verilen iki DLL'nin fonksiyon paydasına dahil değildir; anadius içindeki loader/factory/version-check ve sanal çağrı sınırları kapsam içindedir. Görüntüler ayrı adres alanlarıyla izlenir.

Denetim saklanan kanıtları ve kaynakları okur; orijinal hedefleri çalıştırmaz veya değiştirmez. Gerçek CFG/token/achievement dosyaları okunmaz. Bağımsız önsayım ve symbol kontrolü tamamlanmış olsa da son üretilmiş envanterin ayrı worker tarafından tam incelemesi tamamlanmadı. Kaynak varlığından, test sayısından veya adres eşlemesinden tamamlanma yüzdesi hesaplanmaz.
