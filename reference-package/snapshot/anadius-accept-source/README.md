# B36 — Accept kaynakları ve sahipli tampon ömrü

`accept.cpp`, retained Ghidra C/ASM kanıtından `0x180021b00` ve `0x1800271a0`
normal akışlarını kurar. Her iki adres önceki canonical envanterde eksikti.
Kaynaklar instruction-order snapshot/live okumalarını, hata/category sırasını,
retry resetini, endpoint/context/socket devrini ve nested publication dallarını korur.
Kaynak/kanıt adresleri ve retained kayıt hash'leri `evidence.json` içindedir.

`OwnedAcceptMemory` gerçek parent heap'inden473 bayt ve raw+0x68'de288 bayt
adres bölgesini sahiplenir. SDK'ya verilen gerçek typed OVERLAPPED raw overlay
değildir. Tam raw bölge, submission/retry'dan observed completion'a kadar pinlidir;
cancel intent bu ömrü bitirmez. Her işlem kendi authoritative raw bölgesine göre
doğrulanır; yabancı ve başka canlı accept satırının tamponları reddedilir.

Base IO'nun checked inert Posted projection'ı yalnız host generation/reservation
defteridir. Gerçek accept operation ve SDK OVERLAPPED ayrıdır; receive/send alias'ı
yoktur. `AcceptDispatcherEffects` önce accept'i yönlendirir; diğer işlemler
mevcut owned IO router'a gider. B35 admission bütün native worker/dispatch zincirini
kapsamalıdır. Parent lease mevcut Operation altında edinilir; eski idle-only API
değişmemiştir. İki önceki dosyanın hash farkları açıkça kayıtlıdır.

Shared worker cache üzerinde accept satırı taşımış bir allocation, base-only
receive/send projection'a doğrudan geçirilmemelidir. Önce adapter üzerinden
`project_posted(storage)` ile eski accept sidecar'ı retirement'a sokup sonra
base projection kurulmalıdır; shared cache eviction/free yolları da adapter
retirement'ını korumalıdır. Bu aşama accept→posted reuse'u sınar; tam receive/send
context/cache koordinasyonu B37 entegrasyonunun açık gereksinimidir.

Native completion storage'ı handler'dan önce free/cache edebilir. Handler/error
yerel kopyadır; eski satır callback'te yeniden kullanılabilir. Retirement packet
bağını typed object bitmeden temizler. Scheduled task/lower etkileri active guard
taşır. Close, base mutex beklerken accept mutex'ini tutmaz ve mevcut callback'i
reddetmeden önce base preflight yapar. Bunlar external stop/drain kanıtının yerini
almaz; Runtime.quiescent sağlayıcıyı kapatıp parent lease'i B35 kontrolünden önce bırakır.

Native029660 retry,029590 holder reset,029df0 factory,0237f0 pending reset,
021a10/025720 handler ile environment/executor/schedule bağları mandatory lower'dır.
Seçilmiş host22520 task closure ayrıca recovered native body sayılmamıştır.
Native class layout, foreign ABI, native EH ve arbitrary concurrency kanıtlanmamıştır.
Native0241b0 context policy ve0228c0 açık kalır; generic CRT beklemededir.

Quick kontroller: 2045 core + 177 integration. Entegrasyon gerçek
own IOCP/heap/FLS/TLS ile actual023000 factory, posted disposal/cache reuse ve
observed995 packet drain çalıştırır. AcceptEx/GetAcceptExSockaddrs/setsockopt
enjekte edilir; gerçek ağ veya AcceptEx kernel submission çalıştırılmaz.
Tam üretim Resources/Transport/Scheduling ve loader bağlantıları açık; DLL BLOCKED.
Özgün/rebuilt DLL/oyun çalıştırılmadı veya değiştirilmedi. Yeni target byte sorgusu
yoktur; CFG/token/user achievement içerikleri okunmadı, AV engeli tekrar denenmez.

Sonraki paket: B37, actual025720 ilk oturum mesajı ve response akışını bu
sahiplik grafiğine bağlar. Genel DLL yüzdesi bu kaynak/etiket sayılarından türetilmez.
Tek ortak build log'u `work/b36-common-final-build.log`; ortak sonuç aşağıya eklenir.

## B36 ortak doğrulama

**2045 çekirdek + 177 entegrasyon = 2222 kontrol PASS.** Ortak build 254 nesne (250 C++ / 4 C), 301 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 862 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 870 kaynak. Yeni typed normal gövde 2, yeni native ABI/provider kapanışı0; 496 adres, 500 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
