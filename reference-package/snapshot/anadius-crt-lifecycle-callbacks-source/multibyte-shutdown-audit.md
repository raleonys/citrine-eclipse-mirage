# 0644f8 global multibyte kapatma denetimi

Hedef SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Salt okunur retained REA assembly/pseudocode ve yerel PE verisi kullanıldı. Yeni REA oturumu açılmadı; özgün DLL çalıştırılmadı/değiştirilmedi. Üretim koduna dokunulmadı. Denetim çıktısı yalnız bu dosya ve aynı adlı JSON'dur.

## Kesin normal akış

Fonksiyon `0x1800644f8..0x180064537`, 64 byte; dosya ofseti `0x638f8`. Assembly EvidenceID `ev_0d023431ffc05119eee470b5ea148ad58ce9683f6c60cd3468df0b71bac082d7`; pseudo EvidenceID `ev_82ed5c1b2273a1b6458c896c47a7f737a42930c9a261f3be55551ef4e056625a`.

| Adres | Gözlenen işlem |
|---|---|
| 644fe | current-MB A59D0 QWORD'unu RCX'e yükle: ilk load |
| 64505..64508 | EAX=FFFFFFFF; `XADD.LOCK DWORD[RCX],EAX`: ref-1, EAX önceki ref |
| 6450c..6450f | Önceki ref !=1 ise64530'a çık |
| 64511 | current-MB globalini RCX'e **tekrar yükle**: ikinci load |
| 64518..64522 | RBX=initial MB A1C70; RCX==RBX ise64530'a çık |
| 64524 | free0668c0(RCX); **ikinci load'un sonucu free argümanı** |
| 64529 | free normal döndükten sonra current-MB=initial MB |
| 64530..64537 | AL=1 ve return; üst RAX bitleri tanımlı boolean sonucu değildir |

Bu fonksiyonda current globalinin **tam iki load noktası** vardır; ikinci load yalnız önceki ref1 dalındadır. Free argümanı için üçüncü global load yoktur. Atomic provider'ın etkisi globali değiştirirse ikinci load yeni pointer'ı görür. Free provider'ı current'ı değiştirirse64529'daki son yazma bu değişikliği initial ile ezer. `free` normal dönmezse64529'a ulaşıldığı iddia edilemez.

Soyut doğru akış: `old=load(current); previous=fetch_add(old,-1); if(previous==1){candidate=load(current); if(candidate!=initial){free(candidate); store(current,initial);}} return true;`. İlk `old` adresini karşılaştırma/free için cachelemek assembly ile aynı değildir. Buna karşılık free çağrısından hemen önce globali yeniden yüklemek de fazladan gözlenebilir etkidir.

## Üç ana dal ve ek sınırlar

| Başlangıç ref / ikinci load | Sonuç | Current reset |
|---|---|---|
| 0 | Aynı atomik sayaç0->FFFFFFFF, free yok | **Yok** |
| 1 / noninitial | Atomic hedef ref1->0; ikinci load candidate free; free sonrası current=initial | **Var** |
| 1 / initial | Atomic hedef ref1->0; static kimlik nedeniyle free yok | Store işlemi **yok** |

Ref2 için2->1; diğer ref değerlerinde de önceki1 olmadığı sürece ikinci load/free/reset yoktur. Tüm işlemler32-bit modulo aritmetiktir. Atomic hedef ile ikinci load'un candidate'ı callback etkileri yüzünden farklı olabilir; sayacı düşürülen record'un mutlaka free edilen record olduğu varsayılmaz. İkinci load null olursa null, initial'dan farklıdır: free(null) normal döner ve current=initial yazılır. İlk load null olursa atomic DWORD erişimi null hedefe gider; fonksiyonun null guard'ı yoktur. Native fault recovery bu normal-akış modelinin dışındadır.

## Hangi global değişir

Yalnız **ref1 + ikinci load noninitial + free normal dönüşü** dalı current-MB A59D0'ı initial'a yazar. Aynı callback initialized byte A59DC'yi, type/case pointer A59C0/A59C8'i, systemCP A59D8'i, locale changed A52D8'i, locale mask A1C6C'yi veya initial pair MB A1940'ı sıfırlamaz/değiştirmez. Tabloları kopyalamaz; replacement initial refcount'una retain eklemez. Lock5/lock4 acquire/release yoktur; tek senkronizasyon gözlenen DWORD XADD.LOCK'tur.

