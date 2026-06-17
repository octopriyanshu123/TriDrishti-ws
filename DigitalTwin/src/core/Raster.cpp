#pragma once

#include <GL/glut.h>

class Raster
{
public:
    float length = 4.5f;
    float width  = 0.15f;
    float height = 0.10f;

    float sliderPos   = 0.0f;
    float sliderSpeed = 1.0f;   // units/sec

private:
    int direction = 1;

public:
    Raster() = default;
    ~Raster() = default;

    void update(float dt)
    {
        sliderPos += direction * sliderSpeed * dt;

        if (sliderPos >= length)
        {
            sliderPos = length;
            direction = -1;
        }

        if (sliderPos <= 0.0f)
        {
            sliderPos = 0.0f;
            direction = 1;
        }
    }

    void draw() const
    {
        drawRail();
        drawSlider();
    }

private:

    void drawRail() const
    {
        glPushMatrix();

        glTranslatef(length * 0.5f, 0.0f, 0.0f);

        glColor3f(1.0f, 0.55f, 0.0f);

        glScalef(length, height, width);

        glutSolidCube(1.0);

        glPopMatrix();
    }

    void drawSlider() const
    {
        glPushMatrix();

        glTranslatef(sliderPos, 0.0f, 0.0f);

        glColor3f(0.2f, 0.6f, 1.0f);

        glScalef(0.35f, 0.20f, 0.25f);

        glutSolidCube(1.0);

        glPopMatrix();
    }
};