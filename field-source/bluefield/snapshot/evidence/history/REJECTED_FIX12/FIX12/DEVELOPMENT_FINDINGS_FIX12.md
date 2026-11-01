# Geliştirme sırasında bulunan ve kapatılan durumlar

İlk migration testi başlangıçtaki gereksiz INI yazımını ortaya çıkardı. Storage initialization salt okunur hale getirildi; XML/INI ilk gerçek achievement mutation transaction'ına taşındı. Son Debug/Production/ASan turları aynı son source digest üzerinde geçti.

Bir ara shared workspace derleme turunda link output ELF imzası geçersiz çıktı; testlere geçilmeden FAIL olarak durduruldu. Son derleme ve link ara dosyaları ayrı /tmp build kökünde yeniden üretildi ve executable/object ELF denetimleri korundu. Kaynaklar yeniden donduruldu, başarısız ara tur final kabulüne dahil edilmedi.

CMake bu ortamda kurulu değil. Beş profil tanımı source-policy ile kontrol edildi; gerçek logger object'leri doğrudan g++ ile derlenip çalıştırıldı. CMake configure ve Windows/MSVC yapılandırması NOT_RUN. LSan detect_leaks=1 denemesi ptrace ortamı nedeniyle fatal hata verdi; ASan/UBSan 34/34 detect_leaks=0 sonucu leak-free kabulü değildir.

Temiz son ZIP extraction/rebuild ve exact archive hash bağı dış FINAL_QA raporundadır; ZIP'in kendi SHA'sını içinde saklayarak döngüsel doğrulama yapılmaz.
