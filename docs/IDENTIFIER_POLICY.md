# Aktif adlandırma sınırı

Kaynak kökenini anlatan belgeler ve tarihsel kanıtlar eski adları koruyabilir.
Aktif kaynak, yorumlar, araçlar, testler ve metadata bu adları taşımaz.
Yeni ürün şemaları `surge.*` ailesini kullanır. Vendor kaynakları byte pinleriyle
korunur; bu temizlik onların içeriğini değiştirmez.

Aşağıdaki liste kaynak QA tarafından okunur. Adların kontrol aracının içine
yeniden yazılması yerine belge içinde tutulması, aynı adlandırma sınırını
kontrolün kendisinde de korur.

<!-- active-name-policy -->
```json
{"reserved_reference_labels": ["anadius", "yellowfield", "bluefield", "brownfield"]}
```

`docs/` ve `governance/` altındaki belge içerikleri ile Markdown belgeler bu
kontrolün dışındadır. Kök LICENSE/COPYING/NOTICE/AUTHORS lisans ve atıf belgeleri
de korunur. Aktif dosya yolları da kontrol edilir. `build/`, `dist/`,
`.git/` ve sabitlenen `third_party/` payload'u aktif first-party taramasına girmez.
