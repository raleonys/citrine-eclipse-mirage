# B38 — Socket fabrikası, holder reset ve accept retry

Kayıtlı Ghidra C/ASM üzerinden üç normal C++ fonksiyon gövdesi yeniden yazıldı. Özgün DLL açılmadı, çalıştırılmadı ve değiştirilmedi. REA 6.5.0 sunucu kimliği gözlendi; Ghidra 12.1.4 kullanılabilir. Karşılaştırılacak dış sunucu kimliği verilmediği için kimlik eşleşmesi ayrıca doğrulanmış sayılmıyor.

| Özgün adres | Yeniden yazılan davranış |
| --- | --- |
| `0x180029df0` | Socket oluşturma; hata yakalama ve IPv6 seçeneğinin sırası |
| `0x180029590` | Soket tutucusunu sıfırlama; geçici close hatasında ioctl ve ikinci close |
| `0x180029660` | Accept yeniden denemesi; iş sayacı, hata yayını, AcceptEx ve iptal sırası |

`helpers.cpp` bu gövdeleri, `bindings.cpp` çalışma zamanı bağlantılarını içerir. Socket oluşturma başarıda yalnız `Error.value` alanını sıfırlar. Sıfırlama fonksiyonu normal dönüşte tutucuyu geçersiz yapar. Retry, erken iptal için `995` bildiriminden sonra devam eder; kaynak bu sırayı korur. Sahipli bağlantı yalnız ana işlem kaydı ve boş iptal bağlamı profilini destekler. Çoklu yayın ve alternatif işlem kaydının sonlanması kanıtlanmadığı için bu profiller reddedilir.

Retry, SDK çağrısından önce sahipli 473 baytlık adres aralığını, aynı işlem kaydını ve bellek ödüncünü yeniden doğrulayan `prearmed_api()` kullanır. Retry gövdesi sürerken callback içinden erken completion, free veya yeniden kullanım engellenir. İptal isteği işlemin tamamlandığını kanıtlamaz. Alt çağrı istisna fırlatırsa etkileri bilinmeyen kaynaklar tutulur; kanıtlanmamış bir geri alma uygulanmaz.

B38 normal gövdeleri toplam DLL tamamlanma yüzdesini ölçmez. Tam Runtime/Resources fabrikası, native mesaj bağlantısı, iş/bağlam/kernel işlemlerinin güvenli sonlandırılması ve loader attach/detach bağlantıları açık kalır. Genel CRT çalışmaları ertelenmiştir. Sonraki paket B39, öncelik 1; kullanıcı sırası 1–2–3–5–4 korunur.

[Kanıt](evidence.json), [bağımsız inceleme](architecture-review.md), [kaynak sınırları](resources-audit.json).

## B38 ortak doğrulama

**749 çekirdek + 197 entegrasyon = 946 kontrol PASS.** Ortak build 258 nesne (254 C++ / 4 C), 305 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 874 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 882 kaynak. Yeni typed normal gövde 3, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
