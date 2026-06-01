#pragma once
#include "rcf/transport/UnixTransport.hpp"
#include "rcf/transport/TcpTransport.hpp"
#include "rcf/Types.hpp"

#include <string>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
//  ServiceClient<Req, Res>
//
//  Template declaration only — bodies are in ServiceClient.cpp.
//  Add explicit instantiations there for every Req/Res pair you ship.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

template<typename Req, typename Res>
class ServiceClient {
public:
    // Unix-socket client (same host)
    explicit ServiceClient(const std::string& path, int timeout_ms = 5000);

    // TCP client (remote host)
    ServiceClient(const std::string& host, uint16_t port, int timeout_ms = 5000);

    // Blocking call — opens connection, sends Req, waits for Res, closes.
    // Throws std::runtime_error on transport failure.
    Res call(const Req& req, int timeout_ms = -1);

    // Exception-free variant — returns ServiceStatus, writes result into `out`.
    ServiceStatus tryCall(const Req& req, Res& out, int timeout_ms = -1);

private:
    enum class Mode { UNIX, TCP };
    Mode        mode_;
    std::string path_;
    std::string host_;
    uint16_t    port_{0};
    int         timeout_ms_;
};

} // namespace rcf
