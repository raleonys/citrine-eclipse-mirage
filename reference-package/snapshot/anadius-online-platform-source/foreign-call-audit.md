# Online foreign çağrı denetimi

Bu retained statik caller kanıtıdır; gerçek foreign modül çalıştırılmadı.

| Callsite | Gözlenen çağrı |
|---|---|
| 0x180035e62 | GetProcAddress version_check |
| 0x180035e8c | call returned procedure, compare EAX2 |
| 0x180035f60 | GetProcAddress create_online |
| 0x180035fac | call factory with UTF16 cookie-copy c_str in RCX |
| 0x180036002 | object vtable+08 status |
| 0x180036039 | vtable+10 username pointer |
| 0x18003606c | vtable+18 persona pointer |
| 0x180036098 | vtable+20 userId pointer |
| 0x180036703 | vtable+00 call, then zero object |
| 0x18003671c | FreeLibrary ignoring BOOL, then zero module |

03600d online bayrağını log/getter çağrılarından önce yazar. Getter sırası username→persona→userId; önceki atamalar sonraki hata nedeniyle geri alınmaz.0366f0 slot0 başarılı dönmeden object'i sıfırlamaz; sonra FreeLibrary sonucunu yok sayıp module'ü sıfırlar.035b60 wait1→load→offline isecleanup→finish sırasını kullanır.

Slot0'ın deleting-destructor veya release ayrımı, object boyutu/allocator, gerçek provider gövdeleri, string/pointer ömrü ve thread safety bilinmiyor. Yeni adapter bunları tahmin etmez;7foreign çağrı owned lower sözleşmesidir. GetProcAddress sonucu yalnız opaque integer token olarak döner, burada invoke/dereference edilmez.
