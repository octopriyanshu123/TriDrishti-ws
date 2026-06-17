// ============================================================
//  FlatRobot.cpp
//  Differential-drive robot on a FLAT ground plane.
//  Uses the existing Wheel and Base classes unchanged.
//
//  Flat-ground frame  (standard ROS convention):
//    Y = up
//    X = forward
//    Z = left  (right-hand rule)
//
//  How Wheel fits the flat frame:
//    Wheel class spins around its own Z axis.
//    On flat ground the wheel axle must point along world Z.
//    So we place the wheel with NO extra rotation –
//    its Z axis already points sideways.
//    We only rotate 90° around Z to stand the wheel upright
//    (so the tyre touches the ground at Y=0).
//
//  How Base fits the flat frame:
//    Base class has X=outward, Y=up, Z=axle.
//    On flat ground "outward" becomes "up" (Y).
//    So we rotate the base 90° around Z:
//      Base X → world Y  (up)
//      Base Y → world -X (backward, then we flip)
//      Base Z → world Z  (left/right) stays correct
//    Net: glRotatef(-90, 0,0,1) before drawing Base.
//
//  Assembly (in robot local frame, origin at ground centre):
//    Base    : rotated -90° around Z, centred at (0, BASE_T/2, 0)
//    Left  wheel : at (0, TYRE_R, -HALF_SPAN), rotated 90° around Z
//    Right wheel : at (0, TYRE_R, +HALF_SPAN), rotated 90° around Z
//
//  Kinematics (diff-drive, flat ground):
//    arc   = (dL + dR) / 2
//    omega = (dR - dL) / WHEELBASE
//    yaw  += omega
//    x    += arc * cos(yaw)
//    z    += arc * sin(yaw)
// ============================================================
#pragma once

#include <GL/glut.h>
#include <cmath>
#include "Wheel.cpp"
#include "Base.cpp"
#include "Transform.cpp"
#include "Raster.cpp"

class FlatRobot
{
public:
    // ── assembly constants ───────────────────────────────────
    static constexpr float HALF_SPAN = 0.595f;
    static constexpr float WHEELBASE = 1.19f;

    // ── parts ────────────────────────────────────────────────
    Wheel leftWheel{false};
    Wheel rightWheel{true};
    Base base;

    Raster raster;
    // ── world pose ───────────────────────────────────────────
    float x = 0.f;   // world X position
    float z = 0.f;   // world Z position
    float yaw = 0.f; // heading (rad), 0 = +X direction

    // ── interface ────────────────────────────────────────────
    void setSpeed(float leftDeg, float rightDeg)
    {
        leftWheel.setSpeed(leftDeg);
        rightWheel.setSpeed(rightDeg);
    }
    void stop()
    {
        leftWheel.stop();
        rightWheel.stop();
    }

    void reset()
    {
        x = z = yaw = 0.f;
        leftWheel.reset();
        rightWheel.reset();
    }

    // advance one simulation frame
    void update()
    {
        const float toRad = (float)M_PI / 180.f;
        float dL = leftWheel.speedDeg * toRad * Wheel::TYRE_R;
        float dR = rightWheel.speedDeg * toRad * Wheel::TYRE_R;

        float arc = (dL + dR) * 0.5f;
        float omega = (dR - dL) / WHEELBASE;

        yaw += omega;
        x += arc * cosf(yaw);
        z += arc * sinf(yaw);

        leftWheel.update();
        rightWheel.update();
    }

    // ── draw ─────────────────────────────────────────────────

    //  Transform tree:
    //
    //  world
    //   └── robotWorld          tx=x  tz=z  yaw=yaw_deg   (robot in world)
    //         ├── leftWheelTF   ty=TYRE_R  tz=-HALF_SPAN  roll=-90  (stand upright + left side)
    //         ├── rightWheelTF  ty=TYRE_R  tz=+HALF_SPAN  roll=-90  (stand upright + right side)
    //         └── baseTF        ty=BASE_HEIGHT/2  roll=-90          (plate above ground)
    //
    void draw() const
    {
        // Axis("Robot", 1.0f).draw();

        Transform robotTf;
        robotTf.set(
            0.0f, // X
            0.0f, // Z
            0.0f, // Z

            0.0f,  // roll
            90.0f, // pitch (Y axis)
            0.0f   // yaw
        );

        robotTf.apply();
        drawRobot();
    }

    void drawRobot() const
    {
        drawBase();
        drawLeftWheel();
        drawRightWheel();
        drawRaster();
    }

    void rasterupdate(){
        raster.update();
    }

    void rasterHome(double dt)
    {
        if (raster.isAtHome() == false)
        {
            raster.home();
        }
    }

    void rasterLeft(double dt)
    {
        raster.moveLeft();
    }
    void rasteRight(double dt)
    {
        raster.moveRight();
    }

private:
    // ── drawBase ─────────────────────────────────────────────
    //  Base sits flat on the ground, centred between the wheels.
    //  Base class has X=outward (thickness direction).
    //  On flat ground X must point up (Y), so roll = -90°.
    //  Translate up by half the plate thickness so bottom = Y=0.
    void drawBase() const
    {
        Transform baseTF;
        baseTF.setTranslation(0.43f, 0.16375f, 0.f);
        baseTF.setRotation(-180.f, 0.f, 0.f);

        glPushMatrix(); // ── PUSH base
        baseTF.apply();
        base.draw();
        glPopMatrix(); // ── POP base
    }

    // ── drawLeftWheel ────────────────────────────────────────
    //  Left wheel sits at -Z (left side), height = TYRE_R (centre).
    //  Wheel geometry spins around its own Z axis by default.
    //  roll = -90° stands the wheel upright (disk vertical, axle along Z).
    //  pitch = 180° flips the left wheel so its axis faces outward (-Z).
    void drawLeftWheel() const
    {
        Transform leftWheelTF;
        leftWheelTF.setTranslation(0.495f, 0.16375f, 0.f);
        leftWheelTF.setRotation(0.f, 90.f, 0.f); // roll -90, pitch 180

        glPushMatrix(); // ── PUSH left wheel
        leftWheelTF.apply();
        leftWheel.draw(0);
        glPopMatrix(); // ── POP left wheel
    }

    // ── drawRightWheel ───────────────────────────────────────
    //  Right wheel sits at +Z (right side), height = TYRE_R.
    //  roll = -90° stands the wheel upright.
    //  No pitch flip needed — right wheel faces outward (+Z) by default.
    void drawRightWheel() const
    {
        Transform rightWheelTF;
        rightWheelTF.setTranslation(-0.495f, 0.16375, 0.f);
        rightWheelTF.setRotation(0.f, 90.f, 0.f); // roll -90 only

        glPushMatrix(); // ── PUSH right wheel
        rightWheelTF.apply();
        rightWheel.draw(1);
        glPopMatrix(); // ── POP right wheel
    }

    void drawRaster() const
    {
        glPushMatrix();
        Transform rasterTf;
        rasterTf.setTranslation(-1.8, 0.2, 1);
        rasterTf.apply();
        raster.draw();
        glPopMatrix();
    }
};
