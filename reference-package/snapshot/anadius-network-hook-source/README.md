# Connect callback'i ve hook temizliği

Bu klasör, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için kayıtlı REA/Ghidra statik kanıtından iki C++20 gövdesi içerir. Kaynak özgün ABI veya tam DLL değildir. Gerçek socket, hook, UI veya süreç çıkışı testlerde çalıştırılmaz.

| Özgün VA | Kaynak | Evidence |
|---|---|---|
| `0x18000fd00` | `connect_00fd00` | Assembly `ev_fc78699f1bf625a6a948750e048136e362866045ba59de7d4b783d2361c3bee8`; pseudo `ev_56829116b2c9c143c2c0fe7d3fa88a54fc6499474948b76b08e8cd2740d33c68` |
| `0x1800102b0` | `cleanup_hooks_0102b0` | Assembly `ev_2177b6a02a33ef123da2f83b0e6a1ee4e1e23cce8de54c8170baf93877a2102f`; pseudo `ev_00e35447d2de5559a4451e4788fcc72993ba5950a8158522aaa1efb615b7037c` |

Connect, uzunluk/family çiftleri uyuşmazsa tamponu incelemeden özgün callback sınırına gider. Kaydedilmiş `.data` başlangıç değerleri 16 ve 28'dir (`0x1800a10d0/4`, raw `0x9f6d0`, `10 00 00 00 1c 00 00 00`); `StateRefs` bunları ve port/sayacı çağırandan ister. IP adresi veya scope alanı denetlenmez/değiştirilmez. Uyuşan header'da network-order port okunur. 80/443 için bekleme yapılmaz, yalnız mevcut sunucu portuna eşitse yönlendirme yolu vardır. Diğer portlarda bütün başlangıç aşamaları beklenir; ardından eski istek portu 3216 veya o andaki sunucu portuna eşitse yalnız port alanı yeniden yazılır.

Yönlendirme günlüğü önce eski portu, sonra mevcut sunucu portunu yazar. Ardından 32 bit sayaç wrap ederek artırılır. Yeni değer tam15 ise hedefteki hata diyaloğu ve `exit(-1)` çağrılır; adres yazımı/özgün connect yapılmaz. Bu literal kaynak programın UI verisidir. Diğer durumlarda sunucu portu yeniden okunup sockaddr'ın iki port baytı değiştirilir; özgün connect dönüşü aktarılır. Ghidra'nın void prototipine karşı assembly'deki son indirect JMP esas alınır.

Cleanup hedefleri ileri sırada, sıfır/duplicate değerleri ayıklamadan gezer. Başarısız her disable için günlük vardır; işlem sürer. Uninitialize metni engine çağrısından **önce** yazılır, ardından sonuç ve flush gelir. Hedef listesi temizlenmez. API'nin `span` sınırları eski begin/end snapshot'ıdır: henüz ziyaret edilmemiş elemandaki yerinde değişiklik görülür, span dışındaki büyüme görülmez. Backing storage çağrılar boyunca geçerli kalmalıdır; reallocation desteklenmez.

`ExecutionEffects`, yalnız bekleme sınırını mevcut `Execution::wait_steps_036880` kaynağına bağlayan yeni adapter'dir; diğer etkiler çağıranın zorunlu uygulamasıdır. Testte gerçek kurtarılmış mutex/QPC/bayrak döngüsü, sentetik zaman ve hazır-olma değişimiyle birlikte çalışır.

```powershell
& '.\outputs\anadius-network-hook-source\build.ps1'
```

**324 kontrol PASS:** IPv4/IPv6, family/length uyumsuzlukları, HTTP(S) ayrımı, bekleme sırasında değişen port, flush sonrası yeniden okuma, sayaç wrap/15, tamponun diğer baytları, özgün callback dönüşü ve pointer aktarımı, exception prefix'leri, cleanup sırası/snapshot ve kaynaklar arası bekleme bağlantısı. Bağımsız statik kaynak incelemesinde düzeltme gerektiren bulgu yok.

Sınırlar: `span` okunabilir/yazılabilir backing memory'yi belirtir; gerçek Winsock length ayrı argümandır. İncelenen header için gereken 2/4 bayt yoksa `OutOfEvidenceInput` üretilir; bu özgün erişim ihlalinin eşdeğeri değildir. Geçersiz pointer, readonly veya eşzamanlı değiştirilmiş bellek ve C++ data-race alanı desteklenmez. Başlangıç global değerleri sıfırlanmaz. Log/UI/exit, gerçek connect trampoline'i, native callback ABI ve hook motoru zorunlu kalır. Thread-safety, CRT/EH/SEH/XFG ve çalışma zamanı eşdeğerliği kanıtlanmadı.
