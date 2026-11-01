#include "worker.hpp"
#include "../anadius-resources-factory-source/factory.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::initial_cells {
application_worker::Settings::StateSeed make_worker_defaults(){return {-1,{}};}
void select_worker_defaults(resources_factory::Inputs& resources){
    if(resources.worker.state)throw std::invalid_argument("Fresh Worker state required; existing avatar/path state cannot be replaced");
    if(!resources.domain||resources.domain->phase()!=thread_domain::Phase::Ready)
        throw std::invalid_argument("Worker initial state requires a Ready Domain");
    auto state=make_worker_defaults();
    static_assert(std::is_nothrow_move_constructible_v<application_worker::Settings::StateSeed>);
    resources.worker.state.emplace(std::move(state));
}
}
