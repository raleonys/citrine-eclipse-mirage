# Socket close/cancel: bağımsız üretim kaynağı incelemesi

Sonuç: **PASS — maddi bulgu yok.**

İncelenen dört kaynak: `outputs/anadius-socket-source/close.hpp`, `close.cpp`, `closing.hpp`, `closing.cpp`. Retained026e50/026fb0 pseudocode ve assembly normal akışlarıyla karşılaştırıldı; hash'ler eşlik eden JSON'da donduruldu. Üretim kodu değiştirilmedi; reviewer test, özgün DLL veya gerçek ağ çalıştırmadı.

`close_026e50` invalid socket için API/state/error erişimini atlar; destroying&&state&8 dalında state OR8 ve ignored linger option çağrısını korur. Option failure'da category→WSAerror sonuçları atılır. Her iki close başarısında yalnız error.value temizlenir; ham nonzero status ve category korunur. Failure capture ve iki ayrı kategori name-function identity karşılaştırmasının sırası doğrudur. Retry ioctl sonucu atılır, canlı state bit0/1 ancak bundan sonra temizlenir; aynı socket ikinci kez kapatılır. İkinci failure yeni error capture yapar, yeni retry eklemez.

`cancel_and_close_026fb0`, gerçek `std::atomic<Engine*>` üzerinde strong expected=null/desire=null CAS yapar; slotu NULL'a exchange etmez. Engine nonnull ise socket'i gate girişinden önce saklar; category getter gate içindedir ve cancellation995 bu cached socket/engine'i alır. Gate çıkışından sonra socket yeniden okunarak gerçek026e50 gövdesine destroying=true ile gider. Invalid ilk socket engine/CAS/cancel/close işlemlerini atlar, fakat reset/release yine uygulanır.

Socket invalid/state0/resource0 yazımları, geç control okuması ve slotNULL yazımı release'ten önce gelir. Strong old1 resource callback'ini çağırır; weak ancak bundan sonra azaltılır; weak old1 control delete eder. Callback pair atamasını kaynak sonradan tekrar silmez. Yeni bir allocation veya shared-owner factory çağrısı eklenmemiştir.

`ClosingLifecycleEffects`, mevcut LifecycleEffects close sınırını bu iki gerçek gövdeye bağlar. Service/context eşleşme guard'ı typed adapter domain'idir; özgün native exception ABI'si olarak sunulmaz. Resolver ve executor cleanup soyuttur;026aa0 cancellation map/completion posting/wake içi zorunlu effect olarak kalır. SDK socket/option/close/ioctl yolları mevcut AcceptorApi/NativeAcceptorApi üzerinden kullanılır; yeni gereksiz Win32 sarmalayıcı eklenmemiştir.

İçteki atılan Error'ın generic category başlangıcı veya native error_code padding'i typed modelde byte ABI olarak kurulmaz; bu alanlar caller'a gözlenebilir sonuç olarak sunulmaz. Opaque kategori belirteçleri gerçek vtable pointer overlay'i değildir. Native ASIO/IOCP, kategori singleton/CRT guard, allocator/global/lifetime, exception unwind ve genel concurrency/runtime eşitliği açık kalır. Normal-flow model, cancellation effect veya gate'in exception atması durumundaki native SEH cleanup'ını tamamlanmış saymaz.

Üçüncü bir özgün yordam veya026aa0 içi bu aşamada kaynaklaştırılmış sayılmaz. Test doğrulaması ayrı worker/root kapsamındadır.
