# Mac sembollerinin C++ kaynak kurtarımına katkısı

ZIP üyesi yalnız bellekte okundu ve kimliği önceki REA arşiv envanteriyle eşleştirildi. Dylib dosyası çıkarılmadı, yüklenmedi veya çalıştırılmadı.

C++ adları LLVM llvm-cxxfilt 23.1.3 ile açıldı. Bu adlar Mac dilimlerine aittir; Windows adres eşlemesi veya aynı derleme/sürüm olduğu varsayılmaz. Nlist kod sembolleri de orijinal kaynak fonksiyon sayısı sayılmaz.

## CPU 0x1000007

7271 kod sembolü adayı; 7483 C++ mangled tanım; doğrudan LSXEmu:: ile başlayan 44 uygulama kod sembolü.

| Mac adresi | C++ adı |
|---|---|
| 0xb2a0 | LSXEmu::Types::emu_config::emu_config() |
| 0xb2d0 | LSXEmu::Types::emu_config::~emu_config() |
| 0xb320 | LSXEmu::Main::set_up_console() |
| 0xb330 | LSXEmu::Main::wait_until_initialized() |
| 0xb390 | LSXEmu::Types::emu_config::emu_config() |
| 0x104b0 | LSXEmu::Types::entitlement::~entitlement() |
| 0x104d0 | LSXEmu::Types::entitlement::~entitlement() |
| 0x108f0 | LSXEmu::Types::content::~content() |
| 0x10910 | LSXEmu::Types::content::~content() |
| 0x10a00 | LSXEmu::Types::emu_config::~emu_config() |
| 0x11660 | LSXEmu::Types::achievement::~achievement() |
| 0x11680 | LSXEmu::Types::achievement::~achievement() |
| 0x11880 | LSXEmu::Hooks::install() |
| 0x11890 | LSXEmu::Hooks::uninstall() |
| 0x118b0 | LSXEmu::Main::init() |
| 0x11a70 | LSXEmu::Main::end() |
| 0x13af0 | LSXEmu::Config::parse_args() |
| 0x13fd0 | LSXEmu::Config::parse_config() |
| 0x30870 | LSXEmu::Types::content::content(LSXEmu::Types::content&&) |
| 0x308a0 | LSXEmu::Types::content::content(LSXEmu::Types::content&&) |
| 0x31700 | LSXEmu::Types::entitlement::entitlement(LSXEmu::Types::entitlement&&) |
| 0x31730 | LSXEmu::Types::entitlement::entitlement(LSXEmu::Types::entitlement&&) |
| 0x32330 | LSXEmu::Types::achievement::operator=(LSXEmu::Types::achievement&&) |
| 0x33190 | LSXEmu::Types::achievement::achievement() |
| 0x331b0 | LSXEmu::Types::achievement::achievement() |
| 0x38500 | LSXEmu::Achievements::save_steam() |
| 0x38bc0 | LSXEmu::Achievements::prepare() |
| 0x3afb0 | LSXEmu::Server::start() |
| 0x5bda0 | LSXEmu::License::get_path(bool) |
| 0x5be70 | LSXEmu::License::load_token(ghc::filesystem::path const&) |
| 0x5c150 | LSXEmu::License::create(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>) |
| 0x5cbb0 | LSXEmu::Messages::Handler::Handler() |
| 0x5cbe0 | LSXEmu::Messages::Handler::Handler() |
| 0x5cc00 | LSXEmu::Messages::Handler::initial_message() |
| 0x5ce90 | LSXEmu::Messages::Handler::process_message(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>&) |
| 0x6de80 | LSXEmu::Random::create_key() |
| 0x6dfb0 | LSXEmu::Random::cipher_key_from_response(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>&) |
| 0x6f700 | LSXEmu::Libraries::load(bool) |
| 0x6fef0 | LSXEmu::Libraries::unload() |
| 0x723c0 | LSXEmu::Online::load() |
| 0x72bd0 | LSXEmu::Online::unload() |
| 0x73890 | LSXEmu::Execution::set_finished(bool&) |
| 0x73910 | LSXEmu::Execution::steps_finished_initializing(int) |
| 0x739f0 | LSXEmu::Execution::wait_for_steps(int) |

