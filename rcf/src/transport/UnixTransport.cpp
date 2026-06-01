#include "rcf/transport/UnixTransport.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>

namespace rcf {

// ─── Helpers (file-local, not in header) ─────────────────────────────────────
static std::string unixSysErr() { return std::string(strerror(errno)); }

static void setNonBlock(int fd, bool on) {
    int f = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, on ? f | O_NONBLOCK : f & ~O_NONBLOCK);
}

static void writeAll(int fd, const uint8_t* d, size_t n) {
    size_t s = 0;
    while (s < n) {
        ssize_t r = ::write(fd, d + s, n - s);
        if (r <= 0) throw std::runtime_error("unix write: " + unixSysErr());
        s += r;
    }
}

static void readAll(int fd, uint8_t* buf, size_t n, int tms) {
    size_t g = 0;
    while (g < n) {
        if (tms >= 0) {
            pollfd p{ fd, POLLIN, 0 };
            int r = ::poll(&p, 1, tms);
            if (r == 0)  throw std::runtime_error("recv timeout");
            if (r < 0)   throw std::runtime_error("poll: " + unixSysErr());
            if ((p.revents & POLLERR) && !(p.revents & POLLIN))
                throw std::runtime_error("socket error");
            if ((p.revents & POLLHUP) && !(p.revents & POLLIN))
                throw std::runtime_error("peer closed");
        }
        ssize_t r = ::read(fd, buf + g, n - g);
        if (r == 0) throw std::runtime_error("connection closed");
        if (r < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("read: " + unixSysErr());
        }
        g += r;
    }
}

// ─── UnixTransport ────────────────────────────────────────────────────────────
UnixTransport::UnixTransport(int fd) : fd_(fd) {}

UnixTransport::UnixTransport(const std::string& path, int timeout_ms)
    : fd_(-1)
{
    fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) throw std::runtime_error("socket: " + unixSysErr());

    setNonBlock(fd_, true);

    sockaddr_un a{};
    a.sun_family = AF_UNIX;
    std::strncpy(a.sun_path, path.c_str(), sizeof(a.sun_path) - 1);

    int rc = ::connect(fd_, reinterpret_cast<sockaddr*>(&a), sizeof(a));
    if (rc < 0 && errno != EINPROGRESS)
        throw std::runtime_error("connect(" + path + "): " + unixSysErr());

    if (rc != 0) {
        pollfd p{ fd_, POLLOUT, 0 };
        if (::poll(&p, 1, timeout_ms) <= 0)
            throw std::runtime_error("connect timeout: " + path);
        int err = 0;
        socklen_t el = sizeof(err);
        ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &el);
        if (err) throw std::runtime_error("connect failed: " + std::string(strerror(err)));
    }
    setNonBlock(fd_, false);
}

UnixTransport::~UnixTransport() { close(); }

UnixTransport::UnixTransport(UnixTransport&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }

void UnixTransport::sendFrame(const std::vector<uint8_t>& frame) {
    writeAll(fd_, frame.data(), frame.size());
}

std::vector<uint8_t> UnixTransport::recvFrame(int timeout_ms) {
    uint8_t hdr[FRAME_HEADER_SIZE];
    readAll(fd_, hdr, FRAME_HEADER_SIZE, timeout_ms);
    int32_t pl = Serializer::peekLength(hdr);
    if (pl < 0) throw std::runtime_error("Bad frame magic");
    std::vector<uint8_t> full(FRAME_HEADER_SIZE + pl);
    std::memcpy(full.data(), hdr, FRAME_HEADER_SIZE);
    readAll(fd_, full.data() + FRAME_HEADER_SIZE, pl, timeout_ms);
    return Serializer::decode(full.data(), full.size());
}

bool UnixTransport::isConnected() const { return fd_ >= 0; }
int  UnixTransport::fd()          const { return fd_; }

void UnixTransport::close() {
    if (fd_ >= 0) { ::shutdown(fd_, SHUT_RDWR); ::close(fd_); fd_ = -1; }
}

// ─── UnixTransportServer ──────────────────────────────────────────────────────
UnixTransportServer::UnixTransportServer(std::string path, int backlog)
    : path_(std::move(path)), fd_(-1), backlog_(backlog) {}

UnixTransportServer::~UnixTransportServer() { close(); }

void UnixTransportServer::listen() {
    ::unlink(path_.c_str());

    fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) throw std::runtime_error("socket: " + unixSysErr());

    sockaddr_un a{};
    a.sun_family = AF_UNIX;
    std::strncpy(a.sun_path, path_.c_str(), sizeof(a.sun_path) - 1);

    if (::bind(fd_, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0)
        throw std::runtime_error("bind(" + path_ + "): " + unixSysErr());
    if (::listen(fd_, backlog_) < 0)
        throw std::runtime_error("listen: " + unixSysErr());

    RIPC_DEBUG("UnixServer", "Listening on " + path_);
}

std::unique_ptr<ITransport> UnixTransportServer::accept() {
    int cfd = ::accept(fd_, nullptr, nullptr);
    if (cfd < 0) throw std::runtime_error("accept: " + unixSysErr());
    return std::make_unique<UnixTransport>(cfd);
}

void UnixTransportServer::close() {
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; ::unlink(path_.c_str()); }
}

int UnixTransportServer::fd() const { return fd_; }

} // namespace rcf
