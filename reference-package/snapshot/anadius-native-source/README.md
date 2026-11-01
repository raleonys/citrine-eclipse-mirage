# Host callback ABI köprüsü ve tipli trampoline çağrıları

`native_callbacks.cpp` mevcut on uygulama gövdesine Windows SDK imzalı serbest girişler sağlar. `original_effects.cpp`, host başlangıcının doldurduğu aynı on original slotunu SDK tipindeki function pointer'lara çevirir. Bu **yeni kaynak entegrasyonudur**; özgün MSVC prolog/unwind/SEH/XFG kodunun geri kazanıldığı iddia edilmez. Orijinal DLL ve oyun çalıştırılmaz/değiştirilmez.

| Kaynak giriş | SDK imzası | Özgün gövde VA | Slot |
|---|---|---|---|
| `connect_entry` | `connect` / WSAAPI | `0x18000fd00` | `0x1800a3838` |
| `reg_open_a_entry` / `reg_open_w_entry` | RegOpenKeyExA/W | `0x18000f800` / `0x18000f960` | `0x1800a3840` / `0x1800a3848` |
| `reg_close_entry` | RegCloseKey | `0x18000f5b0` | `0x1800a3850` |
| `reg_query_a_entry` / `reg_query_w_entry` | RegQueryValueExA/W | `0x18000fac0` / `0x18000fb60` | `0x1800a3858` / `0x1800a3860` |
| `process_a_entry` / `process_w_entry` | CreateProcessA/W | `0x18000f3f0` / `0x18000f4d0` | `0x1800a3868` / `0x1800a3870` |
| `folder_a_entry` / `folder_w_entry` | SHGetFolderPathA/W | `0x18000fc00` / `0x18000fc80` | `0x1800a3878` / `0x1800a3880` |

Bütün girişlerin SDK function pointer tipine dönüştürülebilirliği compile-time doğrulanır; `noexcept` farkı C++ tip sözleşmesidir. Kaynak SDK include'ları Winsock2'den başlar. LSTATUS/HRESULT signed bitleri bit_cast ile korunur. `Resolver` tercih edilen binary VA'larını kullanmaz; bu dosyadaki gerçekten derlenen girişlerin adreslerini döndürür. Eski fonksiyon gövdeleri ve Evidence kimlikleri ilgili registry/host-callbacks/network-hook klasörleri ile `evidence.json` içindedir.

## Bellek ve sahiplik

Registry çıktı pointer'ları `HandleCell::from_raw(PHKEY)` / `DwordCell::from_raw(PDWORD)` ile taşınır. Cell yükleme/yazmaları memcpy ile **anında** olur; geçici integer sonuç mirror'u yoktur. Böylece native HKEY/DWORD nesnesinin adresi, original çağrının değişiklikleri, kapasite snapshot'ı ve hata yolunun kısmi çıktıları korunur. Aynı boyutlu farklı C++ türleri arasında yanlış pointer alias yapılmaz.

Wide callback pointer'ları opak UTF-16 storage olarak taşınır; original trampoline'e aynı wchar_t adresi geri gider. Mevcut değer yardımcıları bu borrowed input'u kendileri char16_t nesnesi gibi dereference etmez. `OriginalEffects`, wide compare/copy'yi zorunlu `WideOperations` sınırına yönlendirir. Bunun implementasyonu iki backing türünü bayt bazında okumalı/yazmalı veya ayrıca doğrulanmış CRT adapter'i kullanmalıdır; doğrudan char16_t dereference geçerli değildir. Test policy'si memcpy kullanır; özgün CRT locale/invalid-parameter politikası olarak sunulmaz.

Connect, tanınan length için ilk4 baytın okunabilirliğini ve yönlendirmede port baytlarının yazılabilirliğini caller sözleşmesi sayar. `const sockaddr*` özgün gövde gibi port yazımı için geri çevrilir. Length uyuşmazlığında sıfır uzunluklu span aynı raw pointer'ı taşır ve original'a aynen geçer. Geçersiz veya yetersiz backing bellek bu köprünün eşdeğer alanında değildir.

## Bağlama sırası

1. Caller-owned config/Values/handle setleri/network flags ve **aynı original slot dizisini** oluştur.
2. Bu dizi ve WideOperations ile OriginalEffects'ten türetilen gerçek app/CRT/log/wait policy'sini kur. Başarı döndüren varsayılan backend yoktur.
3. `Context` ve süreç ömürlü `Errors` ile `Binding` kur; ardından `Resolver`'ı `recovered_hooks::HostInitEffects`'e ver ve hook başlangıcını çalıştır. Original slot create dönüşünde yazılır; enable sırasında callback gelebilir.
4. Shutdown'da hook'ları kapat, dış callback çağrılarının tamamen durduğunu sağla, sonra `Binding::close()` çağır. Aktif çağrı varsa close beklemez; Busy bildirip binding'i tutar. Bu sınıf hook kaldırmaz veya hedef thread'leri join etmez.

Tek görüntüde tek process-global binding vardır; thread_local kullanılmaz. Gate, context pointer'ı ile in-flight sayacını birlikte korur. Callback kendi binding'inin Errors pointer'ını saklar; lease bırakıldıktan sonra yeni binding kurulsa bile eski exception yeni politikaya yönlenmez. Context/effects/Values/slotlar close tamamlanana kadar yaşamalı ve uygun şekilde senkronize edilmelidir. Errors, callback giriş girişimleri dahil süreç ömürlü olmalıdır. Aynı Binding'in close/yıkım işlemlerini tek kontrol thread'i yönetmelidir. Gerçek caller state'inin eşzamanlı mutasyonu için otomatik kilit eklenmez.

Native SDK girişleri C++ exception'larını başarıya çevirmez: zorunlu, geri dönmeyen `Errors::callback_exception` politikasına gönderir. Hiç binding kurulmadan giriş std::terminate alanıdır. Bu yeni sınır politikası özgün MSVC EH/SEH kurtarımı değildir. CPU access violation/SEH yakalama iddiası yoktur. Binding destructor idle kapanışı gerektirir; aktif çağrı varken yok etmek geçerli lifecycle değildir. Gate giriş/çıkışı GetLastError/SetLastError ile yalnız thread'in mevcut hata metadata'sını korur; app/original callback'in ürettiği LastError sonucu aktarılır.

## Doğrulama

`build.ps1`: **91 normal kontrol ve 3 terminal policy alt süreci**. Testler doğrudan SDK tipli pointer'larla bu derlenmiş shim'leri çağırır; original slotlar yalnız test programının C++ sahte fonksiyon adresleridir. On resolver girişi, native DWORD/HKEY/wchar çıktıları, bütün query/process argümanları, dönüş bitleri, caller pointer kimliği, connect portu, LastError ve worker-thread binding/busy-close doğrulanır. Gerçek host-init→motor adapter→native resolver→SDK shim→kaynak gövde→tipli sahte original akışı on giriş için çalışır; enable dönmeden callback gelmesi modellenir. Hook kurulmaz ve üretilen trampoline komutları çalıştırılmaz.

Terminal testleri yalnız kendi sentetik EXE'sini `--closed`, `--missing`, `--throw-original` kiplerinde çalıştırır; zorunlu politika exit86 ile doğrulanır. Gerçek registry/process/socket/module/hook çağrısı yapılmaz. LastError TLS metadata'sı ve test worker'ı bu programın kendisine aittir. Ayrı kaynak incelemesinde hata-politikası lifecycle yarışı tespit edilip düzeltilmiştir; kalan CRT/SEH/full-runtime eşdeğerliği açık kalır.

