#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
//  rcf.hpp  —  Single public header for the rcf library.
//
//  This is the ONLY file clients need to include.
//  All implementation is compiled into librobotipc.a / librobotipc.so.
//
//  Usage:
//    #include "rcf.hpp"
//    using namespace rcf;
// ═══════════════════════════════════════════════════════════════════════════════

// ── Core ──────────────────────────────────────────────────────────────────────
#include "rcf/Types.hpp"
#include "rcf/Logger.hpp"
#include "rcf/Serializer.hpp"

// ── Transport ─────────────────────────────────────────────────────────────────
#include "rcf/transport/ITransport.hpp"
#include "rcf/transport/UnixTransport.hpp"
#include "rcf/transport/TcpTransport.hpp"

// ── Service ───────────────────────────────────────────────────────────────────
#include "rcf/service/ServiceServer.hpp"
#include "rcf/service/ServiceClient.hpp"

// ── Action ────────────────────────────────────────────────────────────────────
#include "rcf/action/GoalHandle.hpp"
#include "rcf/action/ActionServer.hpp"
#include "rcf/action/ActionClient.hpp"
