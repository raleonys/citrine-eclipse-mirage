# Acceptor üretim kaynakları: bağımsız inceleme

Sonuç: **PASS**, belirtilen normal akış ve tanımlı typed-input/provider domain'inde maddi bulgu yok.

İncelenen dondurulmuş kaynaklar: `outputs/anadius-socket-source/acceptor.hpp`, `acceptor.cpp`, `acceptor_native.hpp`, `acceptor_native.cpp`. Tam SHA-256 değerleri eşlik eden JSON'dadır. Üretim kaynakları düzenlenmedi. İncelemeci build/test, özgün DLL/oyun veya gerçek ağ/socket/WSAStartup yürütmedi. Kullanıcı cfg/token/XML/INI içeriği okunmadı.

Karşılaştırma dayanağı `work/acceptor-flow-audit.md/json` içindeki retained Windows Ghidra pseudocode/assembly ve adresli kanıtlardır. `open/bind/set_option/listen` context metinleri için baseline DLL hash + bağımsız read-only PE section mapping ile `work/acceptor-context-strings.json` doğrulandı; bu ayrı byte kanıtı REA string Evidence diye sunulmaz.

## Doğrulanan çekirdek noktalar

- `acceptor_020e20`, mandatory0211b0 initializer'dan sonra endpoint family okur; family2 dışındaki bütün değerleri socketAF23'e eşler. Cached family yalnız socket creation/ignored dualstack option için kullanılır; bind uzunluğu için family yeniden okunur.
- Native service identity020e70'de socket creation öncesi cache edilir, completion-port ise020f4c/56'da daha sonra okunur. Kaynak `AcceptorService*` kimliğini cache edip `service->completion_port` değerini attach çağrısında okur; böylece create/ignored-option sırasında port değeri değişse canlı değer kullanılır. Bu typed projection native service/scheduler memory overlay'i değildir.
- WSA failure'da category→WSAGetLastError sırası; IOCP null'da GetLastError→category sırası ayrıdır. IOCP nonzero error durumunda geçici socket close helper'dan sonra ilk captured error korunarak `open` yükseltilir. IOCP null + error0 transfer yoluna devam eder.
- WSASocket invalid + error0 farklıdır: transfer/state0x10/shared reset yoktur; protocol alanları yine resetlenir ve sonraki invalid option/bind guard10009 üretir. Reset alanları family+40, byte+44=0, sabit qword+48=2, qword+50/+58 ve dword+60=0 ile uyumludur.
- Transfer socket/state0x10 yazımından sonra mandatory0237f0 cancellation/shared-resource reset'i çağrılır. Provider'ın allocation/refcount/exception ömrü yeni kurtarılmış sayılmaz. Sonraki reuse/bind/listen raise yollarına native normal gövdede bulunmayan otomatik close eklenmemiştir.
- IPv6 dualstack setsockopt(41,27,int0,size4) sonucu yok sayılır. Reuse setsockopt(65535,4,int1,size4), bind16/28 ve listenINT_MAX için yalnız API status nonzero ise category→WSAGetLastError örneklenir; captured error0 akışı durdurmaz. Her sonraki işlem için state.socket canlı okunur.
- `close_025310` input SOCKET'i bir kez cache eder, caller slotunu invalid yapmaz. Invalid veya ilk close başarı dalında ek hata API'si yoktur. İlk failure'da kategori→hata→kategori karşılaştırma; gerekirse ikinci karşılaştırma getter'ı vardır. Opak+8 kimliği ile10035/1237 eşleşmesi, ioctl0x8004667e/zero ve en çok ikinci close korunur. Ioctl sonucu yok sayılır; ikinci close failure kategori→hata örneklemesi yapılır ve atılır.

## SDK ve kısmi backend

`AcceptorImports`, gerçek SDK fonksiyon imzalarını kullanır. `NativeAcceptorApi` WSASocketW(type1,protocol6,protocolnull,group0,flags1), CreateIoCompletionPort(key0,concurrency0), setsockopt(int32,size4), bind, listen, close ve ioctl argümanlarını korur. Socket/HANDLE64 ve int/long/u_long32 varsayımları compile-time guard'larla işaretlidir. IOCTL command bitleri `bit_cast<long>` ile taşınır. API çağrısı ile sonraki core LastError örneklemesi arasına ek heap allocation, owned temporary destructor veya API eklenmez.

`AcceptorBackend` mevcut AddressBackend/ServerBackend arayüzünde yalnız constructor metodunu sağlar. Endpoint portu byte swap ile network sırasına aktarılır; IPv4 dört, IPv6 onaltı adres baytı ve scope korunur. Fixed INT_MAX dışı backlog açık UnsupportedInput domain'idir. Context/accept/run/destructor metodları soyut kalır; constructor oluşturmak API çalıştırmaz.

Short/başlatılmamış endpoint veya null service, kaynakta açık UnsupportedInput alanıdır; bu koruma özgün DLL'nin WSA/exception ABI'si olarak iddia edilmez. Native ASIO context/intrusive list/service discovery, shared control-block allocator/lifetime, lazy kategori singleton/CRT guard, actual IOCP/socket sahipliği/concurrency, MSVC object/error padding/stack/SEH/unwind ve tüm DLL/runtime eşitliği kanıtlanmış değildir. Yeni özgün normal yordam sayısı **2**; dört SDK/bridge dosyası tam native backend anlamına gelmez.
