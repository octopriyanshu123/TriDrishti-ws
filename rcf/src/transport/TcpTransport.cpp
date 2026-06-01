#include "rcf/transport/TcpTransport.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>

namespace rcf {

// ─── Helpers (file-local) ─────────────────────────────────────────────────────
static std::string tcpSysErr() { return std::string(strerror(errno)); }

static void tcpSetNonBlock(int fd, bool on) {
    int f = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, on ? f | O_NONBLOCK : f & ~O_NONBLOCK);
}

static void tcpWriteAll(int fd, const uint8_t* d, size_t n) {
    size_t s = 0;
    while (s < n) {
        ssize_t r = ::send(fd, d + s, n - s, MSG_NOSIGNAL);
        if (r <= 0) throw std::runtime_error("send: " + tcpSysErr());
        s += r;
    }
}

static void tcpReadAll(int fd, uint8_t* buf, size_t n, int tms) {
    size_t g = 0;
    while (g < n) {
        if (tms >= 0) {
            pollfd p{ fd, POLLIN, 0 };
            int r = ::poll(&p, 1, tms);
            if (r == 0)  throw std::runtime_error("recv timeout");
            if (r < 0)   throw std::runtime_error("poll: " + tcpSysErr());
            if ((p.revents & POLLERR) && !(p.revents & POLLIN))
                throw std::runtime_error("socket error");
            if ((p.revents & POLLHUP) && !(p.revents & POLLIN))
                throw std::runtime_error("peer closed");
        }
        ssize_t r = ::recv(fd, buf + g, n - g, 0);
        if (r == 0) throw std::runtime_error("connection closed");
        if (r < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("recv: " + tcpSysErr());
        }
        g += r;
    }
}

// ─── TcpTransport ─────────────────────────────────────────────────────────────
TcpTransport::TcpTransport(int fd) : fd_(fd) { applyKeepAlive(); }

TcpTransport::TcpTransport(const std::string& host, uint16_t port, int timeout_ms)
    : fd_(-1)
{
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) throw std::runtime_error("socket: " + tcpSysErr());

    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port   = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &a.sin_addr) <= 0)
        throw std::runtime_error("Bad IP: " + host);

    tcpSetNonBlock(fd_, true);

    int rc = ::connect(fd_, reinterpret_cast<sockaddr*>(&a), sizeof(a));
    if (rc < 0 && errno != EINPROGRESS)
        throw std::runtime_error("connect(" + host + ":" + std::to_string(port) + "): " + tcpSysErr());

    if (rc != 0) {
        pollfd p{ fd_, POLLOUT, 0 };
        if (::poll(&p, 1, timeout_ms) <= 0)
            throw std::runtime_error("connect timeout " + host + ":" + std::to_string(port));
        int err = 0;
        socklen_t el = sizeof(err);
        ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &el);
        if (err) throw std::runtime_error("connect err: " + std::string(strerror(err)));
    }

    tcpSetNonBlock(fd_, false);
    applyKeepAlive();
    setNoDelay(true);
}

TcpTransport::~TcpTransport() { close(); }

TcpTransport::TcpTransport(TcpTransport&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }

void TcpTransport::sendFrame(const std::vector<uint8_t>& frame) {
    tcpWriteAll(fd_, frame.data(), frame.size());
}

std::vector<uint8_t> TcpTransport::recvFrame(int timeout_ms) {
    uint8_t hdr[FRAME_HEADER_SIZE];
    tcpReadAll(fd_, hdr, FRAME_HEADER_SIZE, timeout_ms);
    int32_t pl = Serializer::peekLength(hdr);
    if (pl < 0) throw std::runtime_error("Bad frame magic");
    std::vector<uint8_t> full(FRAME_HEADER_SIZE + pl);
    std::memcpy(full.data(), hdr, FRAME_HEADER_SIZE);
    tcpReadAll(fd_, full.data() + FRAME_HEADER_SIZE, pl, timeout_ms);
    return Serializer::decode(full.data(), full.size());
}

bool TcpTransport::isConnected() const { return fd_ >= 0; }
int  TcpTransport::fd()          const { return fd_; }

void TcpTransport::close() {
    if (fd_ >= 0) { ::shutdown(fd_, SHUT_RDWR); ::close(fd_); fd_ = -1; }
}

void TcpTransport::applyKeepAlive() {
    int on = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    int idle = 5, intvl = 1, cnt = 3;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPIDLE,  &idle,  sizeof(idle));
    ::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    ::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPCNT,   &cnt,   sizeof(cnt));
}

void TcpTransport::setNoDelay(bool on) {
    int v = on ? 1 : 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

// ─── TcpTransportServer ───────────────────────────────────────────────────────
TcpTransportServer::TcpTransportServer(uint16_t port, int backlog)
    : port_(port), fd_(-1), backlog_(backlog) {}

TcpTransportServer::~TcpTransportServer() { close(); }

void TcpTransportServer::listen() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) throw std::runtime_error("socket: " + tcpSysErr());

    int on = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));

    sockaddr_in a{};
    a.sin_family      = AF_INET;
    a.sin_port        = htons(port_);
    a.sin_addr.s_addr = INADDR_ANY;

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0)
        throw std::runtime_error("bind(:" + std::to_string(port_) + "): " + tcpSysErr());
    if (::listen(fd_, backlog_) < 0)
        throw std::runtime_error("listen: " + tcpSysErr());

    RIPC_DEBUG("TcpServer", "Listening on port " + std::to_string(port_));
}

std::unique_ptr<ITransport> TcpTransportServer::accept() {
    sockaddr_in peer{};
    socklen_t   len = sizeof(peer);
    int cfd = ::accept(fd_, reinterpret_cast<sockaddr*>(&peer), &len);
    if (cfd < 0) throw std::runtime_error("accept: " + tcpSysErr());

    char ip[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
    RIPC_DEBUG("TcpServer", "Client: " + std::string(ip) +
               ":" + std::to_string(ntohs(peer.sin_port)));

    return std::make_unique<TcpTransport>(cfd);
}

void TcpTransportServer::close() {
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
}

int TcpTransportServer::fd() const { return fd_; }

} // namespace rcf
