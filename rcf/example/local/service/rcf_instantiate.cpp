// =============================================================================
//  rcf_instantiate.cpp
//
//  THIS IS THE ONLY FILE YOU EDIT WHEN YOU ADD A NEW SERVICE TYPE.
//
//  How it works:
//    The library .cpp files contain the template METHOD BODIES but no
//    instantiations for your types.  This file includes those .cpp files
//    so the compiler sees the full template body, then stamps out concrete
//    code for your structs.
//
//    The src/ directory is added to the include path in CMakeLists.txt
//    (RCF_SRC_DIR) so the includes below are always found regardless of
//    where your project lives on disk.
//
//  What you do when adding a new type pair:
//    1. Define your structs in shared_types.hpp  (must be POD)
//    2. Add two lines here:
//           template class rcf::ServiceServer<MyReq, MyRes>;
//           template class rcf::ServiceClient<MyReq, MyRes>;
//    That's it.
// =============================================================================

// Template bodies — included via RCF_SRC_DIR set in CMakeLists.txt
#include "service/ServiceServer.cpp"
#include "service/ServiceClient.cpp"

// Your struct definitions
#include "shared_types.hpp"

// Explicit instantiations — one pair per service
namespace rcf {

template class ServiceServer<SetPowerReq, AckRes>;
template class ServiceClient<SetPowerReq, AckRes>;

// Add more as your project grows:
// template class ServiceServer<SetSpeedReq, AckRes>;
// template class ServiceClient<SetSpeedReq, AckRes>;

} // namespace rcf
