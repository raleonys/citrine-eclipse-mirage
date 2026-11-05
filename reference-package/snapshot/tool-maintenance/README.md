# Araç güncellemesi — 2 Kasım 2026

| Araç | Önce | Kurulan sürüm |
|---|---|---|
| REA | 6.5.0 | 6.8.0 |
| REA becerisi | 42 / 207 araç | 49 / 229 araç |
| Node.js | 24.20.0 | 26.13.0 LTS |
| npm | 11.19.0 | 12.2.1 |
| Java | 21.0.12.1 | Temurin 25.0.5+8 LTS |
| LLVM-MinGW / Clang | 20261006 / 23.1.3 | 20261020 / 23.1.4 |
| Ghidra bağımsız | 12.1.4 | 12.2 |

REA 6.8.0 yalnız Ghidra 12.1.x kabul ettiğinden **REA sağlayıcısı 12.1.4'te
tutuldu**. Ghidra 12.2 `C:\tools\ghidra_12.2_PUBLIC` altında ayrıca kurulu;
Java 25 ile headless yardım ekranı açılarak başlatma doğrulandı. REA'nın
`unsupported_version` denetimi değiştirilmedi.

Yeni PowerShell oturumunda çalışma araçlarını seçmek için proje kökünden:

```powershell
. .\work\use-tools.ps1
```

Bu betik yalnız o kabuğun ortamını ayarlar; sistem PATH'ini değiştirmez ve derleme
başlatmaz. `outputs/source-workspace/build.ps1` varsayılanı yeni LLVM-MinGW
klasörüdür; `-ToolchainRoot` veya `ANADIUS_TOOLCHAIN_ROOT` ile açık seçim korunur.
Geçmiş aşamalara ait betiklerdeki sabit sürümler ve kanıt kayıtları değiştirilmedi.

Codex REA kaydı yeni Node/REA/Java yollarıyla güncellendi. Diğer sunucu ayarları
ve araç onayları korundu. Açık sohbetin eski sunucuyu bırakıp yeni araç kataloğunu
alması için Codex yeniden başlatılmalıdır. Ayrı yeni MCP bağlantısında 6.8.0,
229 araç, hizalı sunucu kimliği ve kullanılabilir Ghidra doğrulandı. Hedef açılmadı.

Python 3.15.2 kurulum komutu otomatik onay denetiminde **blocked by policy**
gerekçesiyle reddedildi. Python güncellemesi tamamlanmadı; Codex'in yönettiği
Python 3.12.14 ve Git kurulumları korundu. REA'nın kullanılmayan isteğe bağlı
Frida kurulum script'ine ek onay verilmedi; dinamik analiz hazır iddiası yoktur.

İndirilen dört arşiv resmi SHA256 değerleriyle doğrulandı. Eski araçlar ve
`work/tool-upgrade-backups-20261102` altındaki Codex ayarı/beceri/build betiği
yedekleri korunuyor. **DLL derlemesi, proje testleri, link, DLL çalıştırma veya
yeniden ZIP oluşturma yapılmadı.** Önceki derleme sonuçları LLVM 23.1.3'e aittir;
yeni araçlarla kaynak uyumluluğu henüz derlenerek doğrulanmadı.

[Sürümler, yollar, hash ve kontroller](2026-11-02.json)
