#include <iostream>

#include "Stm.hpp"

namespace
{

void report(const char* label, STMHal::Result result)
{
    std::cout << label << ": "
              << (result == STMHal::Result::SUCCESS ? "OK" : "FAILED")
              << '\n';
}

}

int main()
{
    STMHal::Stm stm;

    report("initialize all", stm.initializeAll());

    // Turn on only the channels we actually want — turnOn() IS activate(),
    // so there's no separate "activate all" step before this.
    report("channel 0 on", stm.channel(0).turnOn());
    report("channel 2 on", stm.channel(2).turnOn());

    // The raster needs to be ACTIVE before it will accept motion commands.
    report("activate raster", stm.raster().activate());
    report("raster home", stm.raster().homePosition());
    report("raster right side", stm.raster().rightSide());
    std::cout << "raster position: " << stm.raster().currentPosition() << '\n';

    report("channel 0 off", stm.channel(0).turnOff());

    report("shutdown all", stm.shutdownAll());

    return 0;
}