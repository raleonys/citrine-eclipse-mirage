# B14 — Host/hook OS/log politikaları ve ortak engine sahipliği

`Domain`, açık stream/streambuf, engine, resolver, callback-lifetime ve encoding/SDK bağımlılıklarını ortak sahiplikte tutar. `HostPlatform` **11host** yolunu, `HookCleanup` **4cleanup** yolunu sağlar. Bunların3host+2cleanup engine yolu mevcut `HostInitEffects`/`CleanupEffects` adapter'larıdır; yeniden yazılmadı. Constructor/destructor engine/SDK/log veya implicit shutdown çalıştırmaz.

**64 çekirdek +184 entegrasyon =248 yeni kontrol PASS.** Bağımsız reviewer `/root/b14_host_audit`: **PASS**. İlk kullanım limiti yenilendikten sonra tam inceleme tamamlandı. Reviewer kaynaklara yazmadı; root test çalıştırmasını5source/2EXE hashleri ve770eski kaynakla karşılaştırdı, testleri yeniden çalıştırmadı. Önceki770 kaynak aynı; toplam775 kaynak.

## Adres, ABI ve korunmuş davranış

Retained00e310 version hazırlığı00ed53..00edd1: size284,major6,minor2,SPmajor0; koşul maskeleri type2→1→32,condition3,type-mask0x23. Adapter gerçek `OSVERSIONINFOEXW{}` kullanır; beşDWORD,128WCHAR,üçWORD,ikiBYTE alanı tek tek kopyalar. Boyut ve bütün SDK offset'leri static_assert edilir; char16_t/wchar_t alias overlay yoktur. BOOL `!=FALSE` olarak ele alınır; test0x100 başarısını da kapsar.

00af90 integer pseudo Evidence `ev_88663c95104b1392202afc39b7e30244601bdb1a02f505cd0c4b9c6a1e994501`, asm `ev_ec9df5b4c145fcce4d55abed75e57a22b580c3a69b88c7d54ee9e26335b9498a`. EDX/R12D32bit değer00b09a'da facet argümanı olur;00b0b0 locale facet+48 çağrısıdır. Bu yüzden compiler `ostream<<int32_t` seçilir; `to_string` ile zorla decimal yazılmaz. Native facet/locale/EH eşitliği iddia edilmez.

19log/export metni ve5wide module adı retained rdata ile karşılaştırıldı. Domain yalnız gözlenen adları statik NUL-terminated storage'a eşler; borrowed view taranmaz. Empty module currentEXE'ye çevrilmez. `GetModuleHandleW` kullanılır; LoadLibrary fallback yoktur. [Tam adres/Evidence/hash kaydı](source-evidence.json).

00f120/046970 create→enable→append sırası korunur. Engine original output'u **her normal create dönüşünde**, başarısız status'ta da kopyalanır. Throwing injected engine outcome'da copyback uydurulmaz. Nonzero initialize (1already-initialized dahil) kurulumu durdurur.0102b0 cleanup forward order'da disable eder, hata loglar, uninitprefix'i yazıp engine sonucunu loglar. Throw durumunda önceki prefix'ler geri alınmaz; hedef vector'ü temizlenmez.

## Sahiplik ve açık native Context eksikliği

İki wrapper aynı sharedDomain ve engine'i kullanır. Existing adapter'lar this/domain'e reference tutar; wrapper copy/move kapalıdır. Domain resolver/engine yıkımı boyunca stream/resources/callback-lifetime'ı tutar. Streambuf ayrıca yaşamalı; bütün kullanıcılar caller tarafından serileştirilmelidir. Aktivasyondan önce caller native callback binding'ini yayınlamalı, aynı original slotlarını kullanmalı; kapanışta hook disable ve dış callback quiescence sonrası Binding.close başarılı olmalıdır.

**callback_lifetime token'ı bunları kanıtlamaz.** SourceBackend RuntimeSeed'i private Impl::state'e taşır; host setup oradaki original/map/targets'a yazar. Native OriginalEffects/Context'in dışarıdaki pre-move seed'e veya kopyasına referans vermesi yanlış kimliktir. B4 Runtime şu anda exact inner state'i bağlayan factory sağlamaz. B14 native Context yayınlamaz; fake resolver adresleri invoke edilmez. [Somut eksik](native-context-gap.json), [güncel Resources denetimi](resources-audit.json).

## Test sınırı ve kalan iş

Testler OSVERSIONINFOEXW tüm alanları/fullBOOL, modern/eski branches,6–10hook, missingmodule/proc11/12, create/enable hataları/original yazıları, allocation-independent map prefix'i, stream/engine exception prefix'leri ve cleanup sırasını doğrular. Actual host/cleanup ve B3/B4 Runtime kaynakları kontrollü SDK/engine/resolver/join/callback sağlayıcılarıyla çalışır; timeout/retry aynı owner'ları korur. Yalnız kendi stringleri üzerinde SDKencoding ve kendi sayılarıyla condition-mask matematiği çalışır. Gerçek OS version/module/export lookup, hook engine, hook installation, native Context veya orijinal/rebuilt DLL çalıştırılmaz.

Configuration19/21, Services17/17, online15platform/7foreign sınırı korunur. Yeni canonical native gövde/kapanış0;487mapped/491eligible/124normalclaim/52tarihsel kapanış aynı. Toplam DLL yüzdesi türetilmez; tam DLL **BLOCKED**. B10 review borcu, native CRT/ABI/concurrency/unload/PE ve configuration/license boşlukları açık. Orijinaller değiştirilmedi; gerçek kullanıcı CFG/token/profile/achievement içeriği veya AV engelli version girdileri okunmadı/yeniden denenmedi.

B15: taşınmış Runtime iç state'iyle native Context/original-slot kimliğini kesinleştiren bağlama sınırı. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B14 ortak doğrulama

**64 çekirdek + 184 entegrasyon = 248 kontrol PASS.** Ortak build 232 nesne (228 C++ / 4 C), 259 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 770 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 775 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
