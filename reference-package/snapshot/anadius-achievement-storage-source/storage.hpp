#pragma once
#include "../anadius-achievement-writer-source/writer.hpp"
#include "../anadius-messages-source/messages.hpp"
#include "../anadius-status-source/status.hpp"
#include <optional>

namespace anadius::achievement_storage {
struct Paths {configuration::Path xml_path,ini_path;}; // Caller-owned global path cells.
class Numbers {
public:
    virtual ~Numbers()=default;
    virtual bool is_space(unsigned char)=0;
    virtual std::optional<std::uint32_t> scan_decimal(std::string_view)=0; // %d
    virtual std::optional<std::uint32_t> scan_hex(std::string_view)=0;     // %x
};
// Explicit host-C-runtime compatibility choice, not recovered original CRT/ABI.
// Overflow/locale/malformed byte equality is not established.
class HostCrtNumbers final:public Numbers {
public:
    bool is_space(unsigned char) override;
    std::optional<std::uint32_t> scan_decimal(std::string_view) override;
    std::optional<std::uint32_t> scan_hex(std::string_view) override;
};
std::uint32_t attribute_003b70(const messages_recovered::XmlNode&,std::string_view key,
                            std::uint32_t fallback,Numbers&);
class Effects:public achievement_writer::Effects {
public:
    virtual configuration::Path filesystem_widen_006d70(std::string_view)=0;
    virtual std::int64_t timestamp_to_epoch_041e10(std::string_view)=0;
    virtual status_recovered::Status status_01ea20(const configuration::Path&,std::uint32_t flags)=0;
    [[noreturn]] virtual void filesystem_error_018130(std::string_view operation,
        std::uint32_t error,const configuration::Path&)=0;
};
class Storage {
public:
    Storage(Paths& paths,messages_recovered::XmlBackend& xml,Numbers& numbers,
            Effects& effects,file_recovered::Factory& files)
        :paths_(paths),xml_(xml),numbers_(numbers),effects_(effects),files_(files){}
    void update_01eb00(configuration::State&);
private:
    Paths& paths_;
    messages_recovered::XmlBackend& xml_;
    Numbers& numbers_;
    Effects& effects_;
    file_recovered::Factory& files_;
};
// Implements only the persistence method for the existing startup dependency.
// All remaining configuration OS/path/log/license methods remain abstract.
class ConfigurationDependencies:public configuration::Dependencies {
public:
    explicit ConfigurationDependencies(Storage& storage):storage_(storage){}
    void update_achievement_storage_01eb00(configuration::State& state) final {storage_.update_01eb00(state);}
private:
    Storage& storage_;
};
} // namespace anadius::achievement_storage
