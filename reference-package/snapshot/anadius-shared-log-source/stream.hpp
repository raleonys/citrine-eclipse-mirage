#pragma once
#include <memory>
#include <mutex>
#include <ostream>
#include <streambuf>
#include <thread>

namespace anadius::shared_log {
namespace detail {
// First VIRTUAL base: basic_ostream itself has a virtual basic_ios base.
// A merely first nonvirtual owner would die before ios_base erase callbacks.
struct BufferOwner {
    explicit BufferOwner(std::shared_ptr<std::streambuf>);
    std::shared_ptr<std::streambuf> buffer;
};
}
// SOURCE compiler-ostream owner, not recovered MSVC ostream/CRT ABI.
// All selected writers must share THIS SAME object. Raw ostream/buffer access,
// other frontends sharing its buffer, rdbuf/tie changes (even inside a Lease),
// global locale changes, fiber migration, and streambuf/facet callbacks invoking
// or waiting for another writer are unsupported. Format/error state may be
// configured under a Lease; source adapters never replace buffer or tie.
class Stream final:private virtual detail::BufferOwner,public std::ostream,public std::enable_shared_from_this<Stream> {
public:
    explicit Stream(std::shared_ptr<std::streambuf>);
    Stream(const Stream&)=delete;Stream& operator=(const Stream&)=delete;
    Stream(Stream&&)=delete;Stream& operator=(Stream&&)=delete;
private:
    std::mutex gate_;
    friend class Lease;
};
std::shared_ptr<Stream> make_stream(std::shared_ptr<std::streambuf>);
// Short, same-thread SOURCE operation scope. Retains stream AND buffer. Plain
// legacy streams are unchanged outside managed operations. Every adapter scope
// nested inside a managed operation rejects before acquiring another gate or
// touching ostream state, including legacy adapters; tied/rebuffered streams
// reject. No gate may span conversion/wait/dialog/original/provider calls, except
// the necessary nonblocking streambuf/facet work inside a formatted insertion.
class Lease final {
public:
    explicit Lease(std::ostream&);
    ~Lease() noexcept;
    Lease(const Lease&)=delete;Lease& operator=(const Lease&)=delete;
    Lease(Lease&&)=delete;Lease& operator=(Lease&&)=delete;
    bool managed() const noexcept{return bool(stream_);}
private:
    std::shared_ptr<Stream> stream_;
    std::thread::id owner_;
};
Lease acquire(std::ostream&);
} // namespace anadius::shared_log
