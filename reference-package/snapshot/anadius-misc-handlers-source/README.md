# Ek LSX yanıt işleyicileri

`misc_handlers.cpp`, **18 farklı yerel işleyicinin 24 kayıt adına hizmet eden gözlenen davranışını** C++20 olarak yeniden kurar. Bağımsız DLL değildir. Mevcut `anadius-handlers-source/handlers.hpp` içindeki XML/yazıcı ve zaman arayüzleri kullanılır; çocuk elemanlar, text yazımı, IGO olay kuyruğu/mutex ve protokol istisnası için zorunlu ek arayüzler bulunur. Gerçek ağ, arayüz, süreç veya CFG verileri kullanılmaz.

Kaynak: `anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. VA'lar bu DLL'nin tercih edilen `0x180000000` tabanına aittir. `evidence.json`, 16 işleyici ve 10 yardımcı için tutulan REA pseudocode/assembly Evidence kimliklerini, 44 literal kaydının ham ofset/baytlarını ve kayıt adı eşleştirmesini içerir. Yeni kaynak özgün C++ değildir.

| Kaynak işlevi | Özgün VA | Gözlenen sonuç |
|---|---|---|
| success | `0x18002ffb0` | GoOnline, SetPresenceVisibility, SetPresence, SetDownloaderUtilization, SelectStore: ErrorSuccess Code="0", Description="" |
| presence_visibility | `0x18002f4e0` | Visible="false" |
| presence | `0x18002f330` | Presence="INGAME"; RichPresence=game["DISPLAY_NAME"]; UserId="0"; diğer alanlar özgün sırada boş |
| query_presence | `0x180031ef0` | Her çocuk elemana Friend kaydı; UserId=çocuk text; Persona=username; Presence="UNKNOWN", State="NONE" |
| group_info | `0x18002f270` | ErrorSuccess Code="-1577058296", özgün party açıklaması |
| progressive_available | `0x180030cd0` | ItemId="", Available="false" |
| query_chunk_status | `0x180030eb0` | NoResponseRequiredError oluşturup fırlatır; XML veya bool yanıt üretmez |
| chunks_installed | `0x18002d720` | Installed="true"; her çocuk text'ini ayrı ChunkIds text düğümüne geçirir |
| query_friends | `0x1800314a0` | QueryFriendsResponse Return="Success" |
| block_list | `0x18002e5a0` | GetBlockListResponse Return="Success" |
| voip | `0x18002da30` | GetVoipStatus/EnableVoip: Code="-1593835518", özgün not-implemented açıklaması |
| wallet | `0x18002ff60` | Balance="0" |
| restart_game | `0x180032ab0` | Code="1", özgün shutdown bekleme açıklaması |
| permission | `0x18002d9e0` | Access="GRANTED" |
| utc_time | `0x18002fea0` | Response açıldıktan sonra current_time_041b60() çağrısı ve Time attribute'u |
| show_igo | `0x180032b10` | ShowIGO/ShowIGOWindow: başarı XML'i ardından mutex altında DOWN olayı ekleme |
| start_download | `0x180032ca0` | Code="0", özgün DLC download isteği açıklaması; download başlatan başka etki bu gövdede gözlenmedi |
| query_offers | `0x180031ec0` | Boş QueryOffersResponse |

`0x18002d720` ve `0x1800314a0`, tutulan Ghidra envanterinde ayrı fonksiyon değildi; kayıt tablosunda LAB pointer'ı olarak geçer. Bu iki gövde doğrudan özgün dosyanın **salt okunur LLVM disassembly** gözlemiyle `0x18002d720..0x18002d7dc` ve `0x1800314a0..0x1800314e3` aralıklarında tamamlandı. Ham disassembly `evidence.json` içindedir. Bu ek gözlem yeni REA Evidence olarak sunulmaz. QueryPresence inner close çağrısındaki `EDX=0`, pseudocode'un eksik argümanına karşı tutulan assembly'den kontrol edildi (`0x180032083`).

Boş string, "0" ve "false" birbirinin yerine kullanılmaz. GetPresence, DISPLAY_NAME map kaydı eksikse yalnız RichPresence yazımı noktasında boş kayıt ekler. QueryPresence/AreChunksInstalled, çocuk eleman adını filtrelemez; text'i attribute üzerinden okumaz. Text bulunmazsa `nullptr` aynen bağımlılığa gider: gerçek yazıcı bu durumda geçersiz erişime uğrayabilir; bu modül bunu boş string ile örtmez. Çağıran bir geçersiz girdi politikasına ihtiyaç duyuyorsa bunun özgün davranış eşdeğerliği olduğu varsayılmamalıdır.

ShowIGO önce başarı XML'ini kapatır, sonra mutex almaya çalışır. Lock başarısızlığı thread_error(5), sayaç `INT32_MAX` ise önce `INT32_MAX-1` yazımı sonra thread_error(6) olur. Başarılı yolda `02c400` kuyruk seçimi/ilk kurulumu, 68 byte DOWN olayının `034660` ile eklenmesi ve unlock vardır. `02c400` ilk kurulumunda zaten bir DOWN olayı önceden kuyruğa koyar; adapter bu ayrı etkiyi korumalıdır. Kaynak gerçek Win32 mutex veya global deque üzerinde işlem yapmaz. Bağımlılıkların exception üretmesi sırasında özgün SEH/EH cleanup/unlock yolları bu kapsamda kanıtlanmadı; normal yol ve açık hata dalları yeniden kuruldu.

`build.ps1`, taşınabilir mevcut LLVM-MinGW Clang 23.1.3 ile C++20 `-O2 -Wall -Wextra -Werror` derler. **288 sentetik kontrol geçti**: 12 sabit yanıtın tam literal/çağrı sırası; her XML çağrısında hata sonrası yazılmış prefix; map ekleme zamanı; çoklu/boş/eksik çocuk text'i; username NUL sonlandırması; saat çağrısının sırası; XML başarısızlığında IGO yan etkilerinin başlamaması; lock hata kodları, sayaç taşma dalı ve normal kuyruk ekleme. Yalnız yeni test programı çalıştırıldı. Özgün DLL/EXE yüklenmedi veya çalıştırılmadı; kullanıcı kimliği/CFG değerleri testlere kopyalanmadı. Bu kontroller gerçek host ile diferansiyel davranış eşdeğerliği kanıtı değildir.

`collect-evidence.cjs`, çalışma alanı kökünden `node outputs/anadius-misc-handlers-source/collect-evidence.cjs` ile kimlik/retained kayıt/literal denetimini tekrarlar. Gerçek hedef dosyasının yerel olarak mevcut olması gerekir. MSVC global/CRT/SSO/allocator/exception ABI, XML backend ve kuyruk adapter entegrasyonu ayrı katmanlardır. Başlangıç, diğer işleyiciler ve üç lisans/kimlik işlemi bu modülün kapsamına girmez.
