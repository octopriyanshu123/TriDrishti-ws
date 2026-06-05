// ============================================================
//  Axis.h
//  Reusable TF axis frame with custom location and orientation.
// ============================================================
#pragma once

#include <GL/glut.h>
#include <cstring>
#include <cmath>

class Axis
{
public:
    char name[64];
    float size;
    
    // Transform components
    float tx, ty, tz;     // translation
    float rx, ry, rz;     // rotation (Euler angles in degrees)

    // ── constructors ─────────────────────────────────────────
    Axis() : size(1.f), tx(0), ty(0), tz(0), rx(0), ry(0), rz(0)
    {
        name[0] = '\0';
    }

    Axis(const char *n, float s) : size(s), tx(0), ty(0), tz(0), rx(0), ry(0), rz(0)
    {
        strncpy(name, n, 63);
        name[63] = '\0';
    }
    
    // Constructor with position
    Axis(const char *n, float s, float x, float y, float z) 
        : size(s), tx(x), ty(y), tz(z), rx(0), ry(0), rz(0)
    {
        strncpy(name, n, 63);
        name[63] = '\0';
    }
    
    // Constructor with position and orientation
    Axis(const char *n, float s, float x, float y, float z, float roll, float pitch, float yaw)
        : size(s), tx(x), ty(y), tz(z), rx(roll), ry(pitch), rz(yaw)
    {
        strncpy(name, n, 63);
        name[63] = '\0';
    }

    // ── transformation setters ──────────────────────────────
    void setPosition(float x, float y, float z)
    {
        tx = x; ty = y; tz = z;
    }
    
    void setOrientation(float roll, float pitch, float yaw)
    {
        rx = roll; ry = pitch; rz = yaw;
    }
    
    void setTransform(float x, float y, float z, float roll, float pitch, float yaw)
    {
        tx = x; ty = y; tz = z;
        rx = roll; ry = pitch; rz = yaw;
    }

    // ── main draw function ───────────────────────────────────
    void draw() const
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);
        
        // Apply custom location and orientation
        glPushMatrix();
        applyTransform();
        
        // Draw the three axes
        drawXAxis();
        drawYAxis();
        drawZAxis();
        
        // Draw label at origin with offset
        drawLabel();
        
        glPopMatrix();  // restore transform
        glPopAttrib();
    }

private:
    // ── apply custom transform ──────────────────────────────
    void applyTransform() const
    {
        // Translate to position
        glTranslatef(tx, ty, tz);
        
        // Apply rotations in ZYX order (yaw, pitch, roll)
        if (rz != 0.0f) glRotatef(rz, 0.0f, 0.0f, 1.0f);  // yaw
        if (ry != 0.0f) glRotatef(ry, 0.0f, 1.0f, 0.0f);  // pitch
        if (rx != 0.0f) glRotatef(rx, 1.0f, 0.0f, 0.0f);  // roll
    }
    
    // ── axis drawing functions ──────────────────────────────
    void drawXAxis() const
    {
        glColor3f(1.0f, 0.2f, 0.2f);  // Red
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);  // Rotate default Z to X
        drawArrow(size);
        glPopMatrix();
    }
    
    void drawYAxis() const
    {
        glColor3f(0.2f, 1.0f, 0.2f);  // Green
        glPushMatrix();
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        drawArrow(size);
        glPopMatrix();
    }
    
    void drawZAxis() const
    {
        glColor3f(0.2f, 0.4f, 1.0f);  // Blue
        glPushMatrix();

        glRotatef(-90.0f, 0.0f,1.0f, 0.0f); // Rotate around X to point Z upward
        glRotatef(-90.0f, 1.0f,0.0f, 0.0f); // Rotate around X to point Z upward

        drawArrow(size);
        glPopMatrix();
    }
    
    // ── label drawing ───────────────────────────────────────
    void drawLabel() const
    {
        if (name[0] == '\0') return;
        
        glColor3f(0.9f, 0.9f, 0.5f);
        float offset = size * 0.12f;
        glRasterPos3f(offset, offset, offset);
        
        for (const char *p = name; *p; ++p)
        {
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
        }
    }
    
    // ── arrow primitive ─────────────────────────────────────
    static void drawArrow(float len)
    {
        GLUquadric *q = gluNewQuadric();
        
        // Shaft
        gluCylinder(q, len * 0.04f, len * 0.04f, len * 0.76f, 8, 1);
        
        // Arrow head
        glPushMatrix();
        glTranslatef(0, 0, len * 0.76f);
        gluCylinder(q, len * 0.10f, 0.0f, len * 0.24f, 8, 1);
        glPopMatrix();
        
        gluDeleteQuadric(q);
    }
};