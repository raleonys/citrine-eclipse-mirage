# REDFIELD RF08 kaynak kimliği

NON_RELEASE_WORK / NOT_RELEASE. Exact RF07 parent SHA-256 `6ac0c012f67e87ec67e4d81099b7e26006ab9052a34d327291539adb8ecfe506`. Parent ZIP nested eklenmez; PARENT ve DELTA_FROM_RF07 kayıtları bağı taşır.

HF19 source ZIP SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`. RF07 1097 + RF08 1 = 1098 exact whole-file adoptions. Yeni exact query_presence_route.cpp reference'ıdır. Ayrıca 10 derived file (7 TU, 3 header) vardır; byte-identical sayısına katılmaz.

Özgün QueryPresence direct_first_text, validation ve emit fonksiyonları exact byte slices'tır. Handler'da global config lease acquire/release yerine explicit persona argument ve caller-owned immutable snapshot kullanılır; kalan validation/measure/render kontrol akışı kaynak byte'larıyla korunur. StreamWriter'ın iki ServiceTextView overload'u hariç implementation'ı exact fragment'tır. Shared response helper declaration'larına inline eklenir; body'leri değişmez ve duplicate helper implementation üretilmez.

ADOPTED_SOURCES ve DERIVED_SOURCES; origin, offset/length/fragment SHA, scaffolding ve destination SHA'yı ayırır. Her compile/header/test/tool/provenance/recipe input'u build-input digest'ine girer. Evidence/report çıktıları hariçtir; final ZIP ayrı CRC/manifest/SHA kimliğidir. Direct reuse clean-room yazarlık veya donor parity diye sunulmaz.

RF08, current Library metadata/governance ve exact local RF07/HF19 byte'larını revalidate eder; diğer SOURCE_INVENTORY arşivleri RF00 tarihsel kayıtlarıdır. Tüm Library'nin fiziksel incelendiği iddia edilmez. Önceki PASS receipt'leri yeni source'a taşınmaz. Windows/native/game kabulü NOT_RUN, GetConfig diagnostic exit14 olarak açıktır. Resmî HF19/CURRENT ve eski checkpoint'ler immutable kalır. GPL-3.0-only/upstream lisansları ve pinler korunur.
