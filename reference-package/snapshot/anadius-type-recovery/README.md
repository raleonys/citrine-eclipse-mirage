# anadius64.dll: RTTI ve sanal çağrı eşlemesi

Bu kayıt orijinal PE baytlarının salt okunur ayrıştırmasıdır; özgün C++ sınıf tanımları veya derlenebilir kaynak değildir.

SHA-256: 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c

88 type descriptor; 76 complete-object locator; 76 vftable adayı; 424 sanal slot başvurusu.

Vftable bitişi ardışık kod işaretçilerinden tahmin edilmiştir. Yer değiştirmeler tam sınıf boyutu/member yerleşimi sayılmamalıdır. Her yapı ve slot için kaynak dosya ofseti JSON içinde korunur.

| RTTI adı | COL adresi | Vftable adresi | Kod slotu |
|---|---|---|---:|
| .?AVXMLDocument@tinyxml2@@ | 0x1800920a8 | 0x18007fec8 | 17 |
| .?AVXMLElement@tinyxml2@@ | 0x180092178 | 0x18007fd78 | 17 |
| .?AVXMLAttribute@tinyxml2@@ | 0x1800921f8 | 0x18007fd68 | 1 |
| .?AVXMLComment@tinyxml2@@ | 0x180092270 | 0x18007fbb8 | 17 |
| .?AVXMLText@tinyxml2@@ | 0x1800922f0 | 0x18007fb28 | 17 |
| .?AVXMLDeclaration@tinyxml2@@ | 0x180092370 | 0x18007fc48 | 17 |
| .?AVXMLUnknown@tinyxml2@@ | 0x1800923f0 | 0x18007fcd8 | 17 |
| .?AVXMLPrinter@tinyxml2@@ | 0x180092470 | 0x18007ff58 | 15 |
| .?AVMemPool@tinyxml2@@ | 0x180092540 | 0x18007fa18 | 5 |
| .?AVXMLVisitor@tinyxml2@@ | 0x1800925b8 | 0x18007fa48 | 9 |
| .?AVXMLNode@tinyxml2@@ | 0x1800925e0 | 0x18007fa98 | 17 |
| .?AV?$MemPoolT@$0HI@@tinyxml2@@ | 0x180092608 | 0x18007fe08 | 5 |
| .?AV?$MemPoolT@$0FA@@tinyxml2@@ | 0x180092688 | 0x18007fe38 | 5 |
| .?AV?$MemPoolT@$0HA@@tinyxml2@@ | 0x180092708 | 0x18007fe68 | 5 |
| .?AV?$MemPoolT@$0GI@@tinyxml2@@ | 0x180092788 | 0x18007fe98 | 5 |
| .?AVexception@std@@ | 0x180092808 | 0x180080138 | 2 |
| .?AVbad_alloc@std@@ | 0x180092880 | 0x180080168 | 2 |
| .?AVbad_array_new_length@std@@ | 0x180092900 | 0x180080180 | 2 |
| .?AVruntime_error@std@@ | 0x180092988 | 0x1800801c0 | 2 |
| .?AV_System_error@std@@ | 0x180092a58 | 0x1800801d8 | 2 |
| .?AVsystem_error@std@@ | 0x180092ae0 | 0x1800801f0 | 2 |
| .?AV_Generic_error_category@std@@ | 0x180092b70 | 0x180080208 | 6 |
| .?AV_Iostream_error_category2@std@@ | 0x180092bf0 | 0x180080248 | 6 |
| .?AV_System_error_category@std@@ | 0x180092c70 | 0x1800802a8 | 6 |
| .?AVbad_cast@std@@ | 0x180092cf0 | 0x1800802f8 | 2 |
| .?AV_Facet_base@std@@ | 0x180092d70 | 0x180080320 | 3 |
| .?AV?$ctype@D@std@@ | 0x180092f28 | 0x180080350 | 11 |
| .?AVfailure@ios_base@std@@ | 0x180092fc0 | 0x1800803b0 | 2 |
| .?AV?$num_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@ | 0x180093058 | 0x1800807f8 | 11 |
| .?AV?$numpunct@D@std@@ | 0x1800930e8 | 0x180080860 | 8 |
| .?AVios_base@std@@ | 0x180093178 | 0x180083648 | 1 |
| .?AV?$basic_ios@DU?$char_traits@D@std@@@std@@ | 0x180093270 | 0x180083690 | 1 |
| .?AV?$basic_streambuf@DU?$char_traits@D@std@@@std@@ | 0x1800932f8 | 0x1800836b8 | 15 |
| .?AV?$basic_ostream@DU?$char_traits@D@std@@@std@@ | 0x1800933e8 | 0x1800836a0 | 1 |
| .?AV?$basic_stringbuf@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ | 0x180093478 | 0x180083738 | 15 |
| .?AV?$basic_ostringstream@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ | 0x1800934f8 | 0x1800837b8 | 1 |
| .?AVparse_error@pegtl@tao@@ | 0x1800935e0 | 0x1800837d0 | 2 |
| .?AVfilesystem_error@filesystem@std@@ | 0x180093668 | 0x1800837e8 | 2 |
| .?AV?$_Ref_count_obj2@Vparse_error@internal@pegtl@tao@@@std@@ | 0x180093700 | 0x180083da0 | 4 |
| .?AV?$_Ref_count_obj2@U_Dir_enum_impl@filesystem@std@@@std@@ | 0x180093780 | 0x180083de8 | 4 |
| .?AVsystem_category@detail@asio@@ | 0x180093800 | 0x180084248 | 6 |
| .?AVmisc_category@detail@error@asio@@ | 0x180093880 | 0x1800842a8 | 6 |
| .?AVlogic_error@std@@ | 0x180093900 | 0x180084380 | 2 |
| .?AVlength_error@std@@ | 0x180093980 | 0x180084398 | 2 |
| .?AVbad_weak_ptr@std@@ | 0x180093a08 | 0x1800843b0 | 2 |
| .?AVmultiple_exceptions@asio@@ | 0x180093a88 | 0x1800843d8 | 2 |
| .?AVbad_executor@execution@asio@@ | 0x180093b08 | 0x180084408 | 2 |
| .?AVservice@execution_context@asio@@ | 0x180093cd0 | 0x180084438 | 3 |
| .?AVservice_already_exists@asio@@ | 0x180093cf8 | 0x180084458 | 2 |
| .?AVinvalid_service_owner@asio@@ | 0x180093d80 | 0x180084470 | 2 |
| .?AVfunc_base@win_thread@detail@asio@@ | 0x180093e08 | 0x1800844b8 | 2 |
| .?AVwin_iocp_io_context@detail@asio@@ | 0x180093e80 | 0x180084510 | 3 |
| .?AVbad_address_cast@ip@asio@@ | 0x180093f78 | 0x180084550 | 2 |
| .?AV?$win_iocp_socket_service@Vtcp@ip@asio@@@detail@asio@@ | 0x180094000 | 0x1800845b0 | 3 |
| .?AV?$execution_context_service_base@V?$win_iocp_socket_service@Vtcp@ip@asio@@@detail@asio@@@detail@asio@@ | 0x180094170 | 0x180084590 | 3 |
| .?AV?$func@Uthread_function@win_iocp_io_context@detail@asio@@@win_thread@detail@asio@@ | 0x180094198 | 0x180084710 | 2 |
| .?AV?$_Ref_count_obj2@Vsession@?A0x8827afb8@@@std@@ | 0x180094218 | 0x180084750 | 25 |
| .?AV?$_Ref_count_resource@PEAXUnoop_deleter@socket_ops@detail@asio@@@std@@ | 0x180094298 | 0x180084820 | 4 |
| .?AVNoResponseRequiredError@?A0x911f7926@@ | 0x180094318 | 0x180084be8 | 2 |
| .?AVUnknownMessageError@?A0x911f7926@@ | 0x180094398 | 0x180084c00 | 2 |
| .?AV?$basic_istream@DU?$char_traits@D@std@@@std@@ | 0x180094418 | 0x1800867e0 | 1 |
| .?AV?$basic_filebuf@DU?$char_traits@D@std@@@std@@ | 0x1800944a8 | 0x180086748 | 15 |
| .?AV?$basic_ifstream@DU?$char_traits@D@std@@@std@@ | 0x180094528 | 0x1800867f0 | 1 |
| .?AV?$basic_ofstream@DU?$char_traits@D@std@@@std@@ | 0x1800945c0 | 0x1800867c8 | 1 |
| .?AV?$codecvt@DDU_Mbstatet@@@std@@ | 0x1800946c0 | 0x180086808 | 10 |
| .?AV?$basic_istringstream@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ | 0x180094758 | 0x1800869b0 | 1 |
| .?AV?$time_put@DV?$ostreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@ | 0x180094858 | 0x1800869c8 | 4 |
| .?AV?$time_get@DV?$istreambuf_iterator@DU?$char_traits@D@std@@@std@@@std@@ | 0x1800948e8 | 0x1800869f0 | 10 |
| .?AVinvalid_argument@std@@ | 0x180094980 | 0x180086d08 | 2 |
| .?AVout_of_range@std@@ | 0x180094a08 | 0x180086d20 | 2 |
| .?AV_Locimp@locale@std@@ | 0x180094a90 | 0x180087cc0 | 3 |
| .?AVbad_exception@std@@ | 0x180094b20 | 0x180087d20 | 2 |
| .?AV_ExceptionPtr_normal@?A0x6e02efe5@@ | 0x180094ba0 | 0x180087d48 | 4 |
| .?AV?$_ExceptionPtr_static@Vbad_alloc@std@@@?A0x6e02efe5@@ | 0x180094c20 | 0x180087d70 | 4 |
| .?AV?$_ExceptionPtr_static@Vbad_exception@std@@@?A0x6e02efe5@@ | 0x180094ca0 | 0x180087d98 | 4 |
| .?AVtype_info@@ | 0x180094d20 | 0x180088038 | 1 |

Ayrıntılı kalıtım grafiği, alt nesne ofsetleri ve slot-fonksiyon eşlemeleri: [msvc-rtti.json](msvc-rtti.json).

Ayrıştırma düzeni için birincil kaynak: [LLVM MicrosoftCXXABI.cpp](https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/MicrosoftCXXABI.cpp).