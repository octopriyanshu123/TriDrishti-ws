#include "rcf/service/ServiceClient.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <stdexcept>

namespace rcf
{

    template <typename Req, typename Res>
    ServiceClient<Req, Res>::ServiceClient(const std::string &path, int timeout_ms)
        : mode_(Mode::UNIX), path_(path), timeout_ms_(timeout_ms)
    {

        RIPC_INFO("ServiceClient", "Constructor Local");
    }

    template <typename Req, typename Res>
    ServiceClient<Req, Res>::ServiceClient(const std::string &host, uint16_t port, int timeout_ms)
        : mode_(Mode::TCP), host_(host), port_(port), timeout_ms_(timeout_ms)
    {
        RIPC_INFO("ServiceClient", "Constructor Network");
    }

    template <typename Req, typename Res>
    Res ServiceClient<Req, Res>::call(const Req &req, int timeout_ms)
    {
                RIPC_INFO("ServiceClient", "call Function Call");

        if (timeout_ms < 0)
            timeout_ms = timeout_ms_;
        if (mode_ == Mode::UNIX)
        {
            UnixTransport t(path_, timeout_ms);
            t.sendFrame(Serializer::encodeStruct(req));
            return Serializer::decodeStruct<Res>(t.recvFrame(timeout_ms));
        }
        else
        {
            TcpTransport t(host_, port_, timeout_ms);
            t.sendFrame(Serializer::encodeStruct(req));
            return Serializer::decodeStruct<Res>(t.recvFrame(timeout_ms));
        }
    }

    template <typename Req, typename Res>
    ServiceStatus ServiceClient<Req, Res>::tryCall(const Req &req, Res &out, int timeout_ms)
    {
                        RIPC_INFO("ServiceClient", "tryCall Function Call");

        try
        {
            out = call(req, timeout_ms);
            return ServiceStatus::OK;
        }
        catch (const std::runtime_error &e)
        {
            std::string m = e.what();
            if (m.find("timeout") != std::string::npos)
                return ServiceStatus::TIMEOUT;
            RIPC_ERROR("ServiceClient", m);
            return ServiceStatus::ERROR;
        }
    }
} // namespace rcf
 