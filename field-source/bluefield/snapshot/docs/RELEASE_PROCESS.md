# Build ve yayın — FIX31

Bu paket NON_RELEASE tam uygulama kaynağıdır. Windows derlemeleri LOCAL_BUILD üretir; resmî yayın veya canonical pointer değiştirmez. Yerel MSVC/CMake/build/QA G0, yeni yayın izni veya oyun sonucu bekletilmez. Mevcut .cmd launcher düzeni, CRT/PE/ABI kontrolleri ve Production Golden zinciri korunur.

Ayrı bir resmî promotion için en yeni identity-specific yayın yetkisi ve güncel masterın uygulanabilir release-class kontrolleri gerekir. Yalnız FINAL_RELEASE için mandatory game/native acceptance kuralı geçerlidir. Bu görev BROWNFIELD/resmî artifact veya diğer FIELD projelerini değiştirmez.

Paketleme mevcut package_source/check_package araçlarıyla source manifest, CRC/path/hash kontrolü yapar. Final source digestinden sonra source değişikliği receiptleri geçersiz kılar. Exact teslim ZIP bir kez temiz açılıp yeni nesnelerden Production testi yürütülür; sonuç dış FINAL_QA içinde exact ZIP hashine bağlanır, döngüsel hash için ZIP yeniden değiştirilmez.
