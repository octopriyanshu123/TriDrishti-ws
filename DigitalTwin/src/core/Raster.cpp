#pragma once

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>

class Raster
{
public:
    // =====================================================
    // Geometry
    // =====================================================

    float railLength = 4.5f;
    float railWidth = 0.15f;
    float railHeight = 0.10f;

    float sliderLength = 0.35f;
    float sliderWidth = 0.25f;
    float sliderHeight = 0.20f;

    float sliderPos = 0.0f;
    float speed = 1.0f;

private:
    bool movingLeft = false;
    bool movingRight = false;
    bool isAtHome_ = false;

public:
    Raster()
    {
        static bool seeded = false;

        if (!seeded)
        {
            std::srand(std::time(nullptr));
            seeded = true;
        }

        spawnRandom();
    }

    ~Raster() = default;

    float speed_ = 0.02f; // units per frame

    void moveLeft()
    {
        if (isAtHome_)
        {
            movingLeft = true;
            movingRight = false;
        }
    }

    void moveRight()
    {
        if (isAtHome_)
        {
            movingRight = true;
            movingLeft = false;
        }
    }
    void stop()
    {
        movingLeft = false;
        movingRight = false;
    }

    void home()
    {
        movingLeft = true;
        movingRight = false;
        isAtHome_ = true;
    }

    void end()
    {
        sliderPos = railLength;
        stop();
    }

    void spawnRandom()
    {
        sliderPos = 1.0f +
                    ((float)std::rand() / (float)RAND_MAX) *
                        (4.0f - 1.0f);
    }

    // void update(float dt)
    // {
    //     if (movingLeft)
    //     {
    //         sliderPos -= speed * dt;

    //         if (sliderPos <= 0.0f)
    //         {
    //             sliderPos = 0.0f;
    //             stop();
    //         }
    //     }

    //     if (movingRight)
    //     {
    //         sliderPos += speed * dt;

    //         if (sliderPos >= railLength)
    //         {
    //             sliderPos = railLength;
    //             stop();
    //         }
    //     }
    // }
    int direction = 1;

    void update()
    {
        if (movingLeft)
        {
            sliderPos -= speed_;

            if (sliderPos <= 0.0f)
            {
                sliderPos = 0.0f;
                movingLeft = false;
            }
        }

        if (movingRight)
        {
            sliderPos += speed_;

            if (sliderPos >= railLength)
            {
                sliderPos = railLength;
                movingRight = false;
            }
        }
    }
    float getPosition() const
    {
        return sliderPos;
    }

    bool isMoving() const
    {
        return movingLeft || movingRight;
    }

    void draw() const
    {
        drawRail();
        drawSlider();
    }

    bool isAtHome()
    {
        return isAtHome_;
    }

private:
    // =====================================================
    // Rail
    // =====================================================

    void drawRail() const
    {
        glPushMatrix();

        glTranslatef(
            railLength * 0.5f,
            0.0f,
            0.0f);

        glColor3f(
            1.0f,
            0.55f,
            0.0f);

        glScalef(
            railLength,
            railHeight,
            railWidth);

        glutSolidCube(1.0);

        glPopMatrix();
    }

    // =====================================================
    // Slider
    // =====================================================

    void drawSlider() const
    {
        glPushMatrix();

        glTranslatef(
            sliderPos,
            0.0f,
            0.0f);

        glColor3f(
            0.2f,
            0.6f,
            1.0f);

        glScalef(
            sliderLength,
            sliderHeight,
            sliderWidth);

        glutSolidCube(1.0);

        glPopMatrix();
    }
};