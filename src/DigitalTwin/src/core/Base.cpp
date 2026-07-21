// ============================================================
//  Base.h
//  Robot base platform (yellow plate)
// ============================================================
#pragma once

#include <GL/glut.h>
#include <cmath>
#include "Axis.cpp"

class Base
{
public:
    // Pose pose; // relative to robot frame (origin at ground centre)

    // ── constructors ─────────────────────────────────────────
    Base()
    {
        // pose.setPosition(0.f, 0.5f, 0.f);     // centre plate at half thickness above ground
        // pose.setOrientation(-90.f, 0.f, 0.f); // rotate so X (outward) → Y (up), Y (up) → -X (backward)
    }

    Base(float x, float y, float z, float roll, float pitch, float yaw)
    {
        // pose.setPosition(x, y, z);
        // pose.setOrientation(roll, pitch, yaw);
    }

    // ── geometry constants ───────────────────────────────────
    static constexpr float BASE_LENGTH = 0.86f; // X dimension (thickness)
    static constexpr float BASE_WIDTH = 1.425f; // Z dimension (width)
    static constexpr float BASE_HEIGHT = 0.12f; // Y dimension (height)

    // ── material properties ──────────────────────────────────
    void setMaterial() const
    {
        GLfloat yellow_diffuse[] = {0.85f, 0.72f, 0.18f, 1.0f};
        GLfloat yellow_ambient[] = {0.35f, 0.30f, 0.10f, 1.0f};
        GLfloat yellow_specular[] = {0.5f, 0.5f, 0.3f, 1.0f};
        GLfloat shininess = 32.0f;

        glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow_diffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, yellow_ambient);
        glMaterialfv(GL_FRONT, GL_SPECULAR, yellow_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    }

    // ── draw a face with normal ──────────────────────────────
    void drawQuad(const float v[4][3], const float normal[3]) const
    {
        glNormal3fv(normal);
        glBegin(GL_QUADS);
        for (int i = 0; i < 4; ++i)
            glVertex3fv(v[i]);
        glEnd();
    }

    // ── draw the base box ────────────────────────────────────
    void drawBox() const
    {
        float hx = BASE_LENGTH * 0.5f; // half X (depth)
        float hy = BASE_HEIGHT * 0.5f; // half Y (height)
        float hz = BASE_WIDTH * 0.5f;  // half Z (width)

        // 8 vertices of the box (centered at origin)
        float v[8][3] = {
            {-hx, -hy, -hz}, // 0: front-bottom-left
            {hx, -hy, -hz},  // 1: front-bottom-right
            {hx, -hy, hz},   // 2: back-bottom-right
            {-hx, -hy, hz},  // 3: back-bottom-left
            {-hx, hy, -hz},  // 4: front-top-left
            {hx, hy, -hz},   // 5: front-top-right
            {hx, hy, hz},    // 6: back-top-right
            {-hx, hy, hz}    // 7: back-top-left
        };

        // Face indices (6 faces, 4 vertices each)
        int faces[6][4] = {
            {0, 1, 2, 3}, // bottom (Y = -hy)
            {4, 7, 6, 5}, // top    (Y = +hy)
            {0, 4, 5, 1}, // front  (Z = -hz)
            {2, 6, 7, 3}, // back   (Z = +hz)
            {0, 3, 7, 4}, // left   (X = -hx)
            {1, 5, 6, 2}  // right  (X = +hx)
        };

        // Normals for each face
        float normals[6][3] = {
            {0, -1, 0}, // bottom
            {0, 1, 0},  // top
            {0, 0, -1}, // front
            {0, 0, 1},  // back
            {-1, 0, 0}, // left
            {1, 0, 0}   // right
        };

        // Draw all faces
        for (int i = 0; i < 6; ++i)
        {
            float quad[4][3];
            for (int j = 0; j < 4; ++j)
            {
                int idx = faces[i][j];
                quad[j][0] = v[idx][0];
                quad[j][1] = v[idx][1];
                quad[j][2] = v[idx][2];
            }
            drawQuad(quad, normals[i]);
        }
    }

    // ── draw with edges (wireframe overlay) ──────────────────
    void drawEdges() const
    {
        float hx = BASE_LENGTH * 0.5f;
        float hy = BASE_HEIGHT * 0.5f;
        float hz = BASE_WIDTH * 0.5f;

        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(1.5f);

        float v[8][3] = {
            {-hx, -hy, -hz}, {hx, -hy, -hz}, {hx, -hy, hz}, {-hx, -hy, hz}, {-hx, hy, -hz}, {hx, hy, -hz}, {hx, hy, hz}, {-hx, hy, hz}};

        // Edges (12 lines)
        int edges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, // bottom
            {4, 5},
            {5, 6},
            {6, 7},
            {7, 4}, // top
            {0, 4},
            {1, 5},
            {2, 6},
            {3, 7} // vertical
        };

        glBegin(GL_LINES);
        for (int i = 0; i < 12; ++i)
        {
            glVertex3fv(v[edges[i][0]]);
            glVertex3fv(v[edges[i][1]]);
        }
        glEnd();

        glEnable(GL_LIGHTING);
    }

    // ── main draw function ───────────────────────────────────
    void draw() const
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);

        // Apply pose transformation
        glPushMatrix();
        // pose.applyGLTransform();

        // Shift inward so inner face sits at tank-contact level
        // (tyre inner face is at X = 0 in robot frame; base sits just inside)
        float ht = BASE_LENGTH * 0.5f; // half thickness
        glTranslatef(-ht, 0.f, 0.f);   // centre plate behind tyre plane

        // Draw solid box with material
        glEnable(GL_LIGHTING);
        setMaterial();
        drawBox();

        // Draw black edges for definition
        drawEdges();

        // Draw axis for reference (optional)
        // Axis a1("base_link", 1.8f);
        // a1.setOrientation(180.f, 0.f, 0.f);
        // a1.draw();

        glPopMatrix();
        glPopAttrib();
    }

    // ── draw only the box (no axis) ──────────────────────────
    void drawSolid() const
    {
        glPushAttrib(GL_LIGHTING_BIT);

        glPushMatrix();
        // pose.applyGLTransform();

        float ht = BASE_LENGTH * 0.5f;
        glTranslatef(-ht, 0.f, 0.f);

        glEnable(GL_LIGHTING);
        setMaterial();
        drawBox();

        glPopMatrix();
        glPopAttrib();
    }

    // ── draw wireframe only ──────────────────────────────────
    void drawWireframe() const
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);

        glPushMatrix();
        // pose.applyGLTransform();

        float ht = BASE_LENGTH * 0.5f;
        glTranslatef(-ht, 0.f, 0.f);

        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);

        float hx = BASE_LENGTH * 0.5f;
        float hy = BASE_HEIGHT * 0.5f;
        float hz = BASE_WIDTH * 0.5f;

        // Draw 12 edges using GL_LINES
        float v[8][3] = {
            {-hx, -hy, -hz}, {hx, -hy, -hz}, {hx, -hy, hz}, {-hx, -hy, hz}, {-hx, hy, -hz}, {hx, hy, -hz}, {hx, hy, hz}, {-hx, hy, hz}};

        int edges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, // bottom
            {4, 5},
            {5, 6},
            {6, 7},
            {7, 4}, // top
            {0, 4},
            {1, 5},
            {2, 6},
            {3, 7} // vertical
        };

        glBegin(GL_LINES);
        for (int i = 0; i < 12; ++i)
        {
            glVertex3fv(v[edges[i][0]]);
            glVertex3fv(v[edges[i][1]]);
        }
        glEnd();

        glPopMatrix();
        glPopAttrib();
    }
};