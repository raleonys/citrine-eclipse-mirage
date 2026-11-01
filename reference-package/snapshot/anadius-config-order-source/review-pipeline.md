# Config2 pipeline bağımsız kaynak incelemesi

Sonuç: **PASS; engelleyici bulgu yok.** Üretim dosyaları değiştirilmedi ve testler yeniden çalıştırılmadı. Root tarafından bildirilen189 kontrol sonucu ayrı build kanıtıdır; bu inceleme source/retained statik akış eşleştirmesidir. İncelenen dosya hash'leri `review-pipeline.json` içindedir.

- Raw byte'lar mevcut03b3f0 reader'dan fresh parser State'e doğrudan gider; tek BOM'u parse kaldırır. İkinci strip/decode/NUL trim yoktur.
- Yalnız seçilen Config2 çevrilir. İlgisiz büyük root subtree'leri desteklenmeyen order conversion'a zorlanmaz. String/node dönüşü sahipli veri taşır; parser/input ömründen bağımsızdır.
- Read/grammar/order hataları yakalanıp boş başarıya çevrilmez; load_and_apply log ve uygulamasından önce yayılır. İlk stream bad-state sonucu özgün03b3f0 boş string davranışına bırakılır.
- ScalarDependencies, Content ve Entitlements çocuklarıyla Achievements altındaki AchievementNames/AchievementTotals/SteamIdOverrides çocuklarını log/apply öncesinde reddeder. Override için de reddetmesi, belgelenmiş muhafazakâr reconstruction sınırıdır.
- Generated fixture adı config-pipeline-generated-fixture.cfg'dir; NativeFactory yalnız test caller'ının sağladığı fixture dizininde kullanılır. Kullanıcı CFG/token dosyası girdisi yoktur.

İlk kopya sınırı statik olarak tekrar kontrol edildi:019c4c, local1c8'in container'ı local188 için014a90 çağırır; fresh8 bucket/load1 kurulur.019d52→0165c0→0162f0'da size<=8 ise017440 current8'i döndürür, buna rağmen017570 çalışır. Descendant constructor yolları010bc0→0132a0/014970→011ba0, source bucket8'i devralır ve source list'i yeniden ekler. Parser014b10/0164f0 move/swap ile container'ı korur;012820 duplicate key mevcut list entry'yi taşımaz. Böylece seçilen parser subtree'sinin her retained container'ında <=8 unique koşulu, tüm derinlikler için ilk kopyayı sınırlar.

019ad0 assembly Evidence `ev_b75b58538a829894dee04456d214f29001ee3a09dd2d8c04d5b393fa8534df88`, pseudo Evidence `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd`. Diğer yordamların çift kanıtları ve tam listing SHA256'leri `evidence.json` içindedir. Sonraki reused section copy kapasitesi/tüketim sırası, >8 growth, native node/allocator/exception ABI ve tüm DLL eşdeğerliği hâlâ açık kalır.
