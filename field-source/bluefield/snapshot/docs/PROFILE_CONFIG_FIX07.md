# FIX07 — profil ve config

Kullanıcı son sürümde dosyasız başlangıç ve profilin düzgün yüklendiğini doğruladı.
Exact DLL hash'i veya MSVC/Golden receipt'i verilmedi. Bu kullanıcı runtime
bildirimidir; FIX07'nin Windows kabulü değildir.

## Eski örneğin etkisi

Gönderilen zsurge.cfg çalışma sırasında surge.cfg adıyla kullanılmış. İçerik eski
sentetik örnektir; bütün byte'ları legacy_synthetic.cfg fixture'ında korunur.
Yerel C++ Application/Service/GetProfile probunda Persona=Player, PersonaId=1,
UserId=1, IsSubscriber=false, SubscriberLevel=0, SDK=9,9,0,0 ve FakeAuth=0 olur.
Bu dosya burada okunuyor, anadius üretmiyor. Tüketicide görülen anadius'un exact
nedeni bu kanıtla kapanmaz. Yüklü modül/config konumu, override veya online
sağlayıcı/cache etkisi runtime loguyla ayrıştırılmalıdır.

Yeni log yüklü modülün FIX07 kimliği/yolu yanında base ve override config yollarını,
seçilen User anahtar adlarını, provider_authenticated ve identity_changed durumunu
yazar. User değerleri yazılmaz. Kimlik değişmediği sonucu, cache okunmadığını tek
başına kanıtlamaz. Cache ve provider önceliği değiştirilmez.

## Profil alanları

Eski User bölümü dokuz configurable alanı zaten içerir; sorun eksik anahtarlar
değil, çalışan defaults'un sentetik değerlerle değiştirilmesidir.

| Config veya kaynak | GetProfile alanları |
|---|---|
| Username | Persona |
| PersonaId, UserId | PersonaId, UserId |
| Country | Country, GeoCountry, CommerceCountry |
| Currency | CommerceCurrency |
| IsSubscriber, IsSteamSubscriber | Aynı adlı alanlar |
| SubscriberLevel, IsUnderAge | Aynı adlı alanlar |
| Handler sabitleri | IsTrialSubscriber=false, UserIndex=0, AvatarId boş |

Bu dokuz alan tam 14 attribute'lu yanıtı sağlar. Yeni profil şeması/anahtar eklenmez.

## Düzeltme

Varsayılan display name kullanıcı seçimiyle SURGE olur. Çalışan sayısal kaynak
ID'ler korunur; yeni rastgele kimlik veya save kimliği geçişi yapılmaz. Açık Username,
override ve başarılı online provider önceliği sürer. Online sürüm-hatası penceresinin
başlığı da SURGE olur. Eski constructor kanıtındaki anadius tarihsel veri olarak kalır.

surge.cfg.example no-config ile aynı profile/SDK/flags/game/settings durumunu üretir.
FakeAuth ve PretendConnected true; SDK override yoktur, yerleşik 10,5,96,47512 kalır.
Game bilgisi yalnız yorum örneğidir; BLUEFIELD/1.0 gibi uydurma oyun kimliği uygulanmaz.
Achievement/content/entitlement tanımları profilin yüklenmesi için zorunlu değildir.

anadius64online.dll ve özgün cache yolu gerçek uyumluluk adlarıdır. Marka temizliği
adına değiştirilmez. Golden VERSIONINFO/timestamp/internal-name alanları HF19
allowlist'ine bağlıdır; profil adından bağımsızdır ve bu düzeltmede korunur.

## Kullanım

Düzeltilmiş surge.cfg dosyasını gerçekten yüklenen preloader_l.dll ile aynı
klasöre koy. User.Username ile istenen isim verilebilir. surge_override.cfg aynı
alanları tekrar değiştirebilir. Başarılı online sağlayıcı üç kimlik alanını mevcut
öncelikle değiştirir; UseLastOnlineProfile=false başarılı sağlayıcıyı kapatmaz.

Test kapsamı: dosyasız tam profil; corrected config ile 14 alanın eşitliği;
eski dosyanın exact etkisi; override; eksik Config2 diagnostic'i; kaynak SDK ve
flag'ler; loglarda kişisel kimlik değerlerinin bulunmaması. Windows FIX07 henüz
çalıştırılmadı; tüketicideki son anadius görüntüsü kesin çözülmüş sayılmaz.
