#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace rcf {

// Per-connection transport: send / receive one framed message at a time.
class ITransport {
public:
    virtual ~ITransport()                                             = default;
    virtual void                 sendFrame(const std::vector<uint8_t>&) = 0;
    virtual std::vector<uint8_t> recvFrame(int timeout_ms = -1)         = 0;
    virtual bool                 isConnected() const                     = 0;
    virtual void                 close()                                 = 0;
    virtual int                  fd() const { return -1; }
};

// Server-side listener: blocks on accept() and returns one ITransport per client.
class ITransportServer {
public:
    virtual ~ITransportServer()                          = default;
    virtual void                        listen()         = 0;
    virtual std::unique_ptr<ITransport> accept()         = 0;
    virtual void                        close()          = 0;
};

} // namespace rcf
