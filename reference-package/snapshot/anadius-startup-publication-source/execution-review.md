# B19 bağımsız Execution incelemesi

**PASS.** Reviewer `/root/b18_ptd_audit`; üretim kodunun yazarı değildir. Dokuz kaynak hash'i ve iki EXE hash'i final quick kayıtlarla doğrulandı. Root'un **19 çekirdek +16 entegrasyon =35 PASS** sonucu incelendi; reviewer testleri yeniden çalıştırmadı. Genel architecture review ayrı rapordadır.

SourceGuard; acquire_checked, native owner/count, flag/predicate ve native unlock işlemlerini tek kısa bölümde korur. Clock/cache/QPF/QPC ayrı kısa bölüm kullanır. Polling Sleep veya observer çağrısı boyunca source gate tutulmaz. Yeni lock/unlock bookkeeping'i Win32 LastError'ı korur; Platform etkilerini silmez.

Native error5/6, overflow count adjustment ve exception prefix'lerine otomatik native unlock/rollback eklenmedi. Unlock exception sonrası yazılmış flag/count durumu korunur fakat milestone yayınlanmaz. Yeni quick testleri error5 ve post-release exception içerir; error6 prefix'i source-reviewed olup mevcut legacy Execution suite'i ortak build kapsamında yeniden doğrulanacaktır. Unknown native outcome için retry varsayılmaz.

Kendi waiter thread'i Sleep'teyken ayrı finisher ilerler. Observer, başka Execution clock çağrısının tamamlanmasını bekleyebilir; source guard observer öncesinde bırakılmıştır. Sekiz own thread'in800 clock örneğinde frequency cache bir kez sorgulanır. Bu, aynı wrapper'ın korumasıdır; raw helpers, dışarıdan plain state değişimi veya aynı refs üzerinde ikinci Execution bu gate'e katılmaz.

Milestones exact flag adreslerini ve tek Execution kimliğini kullanır. Seed edilmiş1 değerler yayın değildir; ilgisiz flag bit üretmez; duplicate event eksik stage'i doldurmaz. Alias refs/rebind reddedilir, foreign issuer readiness'i invalid yapar. Atomic acq_rel RMW zinciri ve all-bits acquire, event öncesindeki plain payload yazılarını yayınlar; test join'den önce bu payload'ları okur.

Host milestone yalnız host init'in normal dönüşünü gösterir; engine başarısı değildir. Server milestone async_accept/run_context öncesidir ve worker'ın sonlanması değildir. Library/online event'leri kendi initialization prefix'lerini yayınlar. Entegrasyon, kendi canlı server worker'ı run_context'te beklerken foreign callback'in yayınlanmış portu görmesini ve engine-failure durumunda kalan provider proof'un readiness'i kapalı tutmasını doğrular.

Resources observer'ı Execution'dan uzun yaşatır; matched factory actual Runtime Execution kimliğini denetler. Execution immovable'dır; observer başka Runtime için reset/rebind edilemez. Resources ve Execution source ABI değişiklikleri bütün tüketicilerin yeniden derlenmesini gerektirir; recovered MutexState layout'u değiştirilmez.

**Açık sınırlar:** Platform lock/QPC sınırları kısa ve non-reentrant native SDK sözleşmesini izlemelidir. Arbitrary throw/reentry, doğrudan helper erişimi, ikinci wrapper, diğer provider/stream/fallback/OOM paylaşımı bu değişiklikle çözülmez. published noexcept bookkeeping olmalıdır. Milestone, worker/FLS join/quiescence, disabled hook veya unload kanıtı değildir; ayrı provider proof hâlâ zorunludur. Worker event'i callback activation çağırmaz. Özgün CRT/native ABI ve tam DLL eşdeğerliği açık kalır.

Original/rebuilt DLL veya game çalıştırılmadı; orijinal/AV girdilerine dokunulmadı. Yeni native gövde/kapanış0.
