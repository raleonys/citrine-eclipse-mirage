#include "resources.hpp"

namespace anadius::server_runtime {
std::unique_ptr<o::OwnedServerBackend> make_server(std::shared_ptr<thread_domain::DomainStorage> domain,
    o::Dependencies server,Dependencies runtime,std::size_t io_capacity){
    if(server.runtime||!server.errors||server.errors!=runtime.errors||server.sockets!=runtime.sockets||
       server.dispatcher!=runtime.dispatcher||runtime.capacity!=io_capacity)
        throw std::invalid_argument("Server/runtime require identical errors, sockets, dispatcher, capacity and no prebound runtime");
    server.runtime=std::make_shared<Runtime>(std::move(runtime));
    return std::make_unique<o::OwnedServerBackend>(std::move(domain),std::move(server),io_capacity);
}
void install_server(bootstrap_backend::Resources& resources,std::shared_ptr<thread_domain::DomainStorage> domain,
    o::Dependencies server,Dependencies runtime,std::size_t io_capacity){
    if(resources.server)throw std::logic_error("Resources already owns a server");
    resources.server=make_server(std::move(domain),std::move(server),std::move(runtime),io_capacity);
}
std::unique_ptr<o::OwnedServerBackend> make_server_prepared(std::shared_ptr<thread_domain::DomainStorage> domain,
    worker_handoff::PreparedMemory::Borrow&& borrow,o::Dependencies server,Dependencies runtime,std::size_t io_capacity){
    if(!domain||borrow.domain_identity()!=domain.get()||server.runtime||!server.errors||server.errors!=runtime.errors||server.sockets!=runtime.sockets||
       server.dispatcher!=runtime.dispatcher||runtime.capacity!=io_capacity)
        throw std::invalid_argument("Prepared server/runtime require matching Domain, memory and lower providers");
    (void)borrow.memory();
    server.runtime=std::make_shared<Runtime>(std::move(runtime));
    return std::make_unique<o::OwnedServerBackend>(std::move(domain),std::move(borrow),std::move(server),io_capacity);
}
} // namespace anadius::server_runtime
