// ============================================================
//  Tank.cpp
//  Metallic oil/chemical tank – hollow cylinder.
//  The robot's magnetic wheels grip the OUTER surface.
//
//  Geometry:
//    CYL_R  = 10.0   outer radius
//    CYL_H  = 50.0   total height
//    CYL_SEG = 72    circumference segments
//
//  Coordinate frame (tank TF = world):
//    Origin at base centre
//    Y  = up
//    XZ = ground plane
// ============================================================
#include <GL/glut.h>
#include <cmath>
#include <vector>

struct ShellPlate
{
    float hight = 0.0f;
    float width = 0.0f;
    float thickness = 0.0f;
    std::string name = "Default Plate";
};

struct CourceShellPlate
{
    std::vector<ShellPlate> ShellPlates;
};

class Tank
{
public:
    // ── geometry constants ───────────────────────────────────
    // static int courceCount = 8;
    // static int courceCount = 8;

    float TankInnerDia = 39.f; // 39000
    float TankHight = 20.f;    // 20000
    float CYL_R = TankInnerDia / 2;
    float CYL_H = TankHight;

    static constexpr int SEG = 50 * 3;
    static constexpr int LAT = 8;  // latitude  grid rings
    static constexpr int LON = 16; // longitude grid lines

    // draw the full tank (outer wall + caps + surface grid)

    // tank info
    int courceCount = 0;

    std::vector<CourceShellPlate> courseShellPlates;

