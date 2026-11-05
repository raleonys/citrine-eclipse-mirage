# notepad.exe — REA statik analiz kaydı

Hedef: C:\Windows\System32\notepad.exe  
SHA-256: 3f543719a819a9769d8f138a02544f93af8b53ef1779a96bc2403715d9a55380  
Boyut: 360448 bayt; sürüm bilgisi: 10.0.26100.8875; PE32+, x86-64.  
ImageBase: 0x140000000; giriş RVA 0x19c0, VA 0x1400019c0.

## Sunucu ve sağlayıcı

binary_session: rea-agents 6.5.0, sunucu rea, başlangıç 2026-10-16T18:27:50.098Z. 
Sunucu yolu C:\Users\Skysect\AppData\Roaming\npm\node_modules\rea-agents\scripts\rea.mjs.
Paket, yol ve combined catalog SHA-256 karşılaştırması aligned döndü.
Katalog özeti 405725e736ec8dc8483435b970acbf20f9f26b9f1fd25203b992f5ab4cd2a4f4.
Ghidra 12.1.4 (C:\tools\ghidra_12.1.4_PUBLIC), JDK 21.0.12.1.
Seçim açıkça ghidra; import_mode ephemeral-source-immutable.
Analiz profili ab1af6a0e002de5d4940f7e3d2cfd844cfe45f918fcd3678b343002640413374.

## PE bölümleri

VA aralıkları Ghidra bellek bloklarıdır; bitiş hariçtir. Bunlar ham PE VirtualSize ile aynı kavram değildir.

| Bölüm | VA başlangıç | Ghidra bitiş | İzin |
|---|---|---|---|
| .text | 0x140001000 | 0x140028000 | RX |
| fothk | 0x140028000 | 0x140029000 | RX |
| .rdata | 0x140029000 | 0x140034000 | R |
| .data | 0x140034000 | 0x140036740 | RW |
| .pdata | 0x140037000 | 0x140039000 | R |
| .didat | 0x140039000 | 0x14003a000 | RW |
| .rsrc | 0x14003a000 | 0x140059000 | R |
| .reloc | 0x140059000 | 0x14005a000 | R |

Headers ve tdb, PE bölüm tablosundaki sekiz bölümden ayrı Ghidra bloklarıdır.
Kanıt: ev_289f48e1ad34f2c1e94ba27dbd3b745ffad4b01519f3e31a523dc2d75053c6c5.

## Import/export

50 normal import tanımlayıcısında 315 giriş; 6 delay-import tanımlayıcısında 25 giriş.
Normal import dizini RVA 0x309c0, boyut 0x3fc; IAT RVA 0x298d8, boyut 0xb68.
Delay-import dizini RVA 0x303e0, boyut 0xe0.
GDI32:25, USER32:87, COMCTL32:11; diğerleri CRT ve Windows API-set bağımlılıkları.
Delay importlar: ADVAPI32:2, COMDLG32:9, PROPSYS:2, SHELL32:8, WINSPOOL.DRV:3, urlmon:1.
Tam ad/IAT tabloları notepad-rea-inventory.json içindeki imports alanındadır.
Export data-directory RVA=0, Size=0; PE export tablosu yoktur.
Kanıt: başlık ev_1e020848b4fc237fe7069382541b2868f8883baa93f801916fa043b6346b5fe2;
.rdata ev_fe64b5db3b6b2c320af37da24ea71d4e46c69afbfc31c53086d0e3a0811b7138.
Import sayıları bu REA byte gözlemlerinden ayrıştırılmıştır.

Örnek IAT VA'ları: CreateFileW 0x140029de8; ReadFile 0x140029df0; WriteFile 0x140029e10;
GetMessageW 0x140029af0; TranslateMessage 0x140029b08; DispatchMessageW 0x140029b10.
Bunlar hedef API'nin çalışma zamanı adresleri değildir.

## Fonksiyonlar ve önemli ilişkiler

853 Ghidra prosedür kaydı: 519 yerel (45 thunk dahil), 334 external.
Bunlar özgün kaynak fonksiyon sayısını veya kusursuz kod kapsamını kanıtlamaz.
Kanıt: ev_3282c41990e900bd77924d2bf0f5979efe3aca6fd348ebc799e67d866b3ce603.

- entry 0x1400019c0: 0x1400019c4 CALL 0x140001f98; 0x1400019cd JMP 0x140001840.
  İkinci kenarı Ghidra call reference olarak sınıflandırsa da assembly bir tail JMP gösterir.
  Kanıt: ev_388afc6263b8dc01e9691ffef1232dc0b618f73c16fe1c45d2b137353ca9e62d.
- FUN_140001f98: güvenlik cookie başlatıcısı olduğu çıkarımı.
  GetSystemTimeAsFileTime, GetCurrentThreadId, GetCurrentProcessId ve QueryPerformanceCounter
  sonuçlarını birleştirip 0x140034400 ve tümleyenini 0x140034440 adresine yazar.
  Kanıt: ev_41ee5ea4bf0944867f6676e8b430375e7ee7683d26b9427499eaeb0d8f7e2caf.
- FUN_140001840: CRT başlangıcı olduğu çıkarımı.
  0x140001896 -> _initterm_e thunk 0x140002868;
  0x1400018b7 -> _initterm thunk 0x14000285c;
  0x140001940 -> FUN_1400101d8.
  Komut satırı ve gösterim modu hazırlanıp uygulama ana işlevine veriliyor.
  Kanıt: ev_6ce954245c6825b66104aef1830d60ea653829407bf06b9241ab70371b7c957e.
- FUN_1400101d8: WinMain benzeri ana işlev olduğu çıkarımı.
  0x1400102d7 -> CoInitializeEx [IAT 0x140029ce8];
  0x14001034b -> FUN_140013154;
  0x14001047b -> GetMessageW [0x140029af0];
  0x140010453 -> TranslateMessage [0x140029b08];
  0x140010463 -> DispatchMessageW [0x140029b10].
  EventRegister/EventUnregister, pencere olay kancası, hızlandırıcı ve dialog işleme de gözlendi.
  Kanıt: ev_07c16b6972d7d67e77e4a7b3054156878ef76f409deef21c6792cb6f12ea104a.

## Sınırlar ve bütünlük

Statik analizdir; hiçbir hedef yürütülmedi. Fonksiyon envanteri tam alındı; ayrıntılı decompilation dört başlangıç/ana fonksiyonla sınırlıdır.
Çözülmemiş dolaylı akışlar nedeniyle CRT ve ana işlevin call kapsamı partial.
INT3/swi gibi decompiler öğeleri de çözülmemiş-call kayıtlarına girebilir; bunlar çalışma zamanı çağrı kanıtı değildir.
PDB referansı notepad.pdb, GUID af03920a-eeba-d640-d80a-655d70db27ee, age 1; Ghidra yükleme durumu unknown.
Özgün fonksiyon adları ve tiplerin tamamı kurtarılmış değildir.
Analizden sonra SHA-256 başlangıçla aynıdır; dosya değiştirilmedi. Ghidra oturumu kapatıldı.

notepad-evidence.json: REA'nın dışa aktarılmış kanıt paketi.
notepad-rea-inventory.json: envanter, tam import/delay-import tabloları ve seçilmiş fonksiyon dosyaları.
notepad-pe-tables.json: bağımsız, salt okunur PowerShell PE ayrıştırıcısının ek doğrulaması; REA Evidence değildir.

