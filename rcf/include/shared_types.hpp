#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  shared_types.hpp  —  included by every example server AND client.
//
//  Rules:
//    • Every struct must be trivially copyable (POD).
//    • Use #pragma pack(push,1) so layout is identical on both ends.
//    • Never put std::string, vectors, or pointers inside a wire struct.
//      Use StrVal<N> for text.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <cstring>
#include <string>

#pragma pack(push, 1)

// ── Fixed-size string helper ──────────────────────────────────────────────────
template<int N>
struct StrVal {
    char data[N]{};
    void        set(const char* s)        { std::strncpy(data, s, N-1); }
    void        set(const std::string& s) { set(s.c_str()); }
    const char* c_str() const             { return data; }
    std::string str()   const             { return std::string(data); }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  SERVICE types
// ═══════════════════════════════════════════════════════════════════════════════

// ── Generic ack (used by every Set / Trigger call) ────────────────────────────
struct AckRes {
    uint8_t ok;   // 1 = success, 0 = rejected / error
};

// ── Set<int>  — motor power (0-100 %) ────────────────────────────────────────
struct SetPowerReq  { int32_t power; };

// ── Set<float> — max speed (m/s) ─────────────────────────────────────────────
struct SetSpeedReq  { float speed; };

// ── Set<string> — robot display name ─────────────────────────────────────────
struct SetNameReq   { StrVal<64> name; };

// ── Set<struct> — PID gains ───────────────────────────────────────────────────
struct PidGains     { float kp, ki, kd; };
struct SetPidReq    { PidGains pid; };

// ── Get<float> — battery voltage ─────────────────────────────────────────────
struct GetBattReq   {};           // empty request
struct GetBattRes   { float voltage; };

// ── Get<struct> — full robot status snapshot ─────────────────────────────────
struct GetStatusReq {};
struct RobotStatus  {
    float   battery_v;
    float   cpu_temp_c;
    uint8_t drive_mode;    // 0=idle 1=manual 2=auto
    uint8_t arm_enabled;
};

// ── Trigger  — emergency stop ─────────────────────────────────────────────────
struct EStopReq     {};
struct EStopRes     { uint8_t ok; };

// ── Toggle  — arm enable/disable ─────────────────────────────────────────────
struct ToggleReq    {};
struct ToggleRes    { uint8_t state; };   // new state: 0 or 1

// ── Select  — drive mode ─────────────────────────────────────────────────────
struct SelectReq    { uint8_t index; };
struct SelectRes    { uint8_t confirmed; uint8_t ok; };

// ═══════════════════════════════════════════════════════════════════════════════
//  ACTION types  (all trivially copyable POD)
// ═══════════════════════════════════════════════════════════════════════════════

// ── Action 1: Navigate to (x, y) ─────────────────────────────────────────────
struct NavGoal     { float x, y; };
struct NavFeedback { float progress;    // 0.0 – 1.0
                     float eta_sec; };
struct NavResult   { uint8_t reached;
                     float   final_x, final_y; };

// ── Action 2: Calibrate sensor set ───────────────────────────────────────────
struct CalibGoal   { uint8_t mask; };   // bit0=IMU bit1=Lidar bit2=Camera
struct CalibFeedback {
    char    sensor[16];
    uint8_t done, total;
};
struct CalibResult { uint8_t passed, failed; };

#pragma pack(pop)

// ── Convenience constants ─────────────────────────────────────────────────────
static constexpr const char* MODE_NAMES[] = { "idle", "manual", "auto" };
static constexpr const char* SENSOR_NAMES[] = { "IMU", "Lidar", "Camera" };

// ── Unix socket paths ─────────────────────────────────────────────────────────
static constexpr const char* SVC_POWER   = "/tmp/svc_power";
static constexpr const char* SVC_SPEED   = "/tmp/svc_speed";
static constexpr const char* SVC_NAME    = "/tmp/svc_name";
static constexpr const char* SVC_PID     = "/tmp/svc_pid";
static constexpr const char* SVC_BATT    = "/tmp/svc_batt";
static constexpr const char* SVC_STATUS  = "/tmp/svc_status";
static constexpr const char* SVC_ESTOP   = "/tmp/svc_estop";
static constexpr const char* SVC_TOGGLE  = "/tmp/svc_toggle";
static constexpr const char* SVC_SELECT  = "/tmp/svc_select";

static constexpr const char* ACT_NAV     = "/tmp/act_nav";
static constexpr const char* ACT_CALIB   = "/tmp/act_calib";

// ── TCP port base addresses ───────────────────────────────────────────────────
//  Services use one port each.
//  Actions use three consecutive ports (goal / cancel / result).
static constexpr uint16_t TCP_SVC_POWER  = 9100;
static constexpr uint16_t TCP_SVC_SPEED  = 9101;
static constexpr uint16_t TCP_SVC_NAME   = 9102;
static constexpr uint16_t TCP_SVC_PID    = 9103;
static constexpr uint16_t TCP_SVC_BATT   = 9104;
static constexpr uint16_t TCP_SVC_STATUS = 9105;
static constexpr uint16_t TCP_SVC_ESTOP  = 9106;
static constexpr uint16_t TCP_SVC_TOGGLE = 9107;
static constexpr uint16_t TCP_SVC_SELECT = 9108;

static constexpr uint16_t TCP_ACT_NAV    = 9200;   // +1 cancel, +2 result
static constexpr uint16_t TCP_ACT_CALIB  = 9210;   // +1 cancel, +2 result
