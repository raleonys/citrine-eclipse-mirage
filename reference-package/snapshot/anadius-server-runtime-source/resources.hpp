#pragma once
#include "runtime.hpp"
#include "../anadius-bootstrap-backend-source/backend.hpp"

namespace anadius::server_runtime {
// Assemble the server component only. Other bootstrap Resources remain explicit
// caller providers. No worker, callback, socket or DLL starts in this factory.
std::unique_ptr<o::OwnedServerBackend> make_server(std::shared_ptr<thread_domain::DomainStorage>,
    o::Dependencies,Dependencies,std::size_t io_capacity=64);
// Worker assembly from an explicit dedicated controller-owned memory borrow.
std::unique_ptr<o::OwnedServerBackend> make_server_prepared(std::shared_ptr<thread_domain::DomainStorage>,
    worker_handoff::PreparedMemory::Borrow&&,o::Dependencies,Dependencies,std::size_t io_capacity=64);
void install_server(bootstrap_backend::Resources&,std::shared_ptr<thread_domain::DomainStorage>,
    o::Dependencies,Dependencies,std::size_t io_capacity=64);
} // namespace anadius::server_runtime
