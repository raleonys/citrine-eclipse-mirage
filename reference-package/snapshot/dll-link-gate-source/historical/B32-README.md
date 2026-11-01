# B32 güncellemesi

Son ortak kaynak build'i B32:249nesne/293normalgrup/19terminal senaryo. 846 önceki
kaynak ve iki B31 gate kaynağı korunur; dört B32 kaynak dosyasıyla852trackedfile.
B32 actual026880/026570/017420/026290 ve somutRecoveredSessionControl'ü ekler;
üretim allocator/free/contextdomain, executor/accept/server/raw-job/loader bağları açıktır.
Bu klasördeki son validation/source-verification, B32 common archive/snapshot
üzerinde tekrar edilen failed-link tanısıdır. Gate kodu B31'dekiyle aynıdır;
CMake hedefi hâlâ yürütülmemiştir. ToplamDLL yüzdesi bilinmiyor ve DLL BLOCKED.
Eski B31 kayıtları historical/ altında, asıl kaynak gelişimi
[B32 raporunda](../anadius-session-dispose-source/README.md) bulunur.

---

# B31 — DLL bağlama hedefi ve gerçek üretim engelleri

30 Ekim 2026. **DLL üretimi BLOCKED.** Bu paket çalışan bir anadius64.dll değildir.
Önceki B30 kaynak/test paketi korunur. Yeni teslimat, DLL hedefini gerçek kaynak
köklerine bağlayan ve eksik üretim bileşimini görünür kılan bir entegrasyon kapısıdır.

## Ne yapıldı?

- Ana CMake projesine varsayılanı kapalı `RECOVERED_ENABLE_DLL_LINK_GATE` seçeneği
  ve `anadius64_reconstruction` SHARED hedefi eklendi.
- Compiler CRT'nin `DllMainCRTStartup` girişi seçildi. Recovered CRT entry ikinci
  kez çağrılmıyor. Bu seçim özgün MSVC CRT/ABI eşdeğerliği iddiası değildir.
- Aday `DllMain`, zorunlu üretim attach factory'sinden gelecek gerçek Runtime
  üzerinden uygulama girişini çağırıyor. Thread reason'larında uygulama işi yok.
- `CDX @1`, `RUNE @2`, `anadius @3` aynı assembly RET leaf'ine alias olarak
  tanımlandı. Leaf'in nesne kodu tek `C3` talimatıdır; dönüş değeri uydurulmadı.
- Gerçek LLVM-MinGW compile/link denendi. Linker tam iki zorunlu entegrasyon
  sembolünün tanımı olmadığı için durdu. **DLL ve son link map oluşmadı.**
- Linker kaydı `bootstrap_factory`, `bootstrap_owner`, `bootstrap_native`,
  `bootstrap` ve `bootstrap_backend` dahil 25 core arşiv üyesinin seçildiğini
  gösteriyor. Bu, arşivi yalnız komut satırına eklemekten daha somut bir kontroldür;
  bütün üretim sağlayıcılarının kurulmuş olduğunu kanıtlamaz.

## Sonuç nasıl okunmalı?

`validation.json` içindeki `PASS_EXPECTED_BLOCKER`, tanılamanın beklenen eksikliği
doğruladığı anlamına gelir. Normal build **exit 1** verir. DLL derlemesi başarılı
sayılmaz; toplam DLL hazırlık yüzdesi ölçülmüş değildir. İki undefined sembol,
iki native fonksiyon kaldığı anlamına gelmez: ikisi de bu pakette tanımlanan yeni
entegrasyon sınırlarıdır ve çok sayıda zorunlu sağlayıcıyı temsil eder.

1. `require_loader_attach_runtime`: tek sahibi olan somut Resources grafiği,
   başlangıç değerleri, kalıcı Errors/Binding, worker/callback ömrü ve attach
   rollback politikası gerekir. Mevcut Runtime kontrol sözleşmesi loader lock
   dışındadır; onu doğrudan burada kurup publish etmek sözleşmeyi ihlal eder.
   Null yalnız tam rollback sonrası DLL yükleme başarısızlığıdır.
2. `require_loader_detach_quiescence`: normal FreeLibrary, süreç sonlanması,
   başarısız attach ve CRT yıkım sırası ayrılmalıdır. Loader altında worker join
   veya kanıtlanmamış Runtime::close çağrısı eklenemez.

`DllMain` noexcept'tir; yanlış phase sözleşmesinin exception'ı DLL ABI'sinden
çıkmaz, terminal olur. Bu, native EH davranışının çözüldüğü anlamına gelmez.
Nihai PE oluşmadığı için EAT alias RVA'ları, importlar, TLS ve unwind doğrulanmadı.

## Tekrar çalıştırma

Önce B30 `dll-reverse-engineering.zip` paketini açın; bu ek paketi aynı outputs
ağacına uygulayın. Kaynaklar, kayıtlı arşivler ve LLVM-MinGW toolchain mevcut olmalı.
PowerShell betiği 846 dondurulmuş kaynak hash'ini, common build kaydını, compiler'ı
ve iki arşivi doğrular; uyumsuzlukta link denemesine geçmez.

```powershell
# Normal build: üretim factory tanımları eksikken exit 1 beklenir.
& ./outputs/dll-link-gate-source/build-link-gate.ps1
# Tanılama: beklenen blocker doğrulanınca exit 0; DLL başarısı anlamına gelmez.
& ./outputs/dll-link-gate-source/build-link-gate.ps1 -Diagnose
python ./outputs/dll-link-gate-source/verify-link-gate.py
```

CMake kurulu ve LLVM-MinGW ile yapılandırılmış bir Windows x64 build'de:

```text
cmake -S outputs/source-workspace -B work/cmake-dll -G Ninja -DCMAKE_C_COMPILER=<toolchain>/bin/clang.exe -DCMAKE_CXX_COMPILER=<toolchain>/bin/clang++.exe -DRECOVERED_ENABLE_DLL_LINK_GATE=ON
cmake --build work/cmake-dll --target anadius64_reconstruction
```

Bu makinede CMake/Ninja bulunmadığı için CMake hedefi çalıştırılmadı. Eşdeğer
PowerShell compiler/link komutları çalıştırıldı ve tam argümanları kaydedildi.
291 eski test grubu bu aşamada yeniden çalıştırılmadı; son ortak build B30'dur.
846 eski CPP/HPP kaynağı ve canonical native envanteri değişmedi. Yeni native
ABI/provider kapanışı 0; generic CRT kuyruğu duraklatılmıştır.

Öncelikler ve kabul ölçütleri `roadmap.json`, bağımsız inceleme
`production-root-review.md`, kaynak/link denetimi `source-verification.json`
içindedir. REA 6.5.0 ve kullanılabilir Ghidra 12.1.4 oturum kimliği kontrol edildi;
bu aşamada orijinal hedef açılmadı, okunmadı, çalıştırılmadı veya değiştirilmedi.
Yeniden oluşturulan DLL/oyun da çalıştırılmadı; gerçek kullanıcı verileri okunmadı.
