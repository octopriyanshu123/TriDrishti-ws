// ============================================================
//  Transform.cpp
//  Simple rigid-body transform: translation + roll/pitch/yaw.
//
//  Convention (ROS REP-103):
//    tx, ty, tz  – translation
//    roll        – rotation around X  (deg)
//    pitch       – rotation around Y  (deg)
//    yaw         – rotation around Z  (deg)
//
//  Apply order:  Translate → Yaw → Pitch → Roll
//  (standard aerospace / ROS convention)
// ============================================================
#include <GL/glut.h>

struct Transform
{
    // ── data ─────────────────────────────────────────────────
    float tx = 0.f;     // translation X
    float ty = 0.f;     // translation Y
    float tz = 0.f;     // translation Z

    float roll  = 0.f;  // rotation around X (deg)
    float pitch = 0.f;  // rotation around Y (deg)
    float yaw   = 0.f;  // rotation around Z (deg)

    // ── constructors ─────────────────────────────────────────
    Transform() = default;

    // translation only
    Transform(float tx_, float ty_, float tz_)
        : tx(tx_), ty(ty_), tz(tz_) {}

    // translation + rotation
    Transform(float tx_, float ty_, float tz_,
              float roll_, float pitch_, float yaw_)
        : tx(tx_), ty(ty_), tz(tz_)
        , roll(roll_), pitch(pitch_), yaw(yaw_) {}

    // ── apply to OpenGL matrix stack ─────────────────────────
    //  Translate first, then rotate Yaw→Pitch→Roll
    //  OpenGL multiplies right-to-left, so Roll is applied first
    //  to the geometry, then Pitch, then Yaw — correct order.
    void apply() const
    {
        glTranslatef(tx, ty, tz);

        if (yaw   != 0.f) glRotatef(yaw,   0, 0, 1);  // Z
        if (pitch != 0.f) glRotatef(pitch, 0, 1, 0);  // Y
        if (roll  != 0.f) glRotatef(roll,  1, 0, 0);  // X
    }

    // ── setters ──────────────────────────────────────────────
    void setTranslation(float x, float y, float z)
    {
        tx = x;  ty = y;  tz = z;
    }

    void setRotation(float r, float p, float y)
    {
        roll = r;  pitch = p;  yaw = y;
    }

    void set(float x, float y, float z,
             float r, float p, float yw)
    {
        tx = x;  ty = y;  tz = z;
        roll = r;  pitch = p;  yaw = yw;
    }
};