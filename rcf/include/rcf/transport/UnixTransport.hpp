#pragma once
#include "ITransport.hpp"
#include <string>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
//  UnixTransport / UnixTransportServer  —  declarations only.
//  All socket logic lives in UnixTransport.cpp.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

// ── Client-side (or accepted-fd) transport ────────────────────────────────────
class UnixTransport final : public ITransport {
public:
    // Wrap an already-accepted file descriptor (server side)
    explicit UnixTransport(int fd);

    // Connect to a Unix-domain socket path (client side)
    UnixTransport(const std::string& path, int timeout_ms = 3000);

    ~UnixTransport() override;

    UnixTransport(UnixTransport&& o) noexcept;
    UnixTransport(const UnixTransport&) = delete;
    UnixTransport& operator=(const UnixTransport&) = delete;

    void                 sendFrame(const std::vector<uint8_t>& frame) override;
    std::vector<uint8_t> recvFrame(int timeout_ms = -1) override;
    bool                 isConnected() const override;
    int                  fd() const override;
    void                 close() override;

private:
    int fd_;
};

// ── Server-side listener ──────────────────────────────────────────────────────
class UnixTransportServer final : public ITransportServer {
public:
    explicit UnixTransportServer(std::string path, int backlog = 16);
    ~UnixTransportServer() override;

    void                        listen() override;
    std::unique_ptr<ITransport> accept() override;
    void                        close() override;
    int                         fd() const;

private:
    std::string path_;
    int         fd_;
    int         backlog_;
};

} // namespace rcf
