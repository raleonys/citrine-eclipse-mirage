# REDFIELD RF35 — MSVC namespace düzeltmesi

Kullanıcının RF33 logunda 16 translation unit derleniyor; 17. dosya
src/net/tcp_ingress.cpp, C2386 ile duruyor. Bu derleme aşamasındaki hata,
regresyonların veya DLL linkinin çalıştırıldığı anlamına gelmez.

frame_ingress.hpp zaten preloader_l::net namespace'ini tanımlar. tcp_ingress.cpp
aynı preloader_l kapsamı içinde `namespace net=preloader_l::net;` yazar. MSVC
bu tekrar bildirimi reddetmiştir. Gereksiz alias kaldırıldı; tüm net:: ifadeleri
zaten mevcut namespace'e çözülür. GCC önceki satırı kabul ettiği için taşınabilir
derleme bu MSVC uyumsuzluğunu yakalamamıştır. Uygulamadaki diğer aliaslar kaynak
bağlamlarıyla incelendi; başka aynı-kapsam self-alias bulunmadı. Bu inceleme diğer
tüm olası MSVC/SDK sorunlarının dışlandığı iddiası değildir.

RF35, doğrulanmış RF34 ZIP'inden üretilmiştir; RF34 lifetime çalışması korunur.
268 application dosyasından 267'si birebir aynı; tek fark yukarıdaki satırın
silinmesidir. Build graph, kaynak/rol ayrımı, export/resource/vendor içerikleri,
7 hook ve 35 binding aynıdır. Build entrypoint labels RF35 oldu; ortak logdaki
yanıltıcı sabit RF32 başlığı sürümsüz proje başlığına çevrildi. İnsan entrypoint
envanteri, CMD argümanları/çıkış/hold davranışı değiştirilmedi.

Temiz izole kaynak kopyasında 58 core, 2 test-support ve 99 program main nesnesi
derlendi; 99 executable link edildi. Normal 98 regresyon PASS; RF34'ün 98 stdout/
exit sonucu birebir aynı. Tarihsel GetConfig exit14 ayrıca FAIL_OBSERVED kaldı.
Bu tur yeni sanitizer veya Golden çalıştırılmadı; önceki kanıtlar tarihsel
RF34 kaynak sınırında tutulur. Mevcut 16 qualified dosya ve açık proof dependency
hashleri aynıdır; güncel normal derleme tekrar doğrulandı. Yeni qualification
verilmedi ve yeni modül incelemesi kapatılmadı.

Gerçek Windows/MSVC, native/CRT/Debug ve oyun testleri bu ortamda NOT_RUN.
Gösterilen C2386 bildiriminin kaynağı kaldırılmıştır; Windows build zincirinin
kalanı için güncel MSVC sonucu beklenir. M09'un bütün incelemesi, M12 ve M20 dahil
14 modül açık kalır. Oyun profili bulgusu RFCL-005 ile bu derleme hatası eşitlenmez.
BROWNFIELD HF19/CURRENT veya official release kimliği değişmedi.
