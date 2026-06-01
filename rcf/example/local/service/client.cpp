#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>

int main()
{
    rcf::ServiceClient<SetPowerReq, AckRes> cli(SVC_POWER);

    SetPowerReq req;

    // valid call
    req.power = 75;
    AckRes r1 = cli.call(req);
    printf("Set power=75   ->  %s\n", r1.ok ? "OK" : "FAILED");

    // rejected call
    req.power = 999;
    AckRes r2 = cli.call(req);
    printf("Set power=999  ->  %s\n", r2.ok ? "OK" : "REJECTED");

    return 0;
}