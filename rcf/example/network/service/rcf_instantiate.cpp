// =============================================================================
//  rcf_instantiate.cpp  --  network/service
// =============================================================================
#include "service/ServiceServer.cpp"
#include "service/ServiceClient.cpp"
#include "shared_types.hpp"

namespace rcf {
    template class ServiceServer<SetPowerReq, AckRes>;
    template class ServiceClient<SetPowerReq, AckRes>;
}
