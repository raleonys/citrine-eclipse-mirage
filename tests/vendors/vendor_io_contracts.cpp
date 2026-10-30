// Real vendor I/O fixtures. These do not choose application storage policy,
// transport scheduling, config grammar, bootstrap or a public ABI.
#include <asio.hpp>
#include <filesystem>
#include <fstream>
#include <array>
#include <chrono>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition,const char* message) {
    if(!condition) throw std::runtime_error(message);
}
struct TemporaryDirectory {
    std::filesystem::path path;
    TemporaryDirectory() {
        std::random_device random;
        const auto parent=std::filesystem::temp_directory_path();
        for(unsigned attempt=0;attempt<100;++attempt) {
            auto candidate=parent/("surge-vendor-fixture-"+std::to_string(random()));
            if(std::filesystem::create_directory(candidate)) {path=std::move(candidate);return;}
        }
        throw std::runtime_error("Cannot reserve isolated vendor fixture directory");
    }
    ~TemporaryDirectory() {std::error_code ignored;std::filesystem::remove_all(path,ignored);}
};
void filesystem_io() {
    namespace fs=std::filesystem;
    TemporaryDirectory temporary;
    const auto nested=temporary.path/fs::path(u8"alt-\u00e7-\u0131-\u4e2d");
    require(fs::create_directory(nested),"create Unicode directory");
    const auto input=nested/fs::path(u8"veri-\u015f.bin");
    const std::string payload("a\0b\r\n\xff",6);
    {
        std::ofstream output(input,std::ios::binary);
        require(output.is_open(),"open Unicode output");
        output.write(payload.data(),static_cast<std::streamsize>(payload.size()));
        output.close();require(!output.fail(),"write binary file");
    }
    require(fs::is_regular_file(input) && fs::file_size(input)==payload.size(),"file status/size");
    const auto renamed=nested/"renamed.bin";
    fs::rename(input,renamed);
    require(!fs::exists(input) && fs::exists(renamed),"rename file");
    {
        std::ifstream stream(renamed,std::ios::binary);
        const std::string bytes{std::istreambuf_iterator<char>(stream),std::istreambuf_iterator<char>()};
        require(bytes==payload,"binary file content preserved");
    }
    unsigned files=0;
    for(const auto& item:fs::directory_iterator(nested)) if(item.is_regular_file())++files;
    require(files==1,"directory iteration");
    require(fs::remove(renamed) && !fs::remove(renamed),"remove/missing remove");
    std::error_code missing;
    (void)fs::file_size(renamed,missing);
    require(bool(missing),"missing file must report error");
}
void socket_io() {
    using Tcp=asio::ip::tcp;
    asio::io_context context;
    // Kernel-assigned ephemeral port, loopback only; never a product endpoint.
    Tcp::acceptor listener(context,{asio::ip::address_v4::loopback(),0});
    Tcp::socket client(context),server(context);
    const std::string payload("request\0binary",14);
    std::array<char,14> received{},response{};
    bool accepted=false,connected=false,read=false,written=false,replied=false,echoed=false,eof=false;
    asio::steady_timer deadline(context,std::chrono::seconds(5));
    bool timed_out=false;
    deadline.async_wait([&](const asio::error_code& error) {
        if(!error) {timed_out=true;context.stop();}
    });
    listener.async_accept(server,[&](const asio::error_code& error) {
        require(!error,"async accept");accepted=true;
        asio::async_read(server,asio::buffer(received),[&](const asio::error_code& error,std::size_t size) {
            require(!error && size==payload.size(),"async receive length");
            require(std::string(received.data(),size)==payload,"async receive bytes");read=true;
            asio::async_write(server,asio::buffer(received),[&](const asio::error_code& error,std::size_t size) {
                require(!error && size==payload.size(),"async reply");replied=true;
                server.shutdown(Tcp::socket::shutdown_send);
            });
        });
    });
    client.async_connect(listener.local_endpoint(),[&](const asio::error_code& error) {
        require(!error,"async connect");connected=true;
        asio::async_write(client,asio::buffer(payload),[&](const asio::error_code& error,std::size_t size) {
            require(!error && size==payload.size(),"async send");written=true;
        });
        asio::async_read(client,asio::buffer(response),[&](const asio::error_code& error,std::size_t size) {
            require(!error && size==payload.size(),"async reply receive");
            require(std::string(response.data(),size)==payload,"reply content");echoed=true;
            client.async_read_some(asio::buffer(response),[&](const asio::error_code& error,std::size_t size) {
                require(error==asio::error::eof && size==0,"EOF is an error with zero bytes");
                eof=true;deadline.cancel();
            });
        });
    });
    context.run();
    require(!timed_out && accepted && connected && read && written && replied && echoed && eof,
            "all asynchronous socket paths must finish");
    // Cancel a real pending receive and verify the cancellation completion.
    context.restart();
    Tcp::socket pending(context);
    Tcp::acceptor cancellation_listener(context,{asio::ip::address_v4::loopback(),0});
    Tcp::socket peer(context);
    peer.connect(cancellation_listener.local_endpoint());cancellation_listener.accept(pending);
    bool cancelled=false;
    pending.async_read_some(asio::buffer(received),[&](const asio::error_code& error,std::size_t size) {
        require(error==asio::error::operation_aborted && size==0,"receive cancellation");cancelled=true;
    });
    pending.cancel();context.run();require(cancelled,"cancel handler executed");
}
}
int main() {
    try {
        filesystem_io();socket_io();
        std::cout<<"PASS: standard Unicode/binary filesystem I/O; TCP loopback async accept/connect/read/write/EOF/cancel (fixture only)\n";
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
