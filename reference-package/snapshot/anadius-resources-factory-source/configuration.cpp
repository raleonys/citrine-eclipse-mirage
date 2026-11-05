#include "configuration.hpp"
#include <stdexcept>

namespace anadius::resources_factory {
void install_native_configuration_console(bootstrap_backend::Resources& r,std::shared_ptr<shared_log::Stream> stream){
    const DWORD error=::GetLastError();
    struct Error {DWORD value;~Error(){::SetLastError(value);}} preserve{error};
    if(!stream||r.configuration||r.console)
        throw std::invalid_argument("Managed image stream and vacant configuration/console slots required");
    ConfigurationInputs in;
    in.stream=std::move(stream);
    in.documents=configuration_factory::make_scalar_documents();
    install_configuration_console(r,in);
}
void install_configuration_console(bootstrap_backend::Resources& r,ConfigurationInputs& in){
    if(in.consumed_)throw std::logic_error("Configuration/console inputs already consumed");
    if(r.configuration||r.console)throw std::invalid_argument("Configuration and console target slots must be empty");
    const auto& c=in.configuration;const auto& p=c.persistence;const auto& a=c.crt;const auto& w=c.platform;const auto& k=in.console;
    if(!in.stream||!in.documents||(c.stream&&c.stream.get()!=static_cast<std::ostream*>(in.stream.get())))
        throw std::invalid_argument("Managed shared stream and explicit owned legacy reader required");
    if(!c.filesystem||!c.encoding||!c.errors||!a.space||!a.lower||!a.environment||!w.module_filename||!w.environment||!w.last_error||!w.set_last_error||
       !p.files||!p.xml||!p.numbers||!p.parser||!p.stats||!p.encoding||!p.errors)
        throw std::invalid_argument("Complete owned configuration/persistence/SDK/CRT providers required");
    if(!k.attach||!k.allocate||!k.window||!k.move||!k.reopen||!in.streams[0]||!in.streams[1]||!in.streams[2])
        throw std::invalid_argument("Complete compiler-console API and three compiler stream identities required");
    in.consumed_=true;in.configuration.stream=in.stream;
    auto configuration=configuration_factory::make_configuration(std::move(in.configuration),{},std::move(in.documents));
    auto console=std::make_unique<bootstrap_providers::CompilerConsole>(in.console,in.streams);
    // No paths() borrowing here: observed Runtime must bind the real empty root.
    r.configuration=std::move(configuration);r.console=std::move(console);
    in.stream.reset();
}
}
