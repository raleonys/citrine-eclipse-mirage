# Kurtarılmış CRT kaynaklarının Windows heap bağlantısı

`Effects`, mevcut malloc/calloc/realloc/msize/recalloc kaynaklarını tipli Windows SDK heap çağrılarına bağlayan yeni C++ adapter'dır. Yeni çıkarılmış özgün gövde sayısı **0**. Hata eşlemesi ve errno hücre seçimi mevcut kurtarılmış kaynaklardan gelir. Lock, new-handler ve invalid-parameter davranışları zorunlu `Policy` sağlayıcısıdır; özgün image adresi callback olarak yürütülmez.

`crt_heap_native::Calls`, error state, PTD sağlayıcısı ve policy nesneleri çağrı boyunca yaşamalıdır. Heap kimliği kaynak `initialize_heap_0704cc` tarafından kurulabilir; adapter process heap'i yok etmez ve tahsis sahipliğini üstlenmez. Ham native sonuçlarda kaynak pointer aritmetiğinden önce, mevcut baytları üzerine yazmayan placement-new ile `std::byte[]` yaşamı kurulur. Bu işlem typed PTD/Context kurucusu yerine geçmez. Pozitif extent `PTRDIFF_MAX` değerini aşarsa native tahsisten **önce** açık destek sınırı hatası üretilir; bu özgün binary'de bulunduğu iddia edilen bir kontrol değildir.

## Doğrulama

`build.ps1`: C++20/O2/Wall/Wextra/Werror; **2.193 kontrol PASS**. `review-native-bridge.json` bağımsız inceleme ve kaynak hash'lerini içerir. Ortak derleme `common-validation.json` içinde ayrıca kaydedilir.

- Gerçek process heap üzerinde bize ait küçük bloklarda sıfır uzunluğun 1 bayta çevrilmesi, calloc/recalloc sıfırlaması, büyüme, küçülme, msize ve free sınandı. Beklenen büyüme sıfırlaması, `HeapSize` API'sinin bildirdiği eski boyuttan hesaplanır; allocator yuvarlaması varsayılmaz.
- SDK gözlem sarmalayıcıları gerçek heap API'lerini çağırır ve blokları test bellek alanına kaydeder. Tahsis/realloc/free/size başarısızlıkları yalnız bu test katmanında enjekte edilir. Eski veri, overflow öncesi çağrı sayıları, PTD errno22/13 ve değişmeyen DOS alanı doğrulandı.
- Kaynak new-handler, lock0 bırakıldıktan sonra çağrılır; 0x100 sıfır dışı dönüşü yeniden denemeye yol açar. İlk denemede testin reddettiği sentetik heap kimliği, callback sonrası geçerli process heap kimliğine değiştirilir; sentetik kimlik gerçek Windows API'sine verilmez.
- NLS/type/map zinciri gerçek heap ve NLS API'leriyle 504/505 karakterde sınandı: küçük tampon stack sağlayıcısında kalır; büyük type isteği bir, map isteği iki gerçek heap tahsisi yapar. Sonunda bütün gözlenen bloklar, scratch frame'leri ve PTD bit2 sahipliği bırakılır. Büyük tahsis reddinde errno12 ve tanımsız çıktı korunur.

Bu grubun `main` fonksiyonu yukarıdaki üç test akışını çağırır. Önceki fixture'lardan bağlanan tüm argv/wildcard/record/lifecycle algoritmalarının burada çalıştığı iddia edilmez. PTD/FLS edinme, lock/atomic, scratch, global başlangıçları ve genel loader yaşam süresi kontrollüdür. Gerçek Windows çağrıları yalnız bize ait bellek/sentetik stringler ve önceki sabit sistem modülü resolver fixture'ı üzerindedir. Orijinal veya yeniden derlenmiş DLL/oyun çalıştırılmaz. Tam DLL, MSVC ABI, SEH/unwind ve orijinalle runtime eşdeğerliği açık kalır.

## Kanıt ve API sözleşmeleri

Hash bağlı statik dayanaklar `evidence.json` içindedir: `0x1800704cc`, `0x180066848/0668c0`, `0x18006a308/06e3a0`, `0x180070434/0773bc`, `0x1800634b0/0634f0`, `0x18005d650/05d724`; NLS devamı önceki adresli modülleri kullanır. Bunlar statik adreslerdir, çalıştırılan fonksiyon pointer'ları değildir.

[HeapReAlloc](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heaprealloc) başarısızlığında eski blok geçerli kalır; bu API ve [HeapSize](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsize) başarısızlığında yeni bir LastError değeri beklenmez. Adapter tahsis hatalarına hayalî OS hata eşlemesi eklemez. Kaynak free başarısızlığındaki GetLastError→errno yolu ayrı tutulur. Yanlış heap, serbest bırakılmış pointer veya aşırı büyük gerçek tahsis denenmez.
