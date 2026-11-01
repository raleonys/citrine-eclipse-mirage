# Üretim Resources denetimi

Bu bir mevcut kaynak denetimidir; özgün runtime gözlemi değildir. Runtime prepare yalnız files/input/library_os/execution/tamamen-boş join API varsayılanlarını kurar. Somut wrapper sınıfı veya17/17 Services seçimi, bütün alt sağlayıcıların üretime hazır olduğunu kanıtlamaz.

| Alan | Kaynakta gözlenen durum |
|---|---|
| state_lifetime | optional owned backing lifetime; not a factory |
| configuration | B5/B7/B8/B9/B11 selected19/21; legacy document and KeyForLicense remain |
| files | Runtime default NativeFactory when null; original MSVC file/EH ABI remains unproven |
| input | Runtime default NativeInputPlatform when null; not exercised in B11 |
| console | B5 CompilerConsole available via explicit binder; caller must supply before SourceBackend |
| threads | B5/B6 NativeThreads/NativeRuntime available; caller must assemble shared domain and lifetime |
| remaining | mandatory caller Services; B3/B4/B7/B9/B10 selected composition17/17 does not supply all lower foreign resources |
| execution | Runtime default NativeExecution when null; native mutex/cache/concurrency contract remains |
| server | mandatory ServerBackend; SourceBackend executes server algorithms but is not a concrete complete lower network backend |
| online | mandatory lower online Effects; OnlineExecution only binds wait/ready and forwards foreign module/object operations |
| library_os | Runtime default NativeSystemCalls when null; module loading must not be executed in this investigation |
| library_effects | mandatory logging/UI/error lower Effects; LibraryExecution only binds wait/ready; next high-value production boundary |
| host | mandatory host-init Effects including callback/original/provider domain; wrapper does not implement missing foreign ABI |
| hook_cleanup | mandatory lower hook cleanup Effects; source cleanup algorithm and pinned engine fixtures do not install production hooks |
| stop_sources | mandatory external admission stopper; callback wrapper additionally pauses registry but does not stop all source admissions by itself |
| join_api | Runtime chooses native_join_api only if entirely empty; partial API rejected |
| join_timeout_ms | explicit caller policy; no inferred binary default |
| RuntimeSeed and InitialState | all source globals/native defaults caller supplied; no complete shipped image seed or PE entry installed |

Yüksek değerli sonraki parça library_effects logging/UI/error sınırıdır. Ana yolun atladığı legacy load_document, yalnız21/21 sayısını tamamlamak için öncelik yapılmaz. KeyForLicense ayrı dosya/şifreleme alt sistemi olarak açık tutulur. Native CRT/ABI, genel eşzamanlılık, foreign arayüzler, image unload, loader-lock ve PE giriş kapıları açık.
