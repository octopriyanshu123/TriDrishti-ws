#include "rcf/service/ServiceServer.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <stdexcept>
#include <unistd.h>

namespace rcf
{

    template <typename Req, typename Res>
    ServiceServer<Req, Res>::ServiceServer(std::unique_ptr<ITransportServer> srv)
        : srv_(std::move(srv)), running_(false)
    {
        RIPC_INFO("ServiceServer", "Constructor");
    }

    template <typename Req, typename Res>
    ServiceServer<Req, Res>::~ServiceServer() { shutdown(); }


    template <typename Req, typename Res>
    std::unique_ptr<ServiceServer<Req, Res>> ServiceServer<Req, Res>::makeUnix(const std::string &path)
    {
        RIPC_INFO("ServiceServer", "makeUnix Function Call");
        return std::make_unique<ServiceServer<Req, Res>>(std::make_unique<UnixTransportServer>(path));
    }

    template <typename Req, typename Res>
    std::unique_ptr<ServiceServer<Req, Res>>    ServiceServer<Req, Res>::makeTcp(uint16_t port)
    {
        return std::make_unique<ServiceServer<Req, Res>>(
            std::make_unique<TcpTransportServer>(port));
    }

    template <typename Req, typename Res>
    void ServiceServer<Req, Res>::bind(Handler h)
    {
        RIPC_INFO("ServiceServer", "bind Function Call");

        handler_ = std::move(h);
    }

    template <typename Req, typename Res>
    void ServiceServer<Req, Res>::spin()
    {
        RIPC_INFO("ServiceServer", "spin Function Call");

        if (!handler_)
            throw std::runtime_error(
                "ServiceServer::spin() called without bind() — "
                "call bind(handler) before spinAsync()");

        srv_->listen(); // opens the socket file on disk, starts accepting connections
        running_.store(true);
        RIPC_INFO("ServiceServer", "Ready");

        while (running_.load())
        {
            try
            {
                auto client = srv_->accept(); // BLOCKS here until a client connects
                if (!running_.load())
                    break;
                std::thread([this, c = std::move(client)]() mutable // spawns a new thread for this client
                            { handle(std::move(c)); })
                    .detach();
            }
            catch (const std::exception &e)
            {
                if (running_.load())
                    RIPC_ERROR("ServiceServer", e.what());
            }
        }
        srv_->close();
    }

    template <typename Req, typename Res>
    void ServiceServer<Req, Res>::spinAsync()
    {
        // ── Guard: catch missing bind() on the calling thread, not inside a
        //    detached background thread where the exception would be lost ──────────
        RIPC_INFO("ServiceServer", "spinAsync Function Call");

        if (!handler_)
            throw std::runtime_error(
                "ServiceServer::spinAsync() called without bind() — "
                "call bind(handler) before spinAsync()");

        spin_thread_ = std::thread([this]
                                   { spin(); });
    }

    template <typename Req, typename Res>
    void ServiceServer<Req, Res>::shutdown()
    {
        RIPC_INFO("ServiceServer", "shutdown Function Call");

        running_.store(false);
        srv_->close();

        if (spin_thread_.joinable())
            spin_thread_.join();
    }

    template <typename Req, typename Res>
    void ServiceServer<Req, Res>::handle(std::unique_ptr<ITransport> client)
    {
        RIPC_INFO("ServiceServer", "handle Function Call");

        try
        {
            // ── Guard inside the connection handler as a safety net ───────────────
            // Should never be needed if spinAsync() guard above is in place,
            // but protects against race conditions if bind() was somehow cleared.
            if (!handler_)
            {
                RIPC_ERROR("ServiceServer::handle", "no handler bound — dropping connection");
                return;
            }

            auto payload = client->recvFrame(5000);
            Req req = Serializer::decodeStruct<Req>(payload);
            Res res = handler_(req);
            client->sendFrame(Serializer::encodeStruct(res));

            // Graceful drain: let the client read EOF before we close the fd
            uint8_t drain[64];
            while (::read(client->fd(), drain, sizeof(drain)) > 0)
            {
            }
        }
        catch (const std::exception &e)
        {
            RIPC_ERROR("ServiceServer::handle", e.what());
        }
    }

} // namespace rcf
