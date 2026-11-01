# REDFIELD RF37 — Gerçek DLL ile Golden düzeltmesi

Kullanıcının failure.json'u ile627712byte DLL'i hash veGoldenstage zinciri
bağlandı. DLLSHA-256:508d915f9eeb5af50160e1238b257a1a4eb62974a466d21f73e4b120817a7bc3.
Failure receipt'teki3090inputhash exactRF36kaynağıyla birebir eşleşir. Receipt
aggregate'ı kendi sırasıyla yeniden hesaplandı;Linux/Windows pathlib sıralama
farkı aggregate dizisini değiştirir,sourcefilemap eşitliğini değiştirmez.

CORVEXON'da tek unexpectedmodule API-MS-WIN-CORE-SYNCH-L1-2-0.DLL'dir;required
modüller eksik değildir. Üçişlev WaitOnAddress,WakeByAddressAll,WakeByAddressSingle
Microsoft'un belgelenmiş Windows senkronizasyon API'leridir;API-MS-WIN-CRT ailesi
değildir. Runtimecaller'specificSTLfonksiyonu symbolmap olmadan kesinleştirilmedi.
Localbinding yalnız bu tam modül adını vevarsa exactüçnamedimportu kabul eder.
ImmutableHF19policies/provenance aynı kalır;effectivepolicy yalnız bellekte türetilir.

GerçekPE ileFALCION da incelendi. .dataVS114116byte,rawsize10752byte iken eski6KiB
sınırı uygulanamaz. Mevcut HostBridgeRuntimeState source'u32768char ve32768uint16
pathbuffer barındırır:98304byte. Runtimeownership veyaoyun davranışı sırf eski
boyutlimitine sığsın diye değiştirilmedi. Destinationbudget açık128KiB olarak
96KiBretainedpathstorage+32KiBboundedotherstate biçiminde tanımlandı. Actual kalan
15812byte'dır;tamCRT/symbolownership iddiası yoktur. Sourcecapacity/arrayshape
bindingguard'a bağlandı. Bu mevcutM13tasarımını optimal/kapanmış ilan etmez;
yalnız bilinen mevcut üretim yapısının M20bütçesini açıklar. Limit aşımı reddedilir.

Üç tarihi log kopyasındaki kişisel Windows path alanı redacted oldu;originals
Libraryattachment kimliklerinde immutable kalır. Eski sourceevidencehashleri
originalparent kayıtlarıdır;currentredactedhashler rf37_evidence_redactions.json
ile ayrılır. UserDLL external kalır;sourceZIP'e binary gömülmez.

ParentCORVEXON failure exactDLL ile tekrarlandı. GüncelCORVEXON geçti;15negatif
case unknown/dynamicCRT/missingrequired/ordinal/eksik-extra function/wildcard,
policy/sourcecapacity drift'i reddetti. Tam orderedbeşGolden stage actualuserPE'de
geçti;URSYNEX zaten finalizedinputta0byte değiştirdi veDLLSHA aynı kaldı.
Veribütçesi,privatepath veembeddedbinary için3hygiene negatif kontrol degeçti.
Current98normalGCCregresyon temizkaynaktan PASS;RF36'nın98stdout/exitaynı.
LegacyGetConfig14 ayrıFAIL_OBSERVED;oyunprofili nedeni RFCL-005açık kalır.

268application dosyası aynı;ABI,hook,route,resource,vendor,build graph/profiles
değişmedi.6modül source reviewkapalı/14açık,16qualified/284code+4vendorpending
aynı. M20bütün incelemesi bu scopedrepair ilekapanmaz. Sourcecurrenttools qualify
olmuş sayılmadı. RF37MSVCrebuild/native/CRT/Debug/game NOT_RUN. KullanıcınınRF36
compile/link ve3WindowsABIcyclePASS kaydı ayrı observational kanıttır.
HF19/CURRENT,semanticreleasekimliği vekodadları değiştirilmedi.
