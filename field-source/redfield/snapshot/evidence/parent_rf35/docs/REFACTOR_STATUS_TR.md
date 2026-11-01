# Refactor durumu — RF35

RF33 MSVC C2386 nedenli tekrar namespace aliası kaldırıldı; RF34 korundu.
98 temiz GCC regresyon PASS; önceki98 stdout/exit aynı; legacy14 ayrı tanı.
Kaynak review: 6 modül kapalı,14 açık. Yeni modül kapanışı yok; M09 TCP açık.
300 code kaydı:16 QUALIFIED,284 PENDING;4 vendor integration PENDING.
Gerçek Windows/MSVC/native/CRT/Debug/oyun bu tur NOT_RUN.
Sonraki kaynak işi M12; M20 carryover ve RFCL-005 halen açık.
