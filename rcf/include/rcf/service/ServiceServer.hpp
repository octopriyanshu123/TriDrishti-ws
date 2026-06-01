#pragma once
#include "rcf/transport/ITransport.hpp"
#include "rcf/transport/UnixTransport.hpp"
#include "rcf/transport/TcpTransport.hpp"
#include "rcf/Types.hpp"
 
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <cstdint>

namespace rcf {

template<typename Req, typename Res>
class ServiceServer {
public:
    using Handler = std::function<Res(const Req&)>;

    explicit ServiceServer(std::unique_ptr<ITransportServer> srv);

    // Convenience factories 
    static std::unique_ptr<ServiceServer<Req, Res>> makeUnix(const std::string& path);
    static std::unique_ptr<ServiceServer<Req, Res>> makeTcp(uint16_t port);

    ~ServiceServer();

    // Register the request handler.  Must be called before spin/spinAsync.
    void bind(Handler h);

    void spin();

    void spinAsync();
    void shutdown();

private:
    void handle(std::unique_ptr<ITransport> client);

    std::unique_ptr<ITransportServer> srv_;
    Handler                           handler_;
    std::atomic<bool>                 running_;
    std::thread                       spin_thread_;
};

} // namespace rcf
