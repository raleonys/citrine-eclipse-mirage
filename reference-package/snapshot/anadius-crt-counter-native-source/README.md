# Borrowed typed LONG sayaçları için native registry

Yeni `Counters`, mevcut locale/PTD kaynaklarındaki adresli atomik alanları **caller'a ait gerçek `LONG` nesnelerine** bağlar. `bind(LONG&)` alanın gerçek adresini kimlik olarak kaydeder; değeri değiştirmez, depolama tahsis etmez ve nesne ömrü başlatmaz. Arbitrary byte/global adresi `LONG*` yapılmaz. Yeni çıkarılmış özgün fonksiyon gövdesi **0**; bu bir native sağlayıcıdır.

`fetch_add` ve `exchange` önceki tam 32 bit değeri döndürür. `load`, `InterlockedCompareExchange(...,0,0)` kullanır. `bit_cast` signed `LONG` ile unsigned kaynak bitlerini ve modulo-2^32 taşmayı korur. Yalnız kayıtlı kimlikler native çağrıya ulaşır; bilinmeyen kimlik, duplicate bind veya olmayan unbind açık hata üretir.

## Yaşam ve eşzamanlılık sözleşmesi

Registry ve bütün kayıtlı `LONG` nesneleri çağrı boyunca yaşamalı, alanlar taşınmamalıdır. Bind/unbind, nesne kurulumu/yıkımı ve registry yıkımı **bütün registry kullanıcıları durmuşken** yapılır. Yalnız hedef alanın refcount'unun sıfır olması map mutation için yeterli değildir: diğer alanlardaki worker lookup'ları da tamamlanmalıdır. Map worker'lar süresince değişmez; aramalar const, değer güncellemeleri Interlocked'dır. Destructor kayıtlı nesneleri serbest bırakmaz ve canlı worker'ları durdurmaz. Kaydın süresi caller sorumluluğundadır; kayıt kaldırılmadan aynı adres başka nesne için kullanılmaz.

`bind` tür ve nesne ömrü sözleşmesinin yerine geçmez. Yanlış bir cast ile önceden oluşturulmuş geçersiz/misaligned `LONG&` güvenli hale getirilmez; testlerde böyle bir C++ referansı oluşturulmaz.

## Doğrulama

`build.ps1`: C++20/O2/Wall/Wextra/Werror ile **94 kontrol PASS**. Kendi fixture EXE'si dışında hedef çalıştırılmaz.

- Gerçek typed struct içindeki `LONG` alanları `+0x10` ve `+0x15c` ofsetlerinde kurulur; offset'ler `static_assert` ile doğrulanır. Bu iki test yapısı tüm özgün locale/time ABI'sinin yeniden oluşturulduğu anlamına gelmez.
- Tam-bit değerler, UINT32 taşması/decrement, eski değer dönüşü, LastError'ın değişmemesi, bağımsız alanlar ve bilinmeyen kimliklerin reddi kontrol edilir.
- Dört worker toplam 100.000 artırma yapar; final değer ve dönen önceki değerlerin toplamı denetlenir. Dört worker'ın 100.000 azaltması sonunda sıfır doğrulanır. Bütün worker'lar join edildikten sonra alan kaydı kaldırılır.
- Derlenen nesnenin disassembly'si `build/counters.disasm.txt` içindedir. Bu kendi adapter çıktısıdır; özgün binary ile komut eşdeğerliği iddiası değildir.

Kaynak dayanakları: locale `0x180070634/0708c0` ve time `0x180070838/070898`; PTD-data `0x1800647d4/06489c`. Hash bağlı önceki REA kanıtları `evidence.json` içindedir. Orijinal veya yeniden derlenen DLL/oyun çalıştırılmadı; girdiler değiştirilmedi. Global/locale kurucuları, registry'nin dinamik paralel mutation'ı, tam DLL/ABI/SEH eşdeğerliği açık kalır.
