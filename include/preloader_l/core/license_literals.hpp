// SPDX-License-Identifier: GPL-3.0-only

#pragma once
// Static ASCII literals originate in the SHA-256-bound input; no user CFG values.
// The four config diagnostics use the owner-approved surge.cfg product name.
// Address comments identify original evidence, not rebuilt storage locations.
namespace preloader_l::license::literals {
inline constexpr char c_083870[] = "0"; // 0x180083870 / file 0x82070
inline constexpr char c_083960[] = "DISPLAY_NAME"; // 0x180083960 / file 0x82160
inline constexpr char c_083978[] = "INSTALLED_VERSION"; // 0x180083978 / file 0x82178
inline constexpr char c_0839a0[] = "License"; // 0x1800839a0 / file 0x821a0
inline constexpr char c_083a08[] = "dbdata.dll"; // 0x180083a08 / file 0x82208
inline constexpr char c_084c70[] = "ErrorSuccess"; // 0x180084c70 / file 0x83470
inline constexpr char c_084c80[] = "Code"; // 0x180084c80 / file 0x83480
inline constexpr char c_084c88[] = "Description"; // 0x180084c88 / file 0x83488
inline constexpr char c_0851d8[] = "|"; // 0x1800851d8 / file 0x839d8
inline constexpr char c_085570[] = "ClientId"; // 0x180085570 / file 0x83d70
inline constexpr char c_085580[] = "You need to be online to get an AuthCode."; // 0x180085580 / file 0x83d80
inline constexpr char c_0855b0[] = "-1577058293"; // 0x1800855b0 / file 0x83db0
inline constexpr char c_0855c0[] = "AuthCode"; // 0x1800855c0 / file 0x83dc0
inline constexpr char c_0855cc[] = "value"; // 0x1800855cc / file 0x83dcc
inline constexpr char c_085920[] = "\"DenuvoDllHash\" is missing or invalid. Make sure you have the right \"surge.cfg\" file."; // 0x180085920 / file 0x84120
inline constexpr char c_085978[] = "Bad dll hash"; // 0x180085978 / file 0x84178
inline constexpr char c_085990[] = "\"DenuvoDllName\" is missing or invalid. Make sure you have the right \"surge.cfg\" file."; // 0x180085990 / file 0x84190
inline constexpr char c_0859e8[] = "Bad dll name"; // 0x1800859e8 / file 0x841e8
inline constexpr char c_085a00[] = "\" file has incorrect hash.\n\nThat usually means you didn't update your game or you didn't update \"not a crack\"/\"fixed exe\"."; // 0x180085a00 / file 0x84200
inline constexpr char c_085a7c[] = "\""; // 0x180085a7c / file 0x8427c
inline constexpr char c_085a80[] = "RequestTicket"; // 0x180085a80 / file 0x84280
inline constexpr char c_085a90[] = "TicketEngine"; // 0x180085a90 / file 0x84290
inline constexpr char c_085aa0[] = "\"ContentId\" is missing or set to \"0\". Make sure you have the \"surge.cfg\" file in the same folder as the game exe."; // 0x180085aa0 / file 0x842a0
inline constexpr char c_085b18[] = "Bad content ID"; // 0x180085b18 / file 0x84318
inline constexpr char c_085b30[] = "\"DenuvoExeHash\" is missing or invalid. Make sure you have the right \"surge.cfg\" file."; // 0x180085b30 / file 0x84330
inline constexpr char c_085b88[] = "Bad exe hash"; // 0x180085b88 / file 0x84388
inline constexpr char c_085ba0[] = "\" file has incorrect hash.\n\nThat usually means you have the new \"not a crack\"/\"fixed exe\" but you opened the wrong exe."; // 0x180085ba0 / file 0x843a0
inline constexpr char c_085c18[] = " (v"; // 0x180085c18 / file 0x84418
inline constexpr char c_085c1c[] = "; \""; // 0x180085c1c / file 0x8441c
inline constexpr char c_085c20[] = "\") is protected by Denuvo.\n\n"; // 0x180085c20 / file 0x84420
inline constexpr char c_085c40[] = "exe:"; // 0x180085c40 / file 0x84440
inline constexpr char c_085c48[] = ";dll:"; // 0x180085c48 / file 0x84448
inline constexpr char c_085c50[] = ";"; // 0x180085c50 / file 0x84450
inline constexpr char c_085c54[] = "\n\n"; // 0x180085c54 / file 0x84454
inline constexpr char c_085c60[] = "Wine support is bad. Only some games work with this method. Most of them need a new token every time you start the game, so you will never be able to play them like that. This must be because of something Wine does that makes Denuvo think you're on a new machine with each launch.\n\nThat's why I recommend dual booting Windows or installing it in a virtual machine.\n\nDON'T send your ticket to my bot unless you're 100% sure this method works for this game."; // 0x180085c60 / file 0x84460
inline constexpr char c_085e28[] = "Wine detected"; // 0x180085e28 / file 0x84628
inline constexpr char c_085e38[] = "\n\nDENUVO TICKET ABOVE\n\n"; // 0x180085e38 / file 0x84638
inline constexpr char c_085e50[] = "\n\nDENUVO TICKET BELOW\n\n"; // 0x180085e50 / file 0x84650
inline constexpr char c_085e68[] = "Denuvo_ticket_"; // 0x180085e68 / file 0x84668
inline constexpr char c_085e78[] = ".txt"; // 0x180085e78 / file 0x84678
inline constexpr char c_085e80[] = "Your ticket has been saved to:\n"; // 0x180085e80 / file 0x84680
inline constexpr char c_085ea0[] = "Ticket saved"; // 0x180085ea0 / file 0x846a0
inline constexpr char c_085eb0[] = "At the moment of writing this message you can't copy dialogue text when running the game through Wine by pressing Ctrl+C.\n\nCopy your ticket from the terminal instead."; // 0x180085eb0 / file 0x846b0
inline constexpr char c_085f58[] = "Press Ctrl+C to copy this message.\n\n"; // 0x180085f58 / file 0x84758
inline constexpr char c_085f80[] = "Token required"; // 0x180085f80 / file 0x84780
inline constexpr char c_085f90[] = "RequestLicenseResponse"; // 0x180085f90 / file 0x84790
inline constexpr char c_085fa8[] = "License Deleted."; // 0x180085fa8 / file 0x847a8
}
