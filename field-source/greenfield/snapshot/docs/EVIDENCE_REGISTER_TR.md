# Library kanıt envanteri — kapsam ve doğrulama sınırları
`metadata/LIBRARY_EVIDENCE_REGISTER.json` üst klasörler ve ilgili alt araştırma hatlarının kayıtlarını içerir. **Bu çalışma Library'deki her iç içe dosyanın tam SHA taraması değildir**; buna karşılık bütün temel kanıt kategorileri ve öncelikli dizinler tanımlanmıştır.

Exact teyit edilen otoriteler:
- Library `PROJECT_RULES_MASTER.md` ve `.json`, revizyon `2026-10-19.2` (GF00 harici giriş kanıtları).
- `CURRENT_ARTIFACT.json` güncel HOTFIX19'e işaret eder. Exact source ZIP SHA-256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`, 1898 üyeli, CRC PASS.
- OriginSDK, Maxima, EAappEmulater, DLL reverse reconstruction input ZIP'leri tamamen hash ve CRC ile kontrol edilmiştir (ayrıntı `SOURCE_INVENTORY.json`).

Önceki araştırmaların transfer kararları:
- QCE02–QCE05 / RG01–RG05: filtre OR-within / AND-across; duplicate full replace hipotezi native response olmadan kesin kabul edilemez; item synthetic ID ordering exact değil.
- HF16 Registry B: provenance ve tracking OOM P0 — yeni tasarımda raw HKEY heuristics kullanma.
- HF18/19 owner loss: `FreeLibrary(FALSE)` ve cleanup retry sahibi kaybolmasın.
- VDF 1.3.0: parser/Config2 ayrımı, invalid input atomic publication.
- 11-hook: optional platform adapter, hook zorunlu temel değil.
- RC/Final governance: Final-specific native/game G3, other classes truthful not-applicable; yalnız açık onayla yayın.

Eksik veya yalnız harici kanıt: özgün Anadius gerçek LSX XML 0/64; yeni greenfield native x64 build henüz Windows'ta çalıştırılmadı; geçmiş Library klasörlerindeki bütün dosyalar byte düzeyinde taranmadı.
