#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>
#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>

static int32_t           g_power   = 0;
static std::atomic<bool> g_running{ true };

static AckRes on_power(const SetPowerReq& req)
{
    AckRes res;
    if (req.power < 0 || req.power > 100) {
        printf("  REJECTED  power=%d  (valid: 0-100)\n", req.power);
        res.ok = 0;
        return res;
    }
    g_power = req.power;
    printf("  power -> %d%%\n", g_power);
    res.ok = 1;
    return res;
}

int main()
{
    std::signal(SIGINT, [](int){ g_running = false; });
    auto srv = rcf::ServiceServer<SetPowerReq, AckRes>::makeTcp(TCP_SVC_POWER);
    srv->bind(on_power);
    srv->spinAsync();

    printf("Server listening on TCP port %d  --  Ctrl-C to stop\n\n", TCP_SVC_POWER);

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    srv->shutdown();
    return 0;
}
