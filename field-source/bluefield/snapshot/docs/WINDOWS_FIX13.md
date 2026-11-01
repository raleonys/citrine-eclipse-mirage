# Windows operator kabulü — FIX13

Linux ortamında NOT_RUN. Kaynak ZIP ayrı fixture dizinine çıkarılır; gerçek hesap/oyun ilk fixture turunda kullanılmaz.

1. build_production.cmd: MSVC x64 compile/link, /we4702 ve gerçek PE/Golden beş gate receipt'i aynı DLL hash'ine bağlanmalıdır. CMake/MSVC kurulu olmayan Linux sonucu bu adımın yerine geçmez.
2. test_native.cmd ve test_crt.cmd: DLL loader, hook lifecycle, profile/GetConfig, provider version2 synthetic fixture, environment restore, DllShutdown 0 ve FreeLibrary. Native/CRT disk log oluşturmamalı.
3. Config olmadan profil, plain SURGE/surge.png, mevcut module sidecar, custom avatar ve eski last_avatar izolasyonu kontrol edilir. Resmi embedded asset exact korunur; mevcut sidecar için hash şartı yoktur.
4. Flat root cache: remid_cookie / last_profile; raw content, large cache ve eski davranıştaki field split. Provider yalnız module preloader_l_online.dll. Gerçek binary SHA/exports/vtable/lifecycle ayrı kabul gerektirir.
5. Achievement eksik INI startup'ta oluşmalı, var olan INI korunmalı; grant XML/INI hatasında önceki disk/memory/event korunmalı. Dış Achievement Watcher manuel custom root/notification gerçek uygulamada ayrıca test edilir.
6. >1MiB config/frame, 140 nesting, >32 session, >64MiB file, 128 üstü geçerli filename ve kaynak key side-effect sırası yeni native karşı örnekler olarak doğrulanır; kernel/CRT doğal limitleri keyfi uygulama limitleri sayılmaz.
7. Windows file sharing/reparse/error-code, short read, module unload, CRT locale/encoding ve kaynakta bildirilen adapter açıkları exact orijinal DLL oracle ile karşılaştırılır. Unknown alanlar PASS'e çevrilmez.

Production Golden zinciri MSVC → URSYNEX → ARYRA → MUSTAVEX → CORVEXON → FALCION. NON_RELEASE dışına yayın ve canonical pointer değişikliği yetkili değildir.

Yanlış provider sürümü özgün MessageBoxA uyarısını gösterir. Bu durum otomatik child listesinde PASS sayılmaz: --scenario bad_version ile ayrı fixture oluşturulup uyarı görülür/kapatılır ve offline cleanup doğrulanır. Pencereyi gizleyen yeni production/test-environment bypass eklenmez.
