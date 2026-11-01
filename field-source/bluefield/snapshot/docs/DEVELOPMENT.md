# Geliştirme — FIX31

Tek runtime dosyasında iki ek satır:64MiB kalan scan bütçesine sığmayan region read öncesi false. Bu REVIEW31 exact-base proposalıdır; sayıyı değiştirmez ve taranmayan bölgeyi güvenli saymaz. Diğer122 runtime ve bütün vendor bytesı korunur. Önceki64 C++/header fixture unchanged.

test_unload_scan tek TUda actual production cppyi dahil eder. Linux testinde declaration subset + sentetik API etkileri, SEH syntax adapterı ve GNU linker memcpy gözlemi kullanılır. Compiler fortify inline memcpyyi gizlediği için yalnız bu targetta -U_FORTIFY_SOURCE/-D_FORTIFY_SOURCE=0 ve -fno-builtin-memcpy gerekir; üretim hardeningi değişmez. MSVC kolu actual VirtualAlloc/VirtualQuery fixtureını derler, shimi kullanmaz.

Parent body, false-safe ve off-by-one mutantları aynı unchanged test driverla reddedilmelidir. Source algorithm kopyası oracle değildir; test actual body okuma izini gözler. SDK layout/SEH/thread suspension ve bütün unload güvenliği bu modelin kanıtı değildir. Native NOT_RUN ayrı tutulur.