Modül başlatıcıları: __GLOBAL__sub_I_main.cpp, __GLOBAL__sub_I_server.cpp

## CPU 0x100000c

7272 kod sembolü adayı; 7476 C++ mangled tanım; doğrudan LSXEmu:: ile başlayan 44 uygulama kod sembolü.

| Mac adresi | C++ adı |
|---|---|
| 0xb39c | LSXEmu::Types::emu_config::emu_config() |
| 0xb3c8 | LSXEmu::Types::emu_config::~emu_config() |
| 0xb428 | LSXEmu::Main::set_up_console() |
| 0xb42c | LSXEmu::Main::wait_until_initialized() |
| 0xb444 | LSXEmu::Types::emu_config::emu_config() |
| 0x10d9c | LSXEmu::Types::entitlement::~entitlement() |
| 0x10dd0 | LSXEmu::Types::entitlement::~entitlement() |
| 0x11288 | LSXEmu::Types::content::~content() |
| 0x112bc | LSXEmu::Types::content::~content() |
| 0x113e0 | LSXEmu::Types::emu_config::~emu_config() |
| 0x12138 | LSXEmu::Types::achievement::~achievement() |
| 0x1216c | LSXEmu::Types::achievement::~achievement() |
| 0x12334 | LSXEmu::Hooks::install() |
| 0x12338 | LSXEmu::Hooks::uninstall() |
| 0x12340 | LSXEmu::Main::init() |
| 0x12504 | LSXEmu::Main::end() |
| 0x14ac0 | LSXEmu::Config::parse_args() |
| 0x1504c | LSXEmu::Config::parse_config() |
| 0x3350c | LSXEmu::Types::content::content(LSXEmu::Types::content&&) |
| 0x33548 | LSXEmu::Types::content::content(LSXEmu::Types::content&&) |
| 0x34588 | LSXEmu::Types::entitlement::entitlement(LSXEmu::Types::entitlement&&) |
| 0x345c4 | LSXEmu::Types::entitlement::entitlement(LSXEmu::Types::entitlement&&) |
| 0x3531c | LSXEmu::Types::achievement::operator=(LSXEmu::Types::achievement&&) |
| 0x36320 | LSXEmu::Types::achievement::achievement() |
| 0x36354 | LSXEmu::Types::achievement::achievement() |
| 0x3c074 | LSXEmu::Achievements::save_steam() |
| 0x3c6e4 | LSXEmu::Achievements::prepare() |
| 0x3edf8 | LSXEmu::Server::start() |
| 0x63140 | LSXEmu::License::get_path(bool) |
| 0x63234 | LSXEmu::License::load_token(ghc::filesystem::path const&) |
| 0x634f0 | LSXEmu::License::create(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>) |
| 0x64010 | LSXEmu::Messages::Handler::Handler() |
| 0x6404c | LSXEmu::Messages::Handler::Handler() |
| 0x64080 | LSXEmu::Messages::Handler::initial_message() |
| 0x64318 | LSXEmu::Messages::Handler::process_message(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>&) |
| 0x76110 | LSXEmu::Random::create_key() |
| 0x7627c | LSXEmu::Random::cipher_key_from_response(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>&) |
| 0x77d2c | LSXEmu::Libraries::load(bool) |
| 0x784f8 | LSXEmu::Libraries::unload() |
| 0x7ade4 | LSXEmu::Online::load() |
| 0x7b590 | LSXEmu::Online::unload() |
| 0x7c43c | LSXEmu::Execution::set_finished(bool&) |
| 0x7c4f4 | LSXEmu::Execution::steps_finished_initializing(int) |
| 0x7c618 | LSXEmu::Execution::wait_for_steps(int) |

Modül başlatıcıları: __GLOBAL__sub_I_main.cpp, __GLOBAL__sub_I_server.cpp

Tam sembol kaydı [symbol-inventory.json](symbol-inventory.json), uygulama adları [application-symbols.json](application-symbols.json) içindedir.