Mevcut free0668c0 gövdesi heap_free başarısızlığında error hücresini yazıp normal döner. Böyle bir normal başarısız free dönüşünde de callback current'ı initial'a yazar. Callback'in her zaman current'ı initial'a döndürdüğünü veya initialized'ı temizleyip yeniden başlatmaya hazırladığını söylemek doğru değildir. Normal dönüşte AL1, heap-free başarısını kanıtlamaz.

## Initializer tablosu

Çift tablo `[0x18008aa60,0x18008ab60)`, 16 adet16-byte pair. Index13 VA`0x18008ab30`, dosya`0x89330`: initQWORD0; uninitQWORD`0x1800644f8` (+8). Doğrudan orijinal PE okumasıyla doğrulandı; ham adresler rebuilt callback olarak çağrılmamalıdır.

Normal ters walker070588 null initializer'a bakmadan nonnull uninitializer'ı çağırır; bu nedenle index13 kapatmada çağrılır. Initialize rollback0704f4 ise önceki pair için hem init hem uninit nonnull ister; index13 **rollback'te atlanır**. Outer0645cc terminating=true yolu callback walker'ını atlar ve koşullu flush ile döner. Bu nedenle callback'in her shutdown çeşidinde çağrıldığı iddia edilmemelidir. Normal reverse index13, PTD pair index7'nin uninit064ecc'sinden önce gelir; bu statik tablo sırasıdır, çalışan süreç gözlemi değildir.

## Gerçek MB kontrolü ve native sayaç entegrasyonu

06f5f8 current-MB'yi initial'a bağlar;06f2d0/06ee00 başarılı publication ile fresh record'u PTD+global iki sahiplikte tutabilir. Callback0644f8 global ref release'idir; PTD release06489c ile aynı callback değildir ve o fonksiyonun lock5 veya ref/free helper'ı yerine kullanılamaz. PTD hayattaysa fresh ref2->1 dalının reset yapmaması olağandır; daha sonra PTD temizliği son ref'i düşürebilir. Pair/table pointer yaşamlarının tam zinciri ayrıca kanıtlanmalıdır.

`ImageState` ilk MB için gerçek hizalı LONG ömrünü başlatır; image ref0'dır. Yalnız kendi owned ilk MB üzerinde current=initial ile callback uygulandığında ref0->FFFFFFFF beklenir; bunu1'e düzeltmek için kanıt yoktur. Genel fresh record için typed LONG ömrü, native Counters.bind ve doğru heap/free/unbind gerekir. Counters.fetch_add eski bitleri döndürür; previous==1 testi son değere karşılaştırılmamalıdır. ImageState'in const image globals'u canlı mutable bridge Globals ile aynı nesne değildir; callback için ortak live slot ve adres alanı gerekir. Callback çağrılırken hiçbir bellek ömrü sona ermemeli; registry değişimleri quiescent ve senkronize olmalıdır.

Yeni rebuilt callback için önerilen owned fixture kontrolleri: ref0 wrap/current sabit; ref2->1/current sabit; ref1noninitial free+sonreset; ref1initial nofree/nopointerstore; atomic hook current'ı farklı candidate'a yönlendirince doğru free argümanı; free hook current'ı değiştirdiğinde sonreset; diğer bütün globals değişmez; nullcurrent sınırı native execution olmadan açık provider reddi. Bu denetimde test EXE veya özgün DLL çalıştırılmadı; yeni callback modülünü root uyguluyor.

## Açık kalanlar

Tüm DLL shutdown/termination sırası, SEH/abort/heap fault alanı, üst return-register bitleri, tam native deployment ve DLL/ABI eşitliği kapsam dışıdır. Yalnız statik pair sırası ve normal callback akışı kanıtlanmıştır. JSON dosyası retained listing ve ilgili kaynak hash'lerini kaydeder; yeni REA Evidence ID uydurulmaz.