    Tank()
    {
        courceCount = 8;

        float StandardPlateThickness[courceCount] = {20, 18, 14, 12, 10, 8, 8, 8};
        float StandardPlateCount[courceCount] = {11, 12, 12, 12, 12, 12, 12, 12};

        for (int i = 0; i < courceCount; i++)
        {

            CourceShellPlate courceShellPlate;

            // Standard plate
            // ShellPlate StandardPlate;
            // StandardPlate.hight = 2.5f; // 2500
            // StandardPlate.width = 10.f; // 10000
            // StandardPlate.thickness = StandardPlateThickness[i] / 1000;
            // StandardPlate.name = "S" + std::to_string(i + 1);
            // courceShellPlate.ShellPlates.push_back(StandardPlate);

            // for (int j = 0; j < StandardPlateCount[i]; j++)
            // {
            //     courceShellPlate.ShellPlates.push_back(StandardPlate);
            // }

            // Special Plate Cource 1

            if (i == 0)
            {
                ShellPlate StandardPlate;
                ShellPlate Plate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                Plate.hight = 2.5f;                                 // 2500Plate.hight = 2.5f;                                 // 2500
                Plate.width = 6.25f;                                // 10000
                Plate.thickness = StandardPlateThickness[i] / 1000; // 20
                Plate.name = "S" + std::to_string(i + 1) + "B";
                courceShellPlate.ShellPlates.push_back(Plate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                Plate.hight = 2.5f;                                 // 2500
                Plate.width = 2.579;                                // 10000
                Plate.thickness = StandardPlateThickness[i] / 1000; // 20
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                Plate.hight = 2.5f;                                 // 2500
                Plate.width = 3.75f;                                // 10000
                Plate.thickness = StandardPlateThickness[i] / 1000; // 20
                Plate.name = "S" + std::to_string(i + 1) + "A";
                courceShellPlate.ShellPlates.push_back(Plate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);
            }

            // Special Plate Cource 2

            if (i == 1)
            {
                ShellPlate Plate;
                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                Plate.hight = 2.5f; // 2500
                Plate.width = 2.585;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);
                courceShellPlate.ShellPlates.push_back(StandardPlate);
            }

            // Special Plate Cource 3

            if (i == 2)
            {
                ShellPlate Plate;
                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                for (int j = 0; j < 8; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }

                Plate.hight = 2.5f; // 2500
                Plate.width = 2.566;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                for (int j = 0; j < 4; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            // Special Plate Cource 4

            if (i == 3)
            {
                ShellPlate Plate;
                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                // for (int j = 0; j < 8; j++)
                // {
                //     courceShellPlate.ShellPlates.push_back(StandardPlate);
                // }

                Plate.hight = 2.5f; // 2500
                Plate.width = 2.560;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                for (int j = 0; j < 12; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            // Special Plate Cource 5

            if (i == 4)
            {
                ShellPlate Plate;

                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                for (int j = 0; j < 5; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }

                Plate.hight = 2.5f; // 2500
                Plate.width = 2.554;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                for (int j = 0; j < 7; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            // Special Plate Cource 6

            if (i == 5)
            {
                ShellPlate Plate;

                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                for (int j = 0; j < 6; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
                Plate.hight = 2.5f; // 2500
                Plate.width = 2.547;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);

                for (int j = 0; j < 6; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            // Special Plate Cource 7

            if (i == 6)
            {

                ShellPlate Plate;

                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                for (int j = 0; j < 6; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
                Plate.hight = 2.5f; // 2500
                Plate.width = 2.547;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);
                for (int j = 0; j < 6; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            // Special Plate Cource 8

            if (i == 7)
            {
                ShellPlate Plate;

                ShellPlate StandardPlate;

                StandardPlate.hight = 2.5f; // 2500
                StandardPlate.width = 10.f; // 10000
                StandardPlate.thickness = StandardPlateThickness[i] / 1000;
                StandardPlate.name = "S" + std::to_string(i + 1);

                Plate.hight = 2.5f; // 2500
                Plate.width = 2.547;
                Plate.thickness = StandardPlateThickness[i] / 1000;
                Plate.name = "S" + std::to_string(i + 1) + "M";
                courceShellPlate.ShellPlates.push_back(Plate);
                for (int j = 0; j < 12; j++)
                {
                    courceShellPlate.ShellPlates.push_back(StandardPlate);
                }
            }

            courseShellPlates.push_back(courceShellPlate);
        }
        printTankInfo();
    }

    void printTankInfo()
    {
        std::cout << "Tank Inner Dia: " << TankInnerDia << "  Height: " << TankHight << "\n";

        for (int i = 0; i < courseShellPlates.size(); i++)
        {
            std::cout << "Course " << i + 1 << ":\n";
            std::cout << "Plate Count " << courseShellPlates[i].ShellPlates.size() << ":\n";

            for (auto &sp : courseShellPlates[0].ShellPlates)
            {

                std::cout << "  Plate Name: " << sp.name << "\n";

                std::cout << "  Dimention : " << sp.hight << " X " << sp.width << " X " << sp.thickness << "\n";
            }
        }
    }

    void draw() const
    {
        float yB = 0.0f;  // base at 0,0,0
        float yT = CYL_H; // Hight

        drawOuterWall(yB, yT);
        drawCaps(yB, yT);
        drawSurfaceGrid(yB, yT);
        // drawStair();

        // drawSprayNozzles(5);
        //          drawSprayNozzles(8);
        drawAngleMarkers();

        // drawBottomManhole();
    }

    void setRadius(float radius)
    {
        CYL_R = radius;
    }
    void setHeight(float hight)
    {
        CYL_H = hight;
    }
    float getRadius()
    {
        return CYL_R;
    }
    float getHeight()
    {
        return CYL_H;
    }

private:
    void drawAngleMarkers() const
    {
        const float textR = CYL_R + 1.0f;
        const float tickR1 = CYL_R;
        const float tickR2 = CYL_R + 0.30f;

        const float y = 0.0f;

        glDisable(GL_LIGHTING);

        for (int deg = 360; deg > 0; deg -= 30)
        {
            float a = deg * M_PI / 180.0f;

            glColor3f(1.0f, 1.0f, 0.0f);

            glLineWidth(2.5f);

            glBegin(GL_LINES);

            glVertex3f(
                tickR1 * cosf(a),
                y,
                tickR1 * sinf(a));

            glVertex3f(
                tickR1 * cosf(a),
                Tank::CYL_H,
                tickR1 * sinf(a));

            glEnd();

            //--------------------------------------------------
            // Angle Text
            //--------------------------------------------------
            char txt[8];
            sprintf(txt, "%d", deg);
            glPushMatrix();

            glTranslatef(
                tickR1 * cosf(a),
                1.0f,
                tickR1 * sinf(a));

            glRotatef(-a * 180.0 / M_PI + 90, 0, 1, 0);

            glScalef(0.003f, 0.003f, 0.003f);

            glColor3f(1, 1, 0);

            for (char *c = txt; *c; c++)
                glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);

            glPopMatrix();
        }

        glEnable(GL_LIGHTING);
    }
    void drawSprayNozzles(float hightFromTop) const
    {
        const float ringR = CYL_R + 0.18f;
        const float y = CYL_H - hightFromTop;

        glColor3f(0.85f, 0.05f, 0.05f);

        glLineWidth(5.0f);

        glBegin(GL_LINE_LOOP);

        for (int i = 0; i < 72; i++)
        {
            float a = i * 2.0f * M_PI / 72.0f;

            glVertex3f(
                ringR * cosf(a),
                y,
                ringR * sinf(a));
        }

        glEnd();

        //====================================================
        // Nozzles
        //====================================================

        glColor3f(0.80f, 0.80f, 0.80f);

        for (int i = 0; i < 24; i++)
        {
            float a = i * 2.0f * M_PI / 24.0f;

            float x = ringR * cosf(a);
            float z = ringR * sinf(a);

            //------------------------------------------------
            // Branch Pipe
            //------------------------------------------------

            glBegin(GL_LINES);

            glVertex3f(x, y, z);

            glVertex3f(
                (ringR - 0.18f) * cosf(a),
                y - 0.15f,
                (ringR - 0.18f) * sinf(a));

            glEnd();

            //------------------------------------------------
            // Nozzle
            //------------------------------------------------

            glPushMatrix();

            glTranslatef(
                (ringR - 0.18f) * cosf(a),
                y - 0.15f,
                (ringR - 0.18f) * sinf(a));

            glRotatef(-30, 1, 0, 0);
            glRotatef(-a * 180.0f / M_PI + 90, 0, 1, 0);

            GLUquadric *q = gluNewQuadric();

            gluCylinder(q,
                        0.05,
                        0.03,
                        0.35,
                        8,
                        1);

            gluDeleteQuadric(q);

            glPopMatrix();
        }
    }

    void drawStair() const
    {
        const float stairR = CYL_R + 0.65f;

        const int steps = 500.0;
        const float turns = 1.0f;

        const float treadWidth = 0.65f;
        const float treadDepth = 0.20f;
        const float treadHeight = 0.04f;

        const float railHeight = 0.90f;

        glColor3f(0.55f, 0.55f, 0.55f);

        for (int i = 0; i < steps; i++)
        {
            float t = i / (float)(steps - 1);

            float ang = t * turns * 2.0f * M_PI;

            float x = stairR * cosf(ang);
            float y = t * CYL_H;
            float z = stairR * sinf(ang);

            glPushMatrix();

            glTranslatef(x, y, z);

            glRotatef(-ang * 180.0f / M_PI + 90, 0, 1, 0);

            glScalef(treadDepth,
                     treadHeight,
                     treadWidth);

            glutSolidCube(1.0);

            glPopMatrix();
        }

        glColor3f(1.0f, 0.85f, 0.0f);

        glLineWidth(3);

        glBegin(GL_LINE_STRIP);

        for (int i = 0; i <= steps; i++)
        {
            float t = i / (float)steps;

            float ang = t * turns * 2.0f * M_PI;

            glVertex3f(
                (stairR + 0.32f) * cosf(ang),
                t * CYL_H + railHeight,
                (stairR + 0.32f) * sinf(ang));
        }

        glEnd();

        glBegin(GL_LINE_STRIP);

        for (int i = 0; i <= steps; i++)
        {
            float t = i / (float)steps;

            float ang = t * turns * 2.0f * M_PI;

            glVertex3f(
                (stairR + 0.32f) * cosf(ang),
                t * CYL_H + 0.45f,
                (stairR + 0.32f) * sinf(ang));
        }

        glEnd();

        for (int i = 0; i < steps; i += 3)
        {
            float t = i / (float)(steps - 1);

            float ang = t * turns * 2.0f * M_PI;

            float x = (stairR + 0.32f) * cosf(ang);
            float y = t * CYL_H;
            float z = (stairR + 0.32f) * sinf(ang);

            // Vertical

            glBegin(GL_LINES);

            glVertex3f(x, y, z);
            glVertex3f(x, y + railHeight, z);

            glEnd();

            // Diagonal brace

            if (i + 3 < steps)
            {
                float t2 = (i + 3) / (float)(steps - 1);

                float ang2 = t2 * turns * 2.0f * M_PI;

                glBegin(GL_LINES);

                glVertex3f(x, y + railHeight, z);

                glVertex3f(
                    (stairR + 0.32f) * cosf(ang2),
                    t2 * CYL_H,
                    (stairR + 0.32f) * sinf(ang2));

                glEnd();
            }
        }
    }
    void drawBottomManhole() const
    {
        const float r = 0.75f;
        const float x = CYL_R + 0.02f;
        const float y = 1.0f;
        const float z = 0.0f;

        glPushMatrix();

        glTranslatef(x, y, z);
        glRotatef(90, 0, 1, 0);

        // Cover
        glColor3f(0.55f, 0.55f, 0.55f);

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex3f(0, 0, 0);

        for (int i = 0; i <= 36; i++)
        {
            float a = i * 2.0f * M_PI / 36.0f;
            glVertex3f(r * cosf(a), r * sinf(a), 0);
        }

        glEnd();

        // Bolts
        glColor3f(0.20f, 0.20f, 0.20f);

        for (int i = 0; i < 12; i++)
        {
            float a = i * 2.0f * M_PI / 12.0f;

            float bx = (r - 0.10f) * cosf(a);
            float by = (r - 0.10f) * sinf(a);

            glPushMatrix();
            glTranslatef(bx, by, 0.03f);
            glutSolidSphere(0.04, 8, 8);
            glPopMatrix();
        }

        glPopMatrix();
    }

    void drawOuterWall(float yB, float yT) const
    {
        glColor3f(0.20f, 0.26f, 0.36f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= SEG; i++)
        {
            float a = (float)i / SEG * 2.f * (float)M_PI;
            float c = cosf(a), s = sinf(a);
            glNormal3f(c, 0, s);
            glVertex3f(CYL_R * c, yB, CYL_R * s);
            glVertex3f(CYL_R * c, yT, CYL_R * s);
        }
        glEnd();
    }

    void drawCaps(float yB, float yT) const
    {
        glColor3f(0.15f, 0.18f, 0.26f);
        for (int cap = 0; cap < 2; cap++)
        {
            float yy = (cap == 0) ? yB : yT;
            float ny = (cap == 0) ? -1.f : 1.f;
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0, ny, 0);
            glVertex3f(0, yy, 0);
            for (int i = 0; i <= SEG; i++)
            {
                float a = (float)(cap == 0 ? -i : i) / SEG * 2.f * (float)M_PI;
                glVertex3f(CYL_R * cosf(a), yy, CYL_R * sinf(a));
            }
            glEnd();
        }
    }

    void draw3DText(
        float x,
        float y,
        float z,
        const char *text) const
    {
        glPushAttrib(GL_LIGHTING_BIT);

        glDisable(GL_LIGHTING);

        glRasterPos3f(x, y, z);

        while (*text)
        {
            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                *text++);
        }

        glPopAttrib();
    }

    void drawLine3D(
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float r, float g, float b,
        float width = 1.0f) const
    {
        glColor3f(r, g, b);
        glLineWidth(width);

        glBegin(GL_LINES);

        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y2, z2);

        glEnd();
    }

    void makeCourceLine(float yB, float yT) const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);

        const float radius = CYL_R + 0.01f;
        const float sectionHeight = 2.5f;

        for (int j = 0; j <= courseShellPlates.size(); j++)
        {
            float yy = yB + j * sectionHeight;

            glColor3f(0.34f, 0.40f, 0.56f);
            glLineWidth(1.0f);

            glBegin(GL_LINE_LOOP);

            for (int i = 0; i <= SEG; i++)
            {
                float a = (float)i / SEG * 2.0f * M_PI;

                glVertex3f(
                    radius * cosf(a),
                    yy,
                    radius * sinf(a));
            }

            glEnd();
        }

        glPopAttrib();
    }

    float getCourseRadius(const CourceShellPlate &course) const
    {
        if (course.ShellPlates.empty())
            return CYL_R;

        return TankInnerDia / 2 + (course.ShellPlates.front().thickness);
    }

    void verticalPlateBoundaries(float yB, float yT) const
    {
        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);

        float currentY = yB;

        for (size_t courseIndex = 0;
             courseIndex < courseShellPlates.size();
             ++courseIndex)
        {
            const auto &course = courseShellPlates[courseIndex];

            if (course.ShellPlates.empty())
                continue;

            const float courseHeight = course.ShellPlates.front().hight;
            const float courseRadius = getCourseRadius(course);

            const float halfPlateOffset =
                course.ShellPlates.front().width * 0.5f;

            float distance = (courseIndex % 2 == 1) ? halfPlateOffset : 0.0f;

            for (const auto &plate : course.ShellPlates)
            {
                const float angle = distance / courseRadius;

                const float x = courseRadius * cosf(angle);
                const float z = courseRadius * sinf(angle);

                drawLine3D(
                    x, currentY, z,
                    x, currentY + courseHeight, z,
                    0.34f, 0.40f, 0.56f,
                    2.0f);

                

                const float centerDistance = distance + plate.width * 0.5f;
                const float centerAngle = centerDistance / courseRadius;

                const float textRadius = courseRadius + 0.03f;

                glPushMatrix();

                glTranslatef(
                    textRadius * cosf(centerAngle),
                    currentY + courseHeight * 0.5f,
                    textRadius * sinf(centerAngle));

                glRotatef(
                    -centerAngle * 180.0f / (float)M_PI + 90.0f,
                    0.0f, 1.0f, 0.0f);

                glScalef(0.0025f, 0.0025f, 0.0025f);

                glColor3f(1.0f, 1.0f, 1.0f);

                for (char c : plate.name)
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, c);

                glPopMatrix();

                distance += plate.width;
            }

            currentY += courseHeight;
        }

        glPopAttrib();
    }

    void drawSurfaceGrid(float yB, float yT) const
    {
       
        makeCourceLine(yB, yT);

        verticalPlateBoundaries(yB, yT);
    }
};