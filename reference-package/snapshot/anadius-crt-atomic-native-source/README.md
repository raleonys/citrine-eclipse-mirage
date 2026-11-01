# Sahipli Win32 atomik hücre

`anadius::crt_atomic_native::Cell32`, kaynak sağlayıcılarının kullanabilmesi için gerçek ve 4 bayta hizalı bir Windows `LONG` alanına sahip olur. Bu aşamada **yeni özgün DLL fonksiyon gövdesi eklenmedi**; mevcut kaynakların atomik işlem gereksinimine yeni bir native uç sağlandı.

`fetch_add(uint32_t)` ve `exchange(uint32_t)` önceki değerin tüm 32 bitini döndürür. `load() const`, `InterlockedCompareExchange` ile atomik okur. Dönüşümler `bit_cast` kullanır; toplama ve `0xffffffff` ile azaltma 32 bit çevrimini korur. Derlenen `atomic.o` içinde sırasıyla `lock xaddl`, bellek hedefli `xchgl` ve `lock cmpxchgl` görülür (`build/atomic.disasm.txt`). Bunlar **yeni derlenmiş nesnenin** kanıtlarıdır; özgün DLL'nin adresleri değildir.

Hücre kopyalanamaz ve taşınamaz. `address()` yalnız sahip olunan hücreyi sağlayıcıda tanımak için kullanılabilir. Keyfi `uint32_t`, byte dizisi, kurtarılmış kayıt alanı veya özgün DLL adresi `LONG*` olarak kabul edilmez. Kaynak kayıttan gelen kimlikler açık bir sahipli hücre tablosunda eşlenmelidir. Hücrenin ömrü eşzamanlı kullanıcıları kapsamalı; yok etmeden önce bütün kullanıcılar bitmiş olmalıdır. `load` fiziksel olarak atomik RMW kullanır, dolayısıyla salt okunur belleğe uygun değildir. LastError üzerinde düzeltme veya sıfırlama yapılmaz.

Mevcut `0x1800647d4` retain ve `0x18006489c` release kaynakları `atomic_fetch_add_32` sağlayıcısı ister. Bu modül onların sağlayıcısına bağlanabilir; kendisi bu kaynak zincirini çalıştıran bir entegrasyon değildir. Gerçek kayıt ABI'si, özgün global adres düzeni, DLL yaşam döngüsü, SEH/unwind ve özgün hedef ile çalışma zamanı eşitliği kanıtlanmış değildir.

`build.ps1` mevcut LLVM-MinGW ile C++20, `-O2 -Wall -Wextra -Werror` kullanır. Standalone test **88 kontrol** geçirir: tam bit değerleri, sınır değerlerde taşma/azalma, önceki sonuçlar, hizalama/kimlik, LastError ve 8 kendi iş parçacığının 50.000'er artış/azalışı. Artışların son toplamı ve dönen önceki değerlerin toplamı doğrulanır; toplam kontrolü tek başına bir benzersizlik ispatı olarak sunulmaz. Test yalnız kendi hücrelerini ve test EXE'sini çalıştırır. Özgün DLL/oyun yüklenmez, girdiler değiştirilmez.

Adres ve retained REA kanıt kimlikleri `evidence.json`, kaynak/derleyici kimlikleri `provenance.json`, çalıştırılan komutlar ve kaynak hashleri `validation.json` içindedir. Bu modül tek üretim CPP ve tek normal test grubudur.
