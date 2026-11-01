# preloader_l mimarisi

Windows loader yalnız CRT worker başlatır. Worker module/EXE/LocalAppData/ProgramData yollarından immutable RuntimePaths ve Environment kurar. Application constructor → CLI → optional base → optional override sırası ve mevcut profil/Config2 davranışı korunur. Asio listener yalnız kurulum tamamlandıktan sonra ready olur; terminal dispatch hatası stopping/ready=false yayınlar ve listener kapanır.

RuntimePaths config/provider/asset/cache/log/persistence adreslerinin sahibidir. Environment readonly view sağlar. Files NativeFactory salt okunur default kullanır; yazım için policy sahibi zorunludur. Atomic I/O sibling exclusive stage, flush, replace ve batch preimage rollback uygular. Achievement belleği ve hazırlanan event ancak persistence başarılı olunca commit edilir. Batch rollback tamamlanamazsa PersistenceRecoveryRequired uygulamayı durdurur ve kurtarma preimage dosyalarını korur.

AvatarStore yalnız resmi byte dizisini lazy üretir. Default/custom seçimi online initialization boyunca korunur. Başka kullanıcı provider avatarı nonce dizininde geçici olarak tutulur, limit ve PNG IHDR denetiminden geçer; kendi profil cache'i resmi/custom seçimi değiştiremez. Unload yalnız bu instance'ın sahipli download dosyalarını kaldırır.

Kaynak ve dış sözleşme sınırları MIGRATION_FIX12.md ve FILESYSTEM_FIX12.md içindedir. BROWNFIELD HF19 sink tabanlı achievement yolu bu uygulamanın XML/INI persistence yoluyla aynı değildir.
