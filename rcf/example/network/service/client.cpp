
#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>
#include <string>

int main(int argc, char** argv)
{
    const std::string ip = (argc > 1) ? argv[1] : "127.0.0.1";
    printf("=== NETWORK SERVICE CLIENT -> %s:%d ===\n\n", ip.c_str(), TCP_SVC_POWER);

    rcf::ServiceClient<SetPowerReq, AckRes> cli(ip, TCP_SVC_POWER);

    SetPowerReq req;

    req.power = 75;
    AckRes r1 = cli.call(req);
    printf("Set power=75   ->  %s\n", r1.ok ? "OK" : "FAILED");

    req.power = 999;
    AckRes r2 = cli.call(req);
    printf("Set power=999  ->  %s\n", r2.ok ? "OK?" : "REJECTED");

    return 0;
}
