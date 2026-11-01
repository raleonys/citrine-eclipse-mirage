#pragma once
#include "../anadius-file-source/binary_io.hpp"

namespace anadius::application_io {
// Synchronous guarded file factory, shared by application effect providers.
// Parent checkpoint/context and lower factory outlive every wrapper. One
// stream at a time; raw owners are adopted before callback postchecks.
class GuardedFiles final:public file_recovered::Factory {
public:
    struct Checkpoint {void* context;void (*check)(void*);};
    GuardedFiles(file_recovered::Factory&,Checkpoint);
    ~GuardedFiles()noexcept override;
    GuardedFiles(const GuardedFiles&)=delete;GuardedFiles& operator=(const GuardedFiles&)=delete;
    bool has_pending()const noexcept;
    void ensure_known()const;
    std::unique_ptr<file_recovered::Input> open_input_039f70(const configuration::Path&,std::uint32_t,std::uint32_t)override;
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const configuration::Path&,std::uint32_t,std::uint32_t)override;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
}
