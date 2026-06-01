#pragma once
#include "ITransport.hpp"
#include <string>
#include <memory>
#include <cstdint>

namespace rcf {

// ── Client-side transport ────────────────────────────────────
class TcpTransport final : public ITransport {
public:
    // Wrap an already-accepted file descriptor (server side)
    explicit TcpTransport(int fd);

    // Connect to host:port (client side)
    TcpTransport(const std::string& host, uint16_t port, int timeout_ms = 3000);

    ~TcpTransport() override;

    TcpTransport(TcpTransport&& o) noexcept;
    TcpTransport(const TcpTransport&) = delete;
    TcpTransport& operator=(const TcpTransport&) = delete;

    void                 sendFrame(const std::vector<uint8_t>& frame) override;
    std::vector<uint8_t> recvFrame(int timeout_ms = -1) override;
    bool                 isConnected() const override;
    int                  fd() const override;
    void                 close() override;

private:
    int fd_;

    void applyKeepAlive();
    void setNoDelay(bool on);
};

// ── Server-side listener ──────────────────────────────────────────────────────
class TcpTransportServer final : public ITransportServer {
public:
    explicit TcpTransportServer(uint16_t port, int backlog = 16);
    ~TcpTransportServer() override;

    void                        listen() override;
    std::unique_ptr<ITransport> accept() override;
    void                        close() override;
    int                         fd() const;

private:
    uint16_t port_;
    int      fd_;
    int      backlog_;
};

} // namespace rcf
