# N18 — Dört pool allocation normal kaynak gövdesi

25 Ekim 2026. `pool_allocate.hpp/cpp` dört normal gövdeyi ve yeni `PoolAllocationDispatch` / `RecoveredPoolAllocationEffects` generic final `pool_allocate` source adapterini ekler. Bu adapter önceki bir `NativeNodeClearEffects` allocation providerini kapatmaz: o interface'de allocation metodu yoktur. Önceki33 historical closure ve2 semantic session adapter korunur; yeni allocation adapter ayrı kaydedilir. Her hedefte bir tagged normal tanım vardır; generic target guard ek native gövde değildir.

## Kimlik ve kanıt

REA6.5.0 server/catalog kimliği aligned ve Ghidra kullanılabilir checkpoint'i `pool-allocation-rea-session.json` içindedir. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `pool-allocation-open.json` / `pool-allocation-close.json` geçici static import/read yaşam süresini kaydeder; hedef yürütülmez ve dosya byte'ları değiştirilmez. Ghidra12.1.4 ile `0x1800800d0` adresinden96/96 byte okuma `pool-allocation-simd-data.json` içinde tam saklanır. Evidence:`ev_6174d38308fd71da7aa56dfcf6ae30de443aee787ea60c50ea732bc04bff3b6f`. Analysis profile:`dcc676845881c04a2d22833fb35d26569462e033daee5e99a95edac5aa61ac05`. Bu initialized analysis-memory/loader-fixup kanıtıdır; live CPU/global çalışma gözlemi değildir.

Complete shipped pool vtable kanıtı `node-delete-next-vtables.json` byte olarak aynen kullanılır. Evidence:`ev_7349a6f3b8db519dad314c7de8d04be1e89ad0c44e1b592d75731e5c20e44706`. Dört tablonun slot+10 hedefleri aşağıdadır; slot+18 release0033a0 ve slot+20 track005a20 önceki N17 kaynaklarıdır.

| Pool vtable | Allocation | Stride | Block bytes | Öğe | Son link offset |
|---|---|---:|---:|---:|---:|
|18007fe08|180002b10|120|4080|34|f78|
|18007fe38|180002600|80|4080|51|fa0|
|18007fe68|180002960|112|4032|36|f50|
|18007fe98|1800027b0|104|4056|39|f70|

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0x180002b10|`ev_69676d992a02bd265c47a247cd170c8815221b7d90825447e0de7a237b156f39`|`ev_72c7774e42e84de667e4a26375d6ca97e95a9b796b6aebe4122567c82f87ddd6`|
|0x180002600|`ev_3bcc9c0bf4dcd2c6ec224b64614dac1a2459a19f35b72d0494668d93da66f038`|`ev_8e78b707d1a781815a1f68f2ca8f27a0af79b54f73e05ef12ae813734373a7c6`|
|0x180002960|`ev_dddd03633007c4bff924f221d212965ad8a9919050036732afd0b85c90e65b0d`|`ev_65d179a78369608cf6880a9dfc5d98aa344c97bda474d98972cf18e37663a7c3`|
|0x1800027b0|`ev_1ccec149cea5b273c4134f38d196487eb2915f2905923f86a19a7fd613ca8cc7`|`ev_5dab40953bd8da1662aaa4fb65baf20814ff6e665b9630d23f74417506849903`|

## Gözlenen normal işlem sırası

Her yordam pool+68 free root'u cache eder. Nonzero root allocation/fill/append/CPU-mode/link kurulumunu tamamen atlar. Fresh yol04b0bc(block bytes) sonucunu cache eder; nonnull ise0793d0(block,0,bytes) çağırır. Null sonuç fill'i atlar fakat005440(pool+8,block) append çağrısını ve sonraki işlemleri atlamaz. Yeni safe-null dönüşü, retry veya rollback eklenmez. Block pointer-array append link kurulumundan önce gelir; native005440 ve growth04b100/078d30/04b0f8 lower provider olarak açık kalır.

Signed DWORD CPU-mode1800a1550 değeri6'dan küçükse sırayla skaler next-link store'ları yapılır. Mode>=6 dalı observed SIMD lane aritmetiği/store sırasını bounded kaynak projeksiyonuyla kurar.1800800d0/e0/f0/100 çift64-bit stride değerleri80/104/112/120;180080110 DWORD lanes1,1,1,1;180080120 lanes0,1,2,3 şeklindedir. Kaynak seçilen16-byte stride ve ortak iki16-byte sabiti ilgili native dalda birer kez okur. SIMD prefixes stride80/104/112/120 için48/36/32/32 linktir; scalar tail ve son zero-link tamamlar. Kaynak SIMD register/exception/AVX instruction ABI'sinin birebir geri kazanımı değildir; byte sonuçları ve normal memory-store sırası sınırlı olarak modellenir.

Son öğe next-link0 yazılır; pool+68 block olur. Ortak pop tail item+0 next'i okuyup pool+68'e yazar; current DWORD+70 modulo32 artırılır. Yeni current signed olarak live max DWORD+78 ile karşılaştırılır, yalnız büyükse max yazılır. Sonra live total DWORD+74 ve live untracked DWORD+7c modulo32 artırılır; cache edilen item döner. Recycled item ilk QWORD linkini ve payload'ını korur; constructor/vptr/payload temizliği uydurulmaz.

80-byte öğe known XMLNode+60 QWORD alanını içeremez (en az104 byte gerekir). Bu gözlem tek başına C++ type adı vermez; XMLAttribute yorumu yalnız inference'dır. Bu aşama native DOM allocation/materialization veya her pool türünün XML root olduğu iddiasını yapmaz.

## Doğrulama ve sınırlar

515 çekirdek +8.720 entegrasyon =9.235 yeni kontrol PASS. Authoritative sonuçlar ortak build `socket_pool_allocate` / `socket_pool_allocate_integration` gruplarıdır; standalone modül scripti çalıştırılmış sayılmaz. Controlled integration allocator→track→release→reuse zincirini aynı mapped storage üzerinde dört gerçek stride/öğe sınırı ve multiblock koşullarıyla sınar. Tam message/parser/DOM entegrasyonu iddiası yoktur. Lower allocation/fill/append etkileri test fixture'larıdır; native provider implementation değildir. Ayrı bounded static source review source hash'lerine bağlıdır; full native equivalence review false kalır.

Ortak build199 üretim nesnesi(195 C++/4 C),211 normal +19 terminal grup,84 socket grubudur. Önceki631 kaynak/CRT byte olarak korunur;636 current source snapshot girdisi vardır.42 validation mirror çifti=41 historical+1current;82 historical config referansı korunur.109 dependency hash;33 historical typed source closure/2 semantic session adapter/1 ayrı allocation adapter vardır. Bunlar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Orijinal DLL/oyun/network/IOCP çalıştırılmaz; gerçek CFG/token/achievement içerikleri okunmaz. Hash-only input integrity ayrıdır; bilinen engelli version.dll ve generated version DLL tekrar denenmez. Genel CRT değişmez ve aşama5'e kadar bekler. Native allocation/fill/free/005440 append,pool lifecycle,node virtual/parser/DOM/native ABI/EH/concurrency açıktır.

Sıradaki N19:pool block-array append005440 normal gövdesi ve allocation-provider bileşimi. Bu hedefte yeni normal gövde henüz claimed değildir. Öncelik1→2→3→5→4 olarak korunur.
