// // // ================================================================
// // //  Differential-Drive Robot on Cylinder Surface – OpenGL / GLUT
// // //
// // //  KINEMATICS (tangent-plane diff-drive on cylinder):
// // //
// // //    State  :  theta  – angle around cylinder Y-axis   (rad)
// // //              height – position along   cylinder Y-axis
// // //              phi    – heading in tangent plane        (rad)
// // //                       phi=0  → circumferential (+theta)
// // //                       phi=π/2→ axial           (+height)
// // //
// // //    Update (dt = 1 frame):
// // //      arc   = (vL + vR) * 0.5          [arc-length / frame]
// // //      omega = (vR - vL) / WHEELBASE    [heading rate rad/frame]
// // //      phi  += omega
// // //      theta+= arc * cos(phi) / CYL_R   [angle  from arc-len]
// // //      height+= arc * sin(phi)          [axial  from arc-len]
// // //
// // //    Model matrix (4 steps):
// // //      1. Translate to surface point (R·cosθ, h, R·sinθ)
// // //      2. RotateY(θ)        – align outward normal
// // //      3. RotateZ(-90°)     – tilt robot to STAND on surface
// // //      4. RotateY(φ)        – heading in tangent plane
// // //
// // //  Controls:
// // //    W/S      – both wheels forward / backward
// // //    A/D      – left wheel slower / faster  (turn)
// // //    Q/E      – yaw in place
// // //    R        – reset
// // //    ESC      – quit
// // //    Mouse drag  – orbit camera
// // //    Scroll      – zoom
// // // ================================================================

// // #include <GL/glut.h>
// // #include <cmath>
// // #include <string>
// // #include <cstdio>

// // // ── window ──────────────────────────────────────────────────
// // static int  W = 1280, H = 800;
// // static const char* TITLE = "Robot on Cylinder Surface";

// // // ── camera ──────────────────────────────────────────────────
// // static float camTheta =  40.0f;   // azimuth  deg
// // static float camPhi   =  20.0f;   // elevation deg
// // static float camDist  =  35.0f;
// // static int   lastX = 0, lastY = 0;
// // static bool  dragging = false;

// // // ────────────────────────────────────────────────────────────
// // //  Cylinder geometry
// // // ────────────────────────────────────────────────────────────
// // static const float CYL_R       = 5.0f;    // outer radius
// // static const float CYL_INNER_R = 4.7f;    // inner radius
// // static const float CYL_H       = 15.0f;   // total height
// // static const int   CYL_SEG     = 80;      // circumference segments
// // static const int   CYL_VSTEP   = 1;       // vertical steps

// // // ────────────────────────────────────────────────────────────
// // //  Robot geometry  (same as before)
// // // ────────────────────────────────────────────────────────────
// // static const float BODY_W       = 2.0f;
// // static const float BODY_H       = 2.0f;
// // static const float BODY_D       = 2.0f;
// // static const float WHEEL_R      = 0.5f;
// // static const float WHEEL_T      = 0.25f;
// // static const int   WHEEL_SLICES = 40;
// // static const float WHEEL_AXLE_X = BODY_W * 0.5f + WHEEL_T * 0.5f;
// // static const float WHEELBASE    = BODY_W + WHEEL_T;   // distance L<->R wheel

// // // ────────────────────────────────────────────────────────────
// // //  Robot state  (cylinder-surface kinematics)
// // // ────────────────────────────────────────────────────────────
// // static float rTheta  = 0.0f;    // angle around cylinder  (rad)
// // static float rHeight = 0.0f;    // position along Y axis
// // static float rPhi    = 0.0f;    // heading in tangent plane (rad)
// //                                  //  0 = circumferential, π/2 = axial

// // static float leftSpeed  = 0.0f; // arc-length per frame, left  wheel
// // static float rightSpeed = 0.0f; // arc-length per frame, right wheel

// // static float wheelAngleL = 0.0f; // visual spin  (deg)
// // static float wheelAngleR = 0.0f;

// // // Height clamp so robot stays on cylinder
// // static const float H_MIN = -CYL_H * 0.5f + WHEEL_R + 0.05f;
// // static const float H_MAX =  CYL_H * 0.5f - WHEEL_R - 0.05f;

// // // ── grid ────────────────────────────────────────────────────
// // static const int   GRID_HALF = 20;
// // static const float GRID_STEP = 2.0f;

// // // ================================================================
// // //  helpers
// // // ================================================================
// // static void setColor(float r, float g, float b, float a = 1.f)
// // { glColor4f(r, g, b, a); }

// // // Cylinder (open barrel) aligned along X, centred at origin
// // static void drawCylinderX(float radius, float length, int slices)
// // {
// //     float half = length * 0.5f;
// //     glBegin(GL_TRIANGLE_STRIP);
// //     for (int i = 0; i <= slices; ++i) {
// //         float ang = (float)i / slices * 2.f * (float)M_PI;
// //         float yy  = radius * cosf(ang);
// //         float zz  = radius * sinf(ang);
// //         glNormal3f(0, cosf(ang), sinf(ang));
// //         glVertex3f(-half, yy, zz);
// //         glVertex3f( half, yy, zz);
// //     }
// //     glEnd();
// // }

// // static void drawDiskX(float radius, float x, int slices, bool flip)
// // {
// //     glBegin(GL_TRIANGLE_FAN);
// //     glNormal3f(flip ? -1.f : 1.f, 0, 0);
// //     glVertex3f(x, 0, 0);
// //     int n = flip ? slices : 0;
// //     int d = flip ? -1 : 1;
// //     for (int i = 0; i <= slices; ++i) {
// //         int  idx = n + d * i;
// //         float ang = (float)idx / slices * 2.f * (float)M_PI;
// //         glVertex3f(x, radius * cosf(ang), radius * sinf(ang));
// //     }
// //     glEnd();
// // }

// // static void drawWheel(float radius, float thickness, int slices)
// // {
// //     drawCylinderX(radius, thickness, slices);
// //     drawDiskX(radius, -thickness * 0.5f, slices, true);
// //     drawDiskX(radius,  thickness * 0.5f, slices, false);
// // }

// // // ================================================================
// // //  Axis arrows  (TF-style)   X=red  Y=green  Z=blue
// // // ================================================================
// // static void drawAxisArrow(float len, float headLen, float r)
// // {
// //     float bodyLen = len - headLen;
// //     GLUquadric* q = gluNewQuadric();
// //     gluCylinder(q, r, r, bodyLen, 10, 1);
// //     glPushMatrix();
// //       glTranslatef(0, 0, bodyLen);
// //       gluCylinder(q, r * 2.5f, 0, headLen, 10, 1);
// //     glPopMatrix();
// //     gluDeleteQuadric(q);
// // }

// // static void drawAxes(float scale)
// // {
// //     glPushAttrib(GL_LIGHTING_BIT);
// //     glDisable(GL_LIGHTING);

// //     setColor(1, 0.1f, 0.1f);
// //     glPushMatrix(); glRotatef(90, 0,1,0);
// //     drawAxisArrow(scale, scale*0.25f, scale*0.03f);
// //     glPopMatrix();

// //     setColor(0.1f, 1, 0.1f);
// //     glPushMatrix(); glRotatef(-90,1,0,0);
// //     drawAxisArrow(scale, scale*0.25f, scale*0.03f);
// //     glPopMatrix();

// //     setColor(0.2f, 0.4f, 1);
// //     drawAxisArrow(scale, scale*0.25f, scale*0.03f);

// //     glPopAttrib();
// // }

// // // ================================================================
// // //  Hollow Cylinder  (the track the robot drives on)
// // // ================================================================
// // static void drawHollowCylinder()
// // {
// //     const float yBot   = -CYL_H * 0.5f;
// //     const float yTop   =  CYL_H * 0.5f;
// //     const float TWO_PI =  2.f * (float)M_PI;

// //     // ── Outer wall ───────────────────────────────────────────
// //     setColor(0.18f, 0.22f, 0.32f, 1.f);
// //     glBegin(GL_TRIANGLE_STRIP);
// //     for (int i = 0; i <= CYL_SEG; ++i) {
// //         float a  = (float)i / CYL_SEG * TWO_PI;
// //         float c  = cosf(a), s = sinf(a);
// //         glNormal3f(c, 0, s);
// //         glVertex3f(CYL_R * c, yBot, CYL_R * s);
// //         glVertex3f(CYL_R * c, yTop, CYL_R * s);
// //     }
// //     glEnd();

// //     // ── Inner wall ───────────────────────────────────────────
// //     setColor(0.12f, 0.15f, 0.22f, 1.f);
// //     glBegin(GL_TRIANGLE_STRIP);
// //     for (int i = 0; i <= CYL_SEG; ++i) {
// //         float a  = (float)i / CYL_SEG * TWO_PI;
// //         float c  = cosf(a), s = sinf(a);
// //         glNormal3f(-c, 0, -s);
// //         glVertex3f(CYL_INNER_R * c, yTop, CYL_INNER_R * s);
// //         glVertex3f(CYL_INNER_R * c, yBot, CYL_INNER_R * s);
// //     }
// //     glEnd();

// //     // ── Top annulus ──────────────────────────────────────────
// //     setColor(0.25f, 0.30f, 0.42f, 1.f);
// //     glBegin(GL_TRIANGLE_STRIP);
// //     for (int i = 0; i <= CYL_SEG; ++i) {
// //         float a = (float)i / CYL_SEG * TWO_PI;
// //         float c = cosf(a), s = sinf(a);
// //         glNormal3f(0, 1, 0);
// //         glVertex3f(CYL_INNER_R * c, yTop, CYL_INNER_R * s);
// //         glVertex3f(CYL_R       * c, yTop, CYL_R       * s);
// //     }
// //     glEnd();

// //     // ── Bottom annulus ───────────────────────────────────────
// //     setColor(0.25f, 0.30f, 0.42f, 1.f);
// //     glBegin(GL_TRIANGLE_STRIP);
// //     for (int i = 0; i <= CYL_SEG; ++i) {
// //         float a = (float)i / CYL_SEG * TWO_PI;
// //         float c = cosf(a), s = sinf(a);
// //         glNormal3f(0, -1, 0);
// //         glVertex3f(CYL_R       * c, yBot, CYL_R       * s);
// //         glVertex3f(CYL_INNER_R * c, yBot, CYL_INNER_R * s);
// //     }
// //     glEnd();

// //     // ── Longitude lines on outer surface (visual guide) ──────
// //     glPushAttrib(GL_LIGHTING_BIT);
// //     glDisable(GL_LIGHTING);
// //     setColor(0.30f, 0.35f, 0.50f, 0.6f);
// //     glLineWidth(1.0f);
// //     int gridLines = 16;
// //     for (int i = 0; i < gridLines; ++i) {
// //         float a = (float)i / gridLines * TWO_PI;
// //         float c = cosf(a), s = sinf(a);
// //         glBegin(GL_LINES);
// //         glVertex3f((CYL_R + 0.01f)*c, yBot, (CYL_R + 0.01f)*s);
// //         glVertex3f((CYL_R + 0.01f)*c, yTop, (CYL_R + 0.01f)*s);
// //         glEnd();
// //     }
// //     // Latitude rings
// //     int rings = 8;
// //     for (int j = 0; j <= rings; ++j) {
// //         float yy = yBot + (float)j / rings * CYL_H;
// //         glBegin(GL_LINE_LOOP);
// //         for (int i = 0; i <= CYL_SEG; ++i) {
// //             float a = (float)i / CYL_SEG * TWO_PI;
// //             glVertex3f((CYL_R+0.01f)*cosf(a), yy, (CYL_R+0.01f)*sinf(a));
// //         }
// //         glEnd();
// //     }
// //     glPopAttrib();
// // }

// // // ================================================================
// // //  Robot body – plain cuboid (base only, no decorations)
// // // ================================================================
// // static void drawRobotBody()
// // {
// //     float w = BODY_W * 0.5f;
// //     float h = BODY_H * 0.5f;
// //     float d = BODY_D * 0.5f;

// //     // Body is centred at origin of the robot frame.
// //     // The robot frame sits ON the cylinder surface,
// //     // so the body's inner face (-X in robot frame) touches the cylinder.
// //     // We shift body outward by half its width so the inner face = surface.
// //     glPushMatrix();
// //     glTranslatef(h, 0, 0);   // shift outward (local +X = outward normal)

// //     // // +X face (outer / top when on cylinder)
// //     // setColor(0.85f, 0.85f, 0.85f);
// //     // glBegin(GL_QUADS);
// //     // glNormal3f(1,0,0);
// //     // glVertex3f( w, -h, -d); glVertex3f( w,  h, -d);
// //     // glVertex3f( w,  h,  d); glVertex3f( w, -h,  d);
// //     // glEnd();

// //     // // -X face (inner / surface-side)
// //     // setColor(0.30f, 0.30f, 0.30f);
// //     // glBegin(GL_QUADS);
// //     // glNormal3f(-1,0,0);
// //     // glVertex3f(-w, -h,  d); glVertex3f(-w,  h,  d);
// //     // glVertex3f(-w,  h, -d); glVertex3f(-w, -h, -d);
// //     // glEnd();

// //     // // +Z face (front – where robot faces)
// //     // setColor(1.0f, 0.55f, 0.1f);
// //     // glBegin(GL_QUADS);
// //     // glNormal3f(0,0,1);
// //     // glVertex3f(-w, -h,  d); glVertex3f( w, -h,  d);
// //     // glVertex3f( w,  h,  d); glVertex3f(-w,  h,  d);
// //     // glEnd();

// //     // // -Z face (back)
// //     // setColor(0.6f, 0.30f, 0.05f);
// //     // glBegin(GL_QUADS);
// //     // glNormal3f(0,0,-1);
// //     // glVertex3f( w, -h, -d); glVertex3f(-w, -h, -d);
// //     // glVertex3f(-w,  h, -d); glVertex3f( w,  h, -d);
// //     // glEnd();

// //     // // +Y face (right side of robot)
// //     // setColor(0.1f, 0.65f, 0.65f);
// //     // glBegin(GL_QUADS);
// //     // glNormal3f(0,1,0);
// //     // glVertex3f(-w,  h, -d); glVertex3f(-w,  h,  d);
// //     // glVertex3f( w,  h,  d); glVertex3f( w,  h, -d);
// //     // glEnd();

// //     // -Y face (left side of robot)
// //     setColor(0.08f, 0.45f, 0.45f);
// //     glBegin(GL_QUADS);
// //     glNormal3f(0,-1,0);
// //     glVertex3f(-w, -h,  d); glVertex3f(-w, -h, -d);
// //     glVertex3f( w, -h, -d); glVertex3f( w, -h,  d);
// //     glEnd();

// //     glPopMatrix();
// // }

// // // ================================================================
// // //  Full robot draw  (body + 2 wheels)
// // //  Called INSIDE the robot's local frame
// // // ================================================================
// // static void drawRobot()
// // {
// //     drawRobotBody();

// //     // Wheels are at ±Y of robot centre, offset outward by body half-width
// //     float axleOutward = BODY_H * 0.5f;  // outward offset same as body half

// //     // Left wheel  (–Y in robot frame)
// //     setColor(0.15f, 0.15f, 0.15f);
// //     glPushMatrix();
// //     glTranslatef(axleOutward, -WHEEL_AXLE_X, 0);
// //     glRotatef(wheelAngleL, 0, 0, 1);   // spin around Z (axial in robot frame)
// //     drawWheel(WHEEL_R, WHEEL_T, WHEEL_SLICES);
// //     glPopMatrix();

// //     // Right wheel  (+Y in robot frame)
// //     setColor(0.15f, 0.15f, 0.15f);
// //     glPushMatrix();
// //     glTranslatef(axleOutward,  WHEEL_AXLE_X, 0);
// //     glRotatef(wheelAngleR, 0, 0, 1);
// //     drawWheel(WHEEL_R, WHEEL_T, WHEEL_SLICES);
// //     glPopMatrix();

// //     // Body-frame TF axes at robot origin
// //     drawAxes(1.5f);
// // }

// // // ================================================================
// // //  XZ ground grid  (y = -CYL_H/2 - 1, just below cylinder)
// // // ================================================================
// // static void drawGrid()
// // {
// //     float yy = -CYL_H * 0.5f - 1.0f;
// //     glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
// //     glDisable(GL_LIGHTING);
// //     glEnable(GL_BLEND);
// //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// //     glLineWidth(1.0f);
// //     glBegin(GL_LINES);
// //     for (int i = -GRID_HALF; i <= GRID_HALF; ++i) {
// //         float fi  = (float)i * GRID_STEP;
// //         bool  maj = (i % 5 == 0);
// //         if (maj) glColor4f(0.50f, 0.50f, 0.55f, 0.8f);
// //         else     glColor4f(0.30f, 0.30f, 0.35f, 0.4f);
// //         glVertex3f(fi, yy, -(float)GRID_HALF * GRID_STEP);
// //         glVertex3f(fi, yy,  (float)GRID_HALF * GRID_STEP);
// //         glVertex3f(-(float)GRID_HALF * GRID_STEP, yy, fi);
// //         glVertex3f( (float)GRID_HALF * GRID_STEP, yy, fi);
// //     }
// //     glEnd();
// //     glPopAttrib();
// // }

// // // ================================================================
// // //  HUD
// // // ================================================================
// // static void renderText(float x, float y, const std::string& s,
// //                         float r, float g, float b)
// // {
// //     glMatrixMode(GL_PROJECTION);
// //     glPushMatrix(); glLoadIdentity();
// //     gluOrtho2D(0, W, 0, H);
// //     glMatrixMode(GL_MODELVIEW);
// //     glPushMatrix(); glLoadIdentity();
// //     glDisable(GL_LIGHTING);
// //     glColor3f(r, g, b);
// //     glRasterPos2f(x, y);
// //     for (char c : s) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
// //     glEnable(GL_LIGHTING);
// //     glPopMatrix();
// //     glMatrixMode(GL_PROJECTION);
// //     glPopMatrix();
// //     glMatrixMode(GL_MODELVIEW);
// // }

// // // ================================================================
// // //  Kinematics update  (called every idle frame)
// // // ================================================================
// // static void updateKinematics()
// // {
// //     // arc-length speeds → heading rate and linear speed
// //     float arc   = (leftSpeed + rightSpeed) * 0.5f;
// //     float omega = (rightSpeed - leftSpeed) / WHEELBASE;

// //     rPhi    += omega;
// //     rTheta  += arc * cosf(rPhi) / CYL_R;
// //     rHeight += arc * sinf(rPhi);

// //     // wrap theta 0..2π
// //     if (rTheta >  (float)M_PI) rTheta -= 2.f * (float)M_PI;
// //     if (rTheta < -(float)M_PI) rTheta += 2.f * (float)M_PI;

// //     // clamp height so robot stays on cylinder
// //     if (rHeight < H_MIN) { rHeight = H_MIN; }
// //     if (rHeight > H_MAX) { rHeight = H_MAX; }

// //     // visual wheel spin  (deg = arc / wheel_R * (180/π))
// //     float toDeg = 180.f / (float)M_PI;
// //     wheelAngleL += leftSpeed  / WHEEL_R * toDeg;
// //     wheelAngleR += rightSpeed / WHEEL_R * toDeg;
// // }

// // // ================================================================
// // //  Build robot model matrix  (4-step surface attachment)
// // //
// // //  Step 1: Translate to surface point
// // //  Step 2: RotateY(theta)   → outward normal faces away from axis
// // //  Step 3: RotateZ(-90°)    → robot stands on surface (base inward)
// // //  Step 4: RotateY(phi)     → heading in tangent plane
// // // ================================================================
// // static void applyRobotTransform()
// // {
// //     float toDeg = 180.f / (float)M_PI;

// //     // Step 1 – surface position
// //     float sx = CYL_R * cosf(rTheta);
// //     float sy = rHeight;
// //     float sz = CYL_R * sinf(rTheta);
// //     glTranslatef(sx, sy, sz);

// //     // Step 2 – align outward normal (rotate around Y by theta)
// //     glRotatef(rTheta * toDeg, 0, 1, 0);

// //     // Step 3 – tilt robot to stand on surface (-90° around Z)
// //     glRotatef(-90.f, 0, 0, 1);

// //     // Step 4 – heading in tangent plane (rotate around X = outward normal)
// //     glRotatef(rPhi * toDeg, 1, 0, 0);
// // }

// // // ================================================================
// // //  Display
// // // ================================================================
// // static void display()
// // {
// //     glClearColor(0.08f, 0.09f, 0.12f, 1.f);
// //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// //     // ── projection ──────────────────────────────────────────
// //     glMatrixMode(GL_PROJECTION);
// //     glLoadIdentity();
// //     gluPerspective(45.0, (double)W / H, 0.1, 300.0);

// //     // ── camera (spherical orbit) ────────────────────────────
// //     glMatrixMode(GL_MODELVIEW);
// //     glLoadIdentity();
// //     float radT = camTheta * (float)M_PI / 180.f;
// //     float radP = camPhi   * (float)M_PI / 180.f;
// //     float cx = camDist * cosf(radP) * sinf(radT);
// //     float cy = camDist * sinf(radP);
// //     float cz = camDist * cosf(radP) * cosf(radT);
// //     gluLookAt(cx, cy, cz,  0, 0, 0,  0, 1, 0);

// //     // ── lighting ────────────────────────────────────────────
// //     glEnable(GL_LIGHTING);
// //     glEnable(GL_LIGHT0);
// //     glEnable(GL_LIGHT1);
// //     glEnable(GL_DEPTH_TEST);
// //     glEnable(GL_NORMALIZE);

// //     GLfloat amb[]  = {0.20f, 0.20f, 0.20f, 1.f};
// //     GLfloat dif[]  = {1.0f,  0.95f, 0.85f, 1.f};
// //     GLfloat pos0[] = {15.f,  20.f,  15.f,  1.f};
// //     GLfloat pos1[] = {-10.f, 10.f, -10.f,  1.f};
// //     GLfloat dif1[] = {0.3f,  0.4f,  0.6f,  1.f};
// //     glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
// //     glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
// //     glLightfv(GL_LIGHT0, GL_POSITION, pos0);
// //     glLightfv(GL_LIGHT1, GL_DIFFUSE,  dif1);
// //     glLightfv(GL_LIGHT1, GL_POSITION, pos1);

// //     GLfloat spec[]  = {0.4f, 0.4f, 0.4f, 1.f};
// //     GLfloat shine[] = {40.f};
// //     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spec);
// //     glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
// //     glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
// //     glEnable(GL_COLOR_MATERIAL);

// //     // ── world axes at origin ─────────────────────────────────
// //     drawAxes(4.0f);

// //     // ── XZ grid (below cylinder) ─────────────────────────────
// //     drawGrid();

// //     // ── hollow cylinder ──────────────────────────────────────
// //     drawHollowCylinder();

// //     // ── robot  (surface-attached transform) ──────────────────
// //     glPushMatrix();
// //     applyRobotTransform();
// //     drawRobot();
// //     glPopMatrix();

// //     // ── HUD ──────────────────────────────────────────────────
// //     float toDeg = 180.f / (float)M_PI;
// //     char buf[256];
// //     renderText(10, H-22,
// //         "Robot on Cylinder  |  CylR=5.0  BodyH=2  WheelR=0.5",
// //         0.9f, 0.9f, 0.9f);
// //     renderText(10, H-42,
// //         "W/S=fwd/back  A/D=turn  Q/E=yaw  R=reset  Drag=orbit  Scroll=zoom",
// //         0.55f, 0.55f, 0.55f);

// //     snprintf(buf, sizeof(buf),
// //         "theta=%.2f deg  height=%.2f  phi=%.1f deg  vL=%.3f  vR=%.3f",
// //         rTheta * toDeg, rHeight, rPhi * toDeg, leftSpeed, rightSpeed);
// //     renderText(10, 10, buf, 0.5f, 1.0f, 0.6f);

// //     glutSwapBuffers();
// // }

// // // ================================================================
// // //  Idle  – kinematics + redraw
// // // ================================================================
// // static void idle()
// // {
// //     updateKinematics();
// //     glutPostRedisplay();
// // }

// // // ================================================================
// // //  Keyboard
// // // ================================================================
// // static void keyboard(unsigned char key, int, int)
// // {
// //     const float SPEED  = 0.04f;   // arc-length per frame

// //     switch (key) {
// //     // ── driving ─────────────────────────────────────────────
// //     case 'w': case 'W':
// //         leftSpeed  = SPEED;
// //         rightSpeed = SPEED;
// //         break;
// //     case 's': case 'S':
// //         leftSpeed  = -SPEED;
// //         rightSpeed = -SPEED;
// //         break;
// //     case 'a': case 'A':   // turn left: right wheel faster
// //         leftSpeed  =  SPEED * 0.3f;
// //         rightSpeed =  SPEED;
// //         break;
// //     case 'd': case 'D':   // turn right: left wheel faster
// //         leftSpeed  =  SPEED;
// //         rightSpeed =  SPEED * 0.3f;
// //         break;
// //     case 'q': case 'Q':   // yaw left in place
// //         leftSpeed  = -SPEED * 0.5f;
// //         rightSpeed =  SPEED * 0.5f;
// //         break;
// //     case 'e': case 'E':   // yaw right in place
// //         leftSpeed  =  SPEED * 0.5f;
// //         rightSpeed = -SPEED * 0.5f;
// //         break;

// //     // ── reset ────────────────────────────────────────────────
// //     case 'r': case 'R':
// //         rTheta = rHeight = rPhi = 0.f;
// //         leftSpeed = rightSpeed = 0.f;
// //         wheelAngleL = wheelAngleR = 0.f;
// //         camTheta = 40.f; camPhi = 20.f; camDist = 35.f;
// //         break;

// //     case 27: exit(0);
// //     default: break;
// //     }
// //     glutPostRedisplay();
// // }

// // static void keyboardUp(unsigned char key, int, int)
// // {
// //     // stop when key released
// //     switch (key) {
// //     case 'w': case 'W':
// //     case 's': case 'S':
// //     case 'a': case 'A':
// //     case 'd': case 'D':
// //     case 'q': case 'Q':
// //     case 'e': case 'E':
// //         leftSpeed = rightSpeed = 0.f;
// //         break;
// //     default: break;
// //     }
// // }

// // // ================================================================
// // //  Mouse
// // // ================================================================
// // static void mouse(int btn, int state, int x, int y)
// // {
// //     if (btn == GLUT_LEFT_BUTTON) {
// //         dragging = (state == GLUT_DOWN);
// //         lastX = x; lastY = y;
// //     }
// //     if (btn == 3) { camDist -= 0.8f; if (camDist < 3.f)  camDist = 3.f;  }
// //     if (btn == 4) { camDist += 0.8f; if (camDist > 120.f) camDist = 120.f; }
// //     glutPostRedisplay();
// // }

// // static void motion(int x, int y)
// // {
// //     if (dragging) {
// //         camTheta -= (x - lastX) * 0.5f;
// //         camPhi   += (y - lastY) * 0.5f;
// //         if (camPhi >  89.f) camPhi =  89.f;
// //         if (camPhi < -89.f) camPhi = -89.f;
// //         lastX = x; lastY = y;
// //         glutPostRedisplay();
// //     }
// // }

// // static void reshape(int w, int h)
// // {
// //     W = w; H = (h > 0 ? h : 1);
// //     glViewport(0, 0, W, H);
// // }

// // // ================================================================
// // //  Main
// // // ================================================================
// // int main(int argc, char** argv)
// // {
// //     glutInit(&argc, argv);
// //     glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
// //     glutInitWindowSize(W, H);
// //     glutCreateWindow(TITLE);

// //     glEnable(GL_DEPTH_TEST);
// //     glEnable(GL_BLEND);
// //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// //     glutDisplayFunc(display);
// //     glutReshapeFunc(reshape);
// //     glutKeyboardFunc(keyboard);
// //     glutKeyboardUpFunc(keyboardUp);   // stop on key release
// //     glutMouseFunc(mouse);
// //     glutMotionFunc(motion);
// //     glutIdleFunc(idle);

// //     glutMainLoop();
// //     return 0;
// // }

// // ================================================================
// //  Differential-Drive Robot on Cylinder Surface – OpenGL / GLUT
// //
// //  KINEMATICS (tangent-plane diff-drive on cylinder):
// //
// //    State  :  theta  – angle around cylinder Y-axis   (rad)
// //              height – position along   cylinder Y-axis
// //              phi    – heading in tangent plane        (rad)
// //                       phi=0  → circumferential (+theta)
// //                       phi=π/2→ axial           (+height)
// //
// //    Update (dt = 1 frame):
// //      arc   = (vL + vR) * 0.5          [arc-length / frame]
// //      omega = (vR - vL) / WHEELBASE    [heading rate rad/frame]
// //      phi  += omega
// //      theta+= arc * cos(phi) / CYL_R   [angle  from arc-len]
// //      height+= arc * sin(phi)          [axial  from arc-len]
// //
// //    Model matrix (4 steps):
// //      1. Translate to surface point (R·cosθ, h, R·sinθ)
// //      2. RotateY(θ)        – align outward normal
// //      3. RotateZ(-90°)     – tilt robot to STAND on surface
// //      4. RotateY(φ)        – heading in tangent plane
// //
// //  Controls:
// //    W/S      – both wheels forward / backward
// //    A/D      – left wheel slower / faster  (turn)
// //    Q/E      – yaw in place
// //    R        – reset
// //    ESC      – quit
// //    Mouse drag  – orbit camera
// //    Scroll      – zoom
// // ================================================================

// #include <GL/glut.h>
// #include <cmath>
// #include <string>
// #include <cstdio>

// // ── window ──────────────────────────────────────────────────
// static int  W = 1280, H = 800;
// static const char* TITLE = "Robot on Cylinder Surface";

// // ── camera ──────────────────────────────────────────────────
// static float camTheta =  40.0f;   // azimuth  deg
// static float camPhi   =  20.0f;   // elevation deg
// static float camDist  =  35.0f;
// static int   lastX = 0, lastY = 0;
// static bool  dragging = false;

// // ────────────────────────────────────────────────────────────
// //  Cylinder geometry
// // ────────────────────────────────────────────────────────────
// static const float CYL_R       = 5.0f;    // outer radius
// static const float CYL_INNER_R = 4.7f;    // inner radius
// static const float CYL_H       = 15.0f;   // total height
// static const int   CYL_SEG     = 80;      // circumference segments
// static const int   CYL_VSTEP   = 1;       // vertical steps

// // ────────────────────────────────────────────────────────────
// //  Robot geometry  (same as before)
// // ────────────────────────────────────────────────────────────
// static const float BODY_W       = 2.0f;
// static const float BODY_H       = 2.0f;
// static const float BODY_D       = 2.0f;
// static const float WHEEL_R      = 0.5f;
// static const float WHEEL_T      = 0.25f;
// static const int   WHEEL_SLICES = 40;
// static const float WHEEL_AXLE_X = BODY_W * 0.5f + WHEEL_T * 0.5f;
// static const float WHEELBASE    = BODY_W + WHEEL_T;   // distance L<->R wheel

// // ────────────────────────────────────────────────────────────
// //  Robot state  (cylinder-surface kinematics)
// // ────────────────────────────────────────────────────────────
// static float rTheta  = 0.0f;    // angle around cylinder  (rad)
// static float rHeight = 0.0f;    // position along Y axis
// static float rPhi    = 0.0f;    // heading in tangent plane (rad)
//                                  //  0 = circumferential, π/2 = axial

// static float leftSpeed  = 0.0f; // arc-length per frame, left  wheel
// static float rightSpeed = 0.0f; // arc-length per frame, right wheel

// static float wheelAngleL = 0.0f; // visual spin  (deg)
// static float wheelAngleR = 0.0f;

// // Height clamp so robot stays on cylinder
// static const float H_MIN = -CYL_H * 0.5f + WHEEL_R + 0.05f;
// static const float H_MAX =  CYL_H * 0.5f - WHEEL_R - 0.05f;

// // ── grid ────────────────────────────────────────────────────
// static const int   GRID_HALF = 20;
// static const float GRID_STEP = 2.0f;

// // ================================================================
// //  helpers
// // ================================================================
// static void setColor(float r, float g, float b, float a = 1.f)
// { glColor4f(r, g, b, a); }

// // Cylinder (open barrel) aligned along X, centred at origin
// static void drawCylinderX(float radius, float length, int slices)
// {
//     float half = length * 0.5f;
//     glBegin(GL_TRIANGLE_STRIP);
//     for (int i = 0; i <= slices; ++i) {
//         float ang = (float)i / slices * 2.f * (float)M_PI;
//         float yy  = radius * cosf(ang);
//         float zz  = radius * sinf(ang);
//         glNormal3f(0, cosf(ang), sinf(ang));
//         glVertex3f(-half, yy, zz);
//         glVertex3f( half, yy, zz);
//     }
//     glEnd();
// }

// static void drawDiskX(float radius, float x, int slices, bool flip)
// {
//     glBegin(GL_TRIANGLE_FAN);
//     glNormal3f(flip ? -1.f : 1.f, 0, 0);
//     glVertex3f(x, 0, 0);
//     int n = flip ? slices : 0;
//     int d = flip ? -1 : 1;
//     for (int i = 0; i <= slices; ++i) {
//         int  idx = n + d * i;
//         float ang = (float)idx / slices * 2.f * (float)M_PI;
//         glVertex3f(x, radius * cosf(ang), radius * sinf(ang));
//     }
//     glEnd();
// }

// static void drawWheel(float radius, float thickness, int slices)
// {
//     drawCylinderX(radius, thickness, slices);
//     drawDiskX(radius, -thickness * 0.5f, slices, true);
//     drawDiskX(radius,  thickness * 0.5f, slices, false);
// }

// // ================================================================
// //  Axis arrows  (TF-style)   X=red  Y=green  Z=blue
// // ================================================================
// static void drawAxisArrow(float len, float headLen, float r)
// {
//     float bodyLen = len - headLen;
//     GLUquadric* q = gluNewQuadric();
//     gluCylinder(q, r, r, bodyLen, 10, 1);
//     glPushMatrix();
//       glTranslatef(0, 0, bodyLen);
//       gluCylinder(q, r * 2.5f, 0, headLen, 10, 1);
//     glPopMatrix();
//     gluDeleteQuadric(q);
// }

// static void drawAxes(float scale)
// {
//     glPushAttrib(GL_LIGHTING_BIT);
//     glDisable(GL_LIGHTING);

//     setColor(1, 0.1f, 0.1f);
//     glPushMatrix(); glRotatef(90, 0,1,0);
//     drawAxisArrow(scale, scale*0.25f, scale*0.03f);
//     glPopMatrix();

//     setColor(0.1f, 1, 0.1f);
//     glPushMatrix(); glRotatef(-90,1,0,0);
//     drawAxisArrow(scale, scale*0.25f, scale*0.03f);
//     glPopMatrix();

//     setColor(0.2f, 0.4f, 1);
//     drawAxisArrow(scale, scale*0.25f, scale*0.03f);

//     glPopAttrib();
// }

// // ================================================================
// //  Hollow Cylinder  (the track the robot drives on)
// // ================================================================
// static void drawHollowCylinder()
// {
//     const float yBot   = -CYL_H * 0.5f;
//     const float yTop   =  CYL_H * 0.5f;
//     const float TWO_PI =  2.f * (float)M_PI;

//     // ── Outer wall ───────────────────────────────────────────
//     setColor(0.18f, 0.22f, 0.32f, 1.f);
//     glBegin(GL_TRIANGLE_STRIP);
//     for (int i = 0; i <= CYL_SEG; ++i) {
//         float a  = (float)i / CYL_SEG * TWO_PI;
//         float c  = cosf(a), s = sinf(a);
//         glNormal3f(c, 0, s);
//         glVertex3f(CYL_R * c, yBot, CYL_R * s);
//         glVertex3f(CYL_R * c, yTop, CYL_R * s);
//     }
//     glEnd();

//     // ── Inner wall ───────────────────────────────────────────
//     setColor(0.12f, 0.15f, 0.22f, 1.f);
//     glBegin(GL_TRIANGLE_STRIP);
//     for (int i = 0; i <= CYL_SEG; ++i) {
//         float a  = (float)i / CYL_SEG * TWO_PI;
//         float c  = cosf(a), s = sinf(a);
//         glNormal3f(-c, 0, -s);
//         glVertex3f(CYL_INNER_R * c, yTop, CYL_INNER_R * s);
//         glVertex3f(CYL_INNER_R * c, yBot, CYL_INNER_R * s);
//     }
//     glEnd();

//     // ── Top annulus ──────────────────────────────────────────
//     setColor(0.25f, 0.30f, 0.42f, 1.f);
//     glBegin(GL_TRIANGLE_STRIP);
//     for (int i = 0; i <= CYL_SEG; ++i) {
//         float a = (float)i / CYL_SEG * TWO_PI;
//         float c = cosf(a), s = sinf(a);
//         glNormal3f(0, 1, 0);
//         glVertex3f(CYL_INNER_R * c, yTop, CYL_INNER_R * s);
//         glVertex3f(CYL_R       * c, yTop, CYL_R       * s);
//     }
//     glEnd();

//     // ── Bottom annulus ───────────────────────────────────────
//     setColor(0.25f, 0.30f, 0.42f, 1.f);
//     glBegin(GL_TRIANGLE_STRIP);
//     for (int i = 0; i <= CYL_SEG; ++i) {
//         float a = (float)i / CYL_SEG * TWO_PI;
//         float c = cosf(a), s = sinf(a);
//         glNormal3f(0, -1, 0);
//         glVertex3f(CYL_R       * c, yBot, CYL_R       * s);
//         glVertex3f(CYL_INNER_R * c, yBot, CYL_INNER_R * s);
//     }
//     glEnd();

//     // ── Longitude lines on outer surface (visual guide) ──────
//     glPushAttrib(GL_LIGHTING_BIT);
//     glDisable(GL_LIGHTING);
//     setColor(0.30f, 0.35f, 0.50f, 0.6f);
//     glLineWidth(1.0f);
//     int gridLines = 16;
//     for (int i = 0; i < gridLines; ++i) {
//         float a = (float)i / gridLines * TWO_PI;
//         float c = cosf(a), s = sinf(a);
//         glBegin(GL_LINES);
//         glVertex3f((CYL_R + 0.01f)*c, yBot, (CYL_R + 0.01f)*s);
//         glVertex3f((CYL_R + 0.01f)*c, yTop, (CYL_R + 0.01f)*s);
//         glEnd();
//     }
//     // Latitude rings
//     int rings = 8;
//     for (int j = 0; j <= rings; ++j) {
//         float yy = yBot + (float)j / rings * CYL_H;
//         glBegin(GL_LINE_LOOP);
//         for (int i = 0; i <= CYL_SEG; ++i) {
//             float a = (float)i / CYL_SEG * TWO_PI;
//             glVertex3f((CYL_R+0.01f)*cosf(a), yy, (CYL_R+0.01f)*sinf(a));
//         }
//         glEnd();
//     }
//     glPopAttrib();
// }

// // ================================================================
// //  Robot body – uniform dark cuboid  (matches reference screenshot)
// //  Frame origin = geometric centre of the body.
// //  +X = along wheel axle (left/right)
// //  +Y = outward from tank surface
// //  +Z = forward / backward along tank wall
// // ================================================================
// // static void drawRobotBody()
// // {
// //     float bx = BODY_W * 0.5f;   // half along axle   (X)
// //     float by = BODY_H * 0.5f;   // half outward       (Y)
// //     float bz = BODY_D * 0.5f;   // half fwd/back      (Z)

// //     setColor(0.18f, 0.18f, 0.18f);   // uniform dark charcoal

// //     glBegin(GL_QUADS);
// //     // +X
// //     // glNormal3f( 1,0,0);
// //     // glVertex3f( bx,-by,-bz); glVertex3f( bx, by,-bz);
// //     // glVertex3f( bx, by, bz); glVertex3f( bx,-by, bz);
// //     // // -X
// //     // glNormal3f(-1,0,0);
// //     // glVertex3f(-bx,-by, bz); glVertex3f(-bx, by, bz);
// //     // glVertex3f(-bx, by,-bz); glVertex3f(-bx,-by,-bz);
// //     // // +Y  (outward face)
// //     // glNormal3f(0, 1,0);
// //     // glVertex3f(-bx, by,-bz); glVertex3f(-bx, by, bz);
// //     // glVertex3f( bx, by, bz); glVertex3f( bx, by,-bz);
// //     // // -Y  (surface-contact face)
// //     // glNormal3f(0,-1,0);
// //     // glVertex3f(-bx,-by, bz); glVertex3f(-bx,-by,-bz);
// //     // glVertex3f( bx,-by,-bz); glVertex3f( bx,-by, bz);
// //     // // +Z  (front)
// //     // glNormal3f(0,0, 1);
// //     // glVertex3f(-bx,-by, bz); glVertex3f( bx,-by, bz);
// //     // glVertex3f( bx, by, bz); glVertex3f(-bx, by, bz);
// //     // -Z  (back)
// //     glNormal3f(0,0,-1);
// //     glVertex3f( bx,-by,-bz); glVertex3f(-bx,-by,-bz);
// //     glVertex3f(-bx, by,-bz); glVertex3f( bx, by,-bz);
// //     glEnd();
// // }

// static void drawRobotBody()
// {
//     float w = BODY_W * 0.5f;
//     float h = BODY_H * 0.5f;
//     float d = BODY_D * 0.5f;

//     // offset so bottom of body rests on wheel centre height (y = WHEEL_RADIUS)
//     float baseY = WHEEL_R;

//     glPushMatrix();
//     glTranslatef(0, baseY + h, 0);  // centre of body in world Y

//     // // front face  +Z  (orange)
//     // setColor(1.0f, 0.55f, 0.1f);
//     // glBegin(GL_QUADS);
//     // glNormal3f( 0,  0,  1);
//     // glVertex3f(-w, -h,  d);
//     // glVertex3f( w, -h,  d);
//     // glVertex3f( w,  h,  d);
//     // glVertex3f(-w,  h,  d);
//     // glEnd();

//     // // back face  –Z  (dark orange)
//     // setColor(0.7f, 0.35f, 0.05f);
//     // glBegin(GL_QUADS);
//     // glNormal3f( 0,  0, -1);
//     // glVertex3f( w, -h, -d);
//     // glVertex3f(-w, -h, -d);
//     // glVertex3f(-w,  h, -d);
//     // glVertex3f( w,  h, -d);
//     // glEnd();

//     // // left face  –X  (teal)
//     // setColor(0.1f, 0.75f, 0.75f);
//     // glBegin(GL_QUADS);
//     // glNormal3f(-1,  0,  0);
//     // glVertex3f(-w, -h, -d);
//     // glVertex3f(-w, -h,  d);
//     // glVertex3f(-w,  h,  d);
//     // glVertex3f(-w,  h, -d);
//     // glEnd();

//     // // right face  +X  (teal-dark)
//     // setColor(0.05f, 0.5f, 0.5f);
//     // glBegin(GL_QUADS);
//     // glNormal3f( 1,  0,  0);
//     // glVertex3f( w, -h,  d);
//     // glVertex3f( w, -h, -d);
//     // glVertex3f( w,  h, -d);
//     // glVertex3f( w,  h,  d);
//     // glEnd();

//     // // top face  +Y  (light grey)
//     // setColor(0.85f, 0.85f, 0.85f);
//     // glBegin(GL_QUADS);
//     // glNormal3f( 0,  1,  0);
//     // glVertex3f(-w,  h,  d);
//     // glVertex3f( w,  h,  d);
//     // glVertex3f( w,  h, -d);
//     // glVertex3f(-w,  h, -d);
//     // glEnd();

//     // bottom face  –Y  (dark grey)
//     setColor(0.3f, 0.3f, 0.3f);
//     glBegin(GL_QUADS);
//     glNormal3f( 0, -1,  0);
//     glVertex3f(-w, -h, -d);
//     glVertex3f( w, -h, -d);
//     glVertex3f( w, -h,  d);
//     glVertex3f(-w, -h,  d);
//     glEnd();

   
    
//     glEnd();
//     glPopAttrib();

//     glPopMatrix();
// }


// // ================================================================
// //  Full robot draw  (body + 2 wheels)
// //
// //  Matches reference screenshot:
// //    • uniform dark charcoal cuboid body
// //    • two thick dark cylinders on the ±X axle (left / right)
// //    • wheels protrude slightly beyond the body sides
// //    • TF axes at body centre  (R=red X, G=green Y, B=blue Z)
// //
// //  Wheel axis = X  →  drawCylinderX spins natively around X
// // ================================================================
// static void drawRobot()
// {
//     // ── body ────────────────────────────────────────────────
//     drawRobotBody();

//     // Wheel centres sit flush to body sides (±X) and centred in Y,Z
//     float wheelCentreX = BODY_W * 0.5f + WHEEL_T * 0.5f;

//     // ── left wheel  (–X) ────────────────────────────────────
//     setColor(0.14f, 0.14f, 0.14f);
//     glPushMatrix();
//     glTranslatef(-wheelCentreX, 0.f, 0.f);
//     glRotatef(wheelAngleL, 1, 0, 0);   // spin around X axle
//     drawWheel(WHEEL_R, WHEEL_T, WHEEL_SLICES);
//     glPopMatrix();

//     // ── right wheel  (+X) ───────────────────────────────────
//     setColor(0.14f, 0.14f, 0.14f);
//     glPushMatrix();
//     glTranslatef( wheelCentreX, 0.f, 0.f);
//     glRotatef(wheelAngleR, 1, 0, 0);
//     drawWheel(WHEEL_R, WHEEL_T, WHEEL_SLICES);
//     glPopMatrix();

//     // ── robot TF axes at body centre ────────────────────────
//     //    X=red (axle/left-right)  Y=green (outward)  Z=blue (forward)
//     drawAxes(1.5f);
// }

// // ================================================================
// //  XZ ground grid  (y = -CYL_H/2 - 1, just below cylinder)
// // ================================================================
// static void drawGrid()
// {
//     float yy = -CYL_H * 0.5f - 1.0f;
//     glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
//     glDisable(GL_LIGHTING);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//     glLineWidth(1.0f);
//     glBegin(GL_LINES);
//     for (int i = -GRID_HALF; i <= GRID_HALF; ++i) {
//         float fi  = (float)i * GRID_STEP;
//         bool  maj = (i % 5 == 0);
//         if (maj) glColor4f(0.50f, 0.50f, 0.55f, 0.8f);
//         else     glColor4f(0.30f, 0.30f, 0.35f, 0.4f);
//         glVertex3f(fi, yy, -(float)GRID_HALF * GRID_STEP);
//         glVertex3f(fi, yy,  (float)GRID_HALF * GRID_STEP);
//         glVertex3f(-(float)GRID_HALF * GRID_STEP, yy, fi);
//         glVertex3f( (float)GRID_HALF * GRID_STEP, yy, fi);
//     }
//     glEnd();
//     glPopAttrib();
// }

// // ================================================================
// //  HUD
// // ================================================================
// static void renderText(float x, float y, const std::string& s,
//                         float r, float g, float b)
// {
//     glMatrixMode(GL_PROJECTION);
//     glPushMatrix(); glLoadIdentity();
//     gluOrtho2D(0, W, 0, H);
//     glMatrixMode(GL_MODELVIEW);
//     glPushMatrix(); glLoadIdentity();
//     glDisable(GL_LIGHTING);
//     glColor3f(r, g, b);
//     glRasterPos2f(x, y);
//     for (char c : s) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
//     glEnable(GL_LIGHTING);
//     glPopMatrix();
//     glMatrixMode(GL_PROJECTION);
//     glPopMatrix();
//     glMatrixMode(GL_MODELVIEW);
// }

// // ================================================================
// //  Kinematics update  (called every idle frame)
// // ================================================================
// static void updateKinematics()
// {
//     // arc-length speeds → heading rate and linear speed
//     float arc   = (leftSpeed + rightSpeed) * 0.5f;
//     float omega = (rightSpeed - leftSpeed) / WHEELBASE;

//     rPhi    += omega;
//     rTheta  += arc * cosf(rPhi) / CYL_R;
//     rHeight += arc * sinf(rPhi);

//     // wrap theta 0..2π
//     if (rTheta >  (float)M_PI) rTheta -= 2.f * (float)M_PI;
//     if (rTheta < -(float)M_PI) rTheta += 2.f * (float)M_PI;

//     // clamp height so robot stays on cylinder
//     if (rHeight < H_MIN) { rHeight = H_MIN; }
//     if (rHeight > H_MAX) { rHeight = H_MAX; }

//     // visual wheel spin  (deg = arc / wheel_R * (180/π))
//     float toDeg = 180.f / (float)M_PI;
//     wheelAngleL += leftSpeed  / WHEEL_R * toDeg;
//     wheelAngleR += rightSpeed / WHEEL_R * toDeg;
// }

// // ================================================================
// //  Build robot model matrix  (4-step surface attachment)
// //
// //  Step 1: Translate to surface point
// //  Step 2: RotateY(theta)   → outward normal faces away from axis
// //  Step 3: RotateZ(-90°)    → robot stands on surface (base inward)
// //  Step 4: RotateY(phi)     → heading in tangent plane
// // ================================================================
// static void applyRobotTransform()
// {
//     float toDeg = 180.f / (float)M_PI;

//     // Step 1 – surface position
//     float sx = CYL_R * cosf(rTheta);
//     float sy = rHeight;
//     float sz = CYL_R * sinf(rTheta);
//     glTranslatef(sx, sy, sz);

//     // Step 2 – align outward normal (rotate around Y by theta)
//     glRotatef(rTheta * toDeg, 0, 1, 0);

//     // Step 3 – tilt robot to stand on surface (-90° around Z)
//     glRotatef(-90.f, 0, 0, 1);

//     // Step 4 – heading in tangent plane (rotate around X = outward normal)
//     glRotatef(rPhi * toDeg, 1, 0, 0);
// }

// // ================================================================
// //  Display
// // ================================================================
// static void display()
// {
//     glClearColor(0.08f, 0.09f, 0.12f, 1.f);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     // ── projection ──────────────────────────────────────────
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
//     gluPerspective(45.0, (double)W / H, 0.1, 300.0);

//     // ── camera (spherical orbit) ────────────────────────────
//     glMatrixMode(GL_MODELVIEW);
//     glLoadIdentity();
//     float radT = camTheta * (float)M_PI / 180.f;
//     float radP = camPhi   * (float)M_PI / 180.f;
//     float cx = camDist * cosf(radP) * sinf(radT);
//     float cy = camDist * sinf(radP);
//     float cz = camDist * cosf(radP) * cosf(radT);
//     gluLookAt(cx, cy, cz,  0, 0, 0,  0, 1, 0);

//     // ── lighting ────────────────────────────────────────────
//     glEnable(GL_LIGHTING);
//     glEnable(GL_LIGHT0);
//     glEnable(GL_LIGHT1);
//     glEnable(GL_DEPTH_TEST);
//     glEnable(GL_NORMALIZE);

//     GLfloat amb[]  = {0.20f, 0.20f, 0.20f, 1.f};
//     GLfloat dif[]  = {1.0f,  0.95f, 0.85f, 1.f};
//     GLfloat pos0[] = {15.f,  20.f,  15.f,  1.f};
//     GLfloat pos1[] = {-10.f, 10.f, -10.f,  1.f};
//     GLfloat dif1[] = {0.3f,  0.4f,  0.6f,  1.f};
//     glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
//     glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif);
//     glLightfv(GL_LIGHT0, GL_POSITION, pos0);
//     glLightfv(GL_LIGHT1, GL_DIFFUSE,  dif1);
//     glLightfv(GL_LIGHT1, GL_POSITION, pos1);

//     GLfloat spec[]  = {0.4f, 0.4f, 0.4f, 1.f};
//     GLfloat shine[] = {40.f};
//     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spec);
//     glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
//     glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//     glEnable(GL_COLOR_MATERIAL);

//     // ── world axes at origin ─────────────────────────────────
//     drawAxes(4.0f);

//     // ── XZ grid (below cylinder) ─────────────────────────────
//     drawGrid();

//     // ── hollow cylinder ──────────────────────────────────────
//     drawHollowCylinder();

//     // ── robot  (surface-attached transform) ──────────────────
//     glPushMatrix();
//     applyRobotTransform();
//     drawRobot();
//     glPopMatrix();

//     // ── HUD ──────────────────────────────────────────────────
//     float toDeg = 180.f / (float)M_PI;
//     char buf[256];
//     renderText(10, H-22,
//         "Robot on Cylinder  |  CylR=5.0  BodyH=2  WheelR=0.5",
//         0.9f, 0.9f, 0.9f);
//     renderText(10, H-42,
//         "W/S=fwd/back  A/D=turn  Q/E=yaw  R=reset  Drag=orbit  Scroll=zoom",
//         0.55f, 0.55f, 0.55f);

//     snprintf(buf, sizeof(buf),
//         "theta=%.2f deg  height=%.2f  phi=%.1f deg  vL=%.3f  vR=%.3f",
//         rTheta * toDeg, rHeight, rPhi * toDeg, leftSpeed, rightSpeed);
//     renderText(10, 10, buf, 0.5f, 1.0f, 0.6f);

//     glutSwapBuffers();
// }

// // ================================================================
// //  Idle  – kinematics + redraw
// // ================================================================
// static void idle()
// {
//     updateKinematics();
//     glutPostRedisplay();
// }

// // ================================================================
// //  Keyboard
// // ================================================================
// static void keyboard(unsigned char key, int, int)
// {
//     const float SPEED  = 0.04f;   // arc-length per frame

//     switch (key) {
//     // ── driving ─────────────────────────────────────────────
//     case 'w': case 'W':
//         leftSpeed  = SPEED;
//         rightSpeed = SPEED;
//         break;
//     case 's': case 'S':
//         leftSpeed  = -SPEED;
//         rightSpeed = -SPEED;
//         break;
//     case 'a': case 'A':   // turn left: right wheel faster
//         leftSpeed  =  SPEED * 0.3f;
//         rightSpeed =  SPEED;
//         break;
//     case 'd': case 'D':   // turn right: left wheel faster
//         leftSpeed  =  SPEED;
//         rightSpeed =  SPEED * 0.3f;
//         break;
//     case 'q': case 'Q':   // yaw left in place
//         leftSpeed  = -SPEED * 0.5f;
//         rightSpeed =  SPEED * 0.5f;
//         break;
//     case 'e': case 'E':   // yaw right in place
//         leftSpeed  =  SPEED * 0.5f;
//         rightSpeed = -SPEED * 0.5f;
//         break;

//     // ── reset ────────────────────────────────────────────────
//     case 'r': case 'R':
//         rTheta = rHeight = rPhi = 0.f;
//         leftSpeed = rightSpeed = 0.f;
//         wheelAngleL = wheelAngleR = 0.f;
//         camTheta = 40.f; camPhi = 20.f; camDist = 35.f;
//         break;

//     case 27: exit(0);
//     default: break;
//     }
//     glutPostRedisplay();
// }

// static void keyboardUp(unsigned char key, int, int)
// {
//     // stop when key released
//     switch (key) {
//     case 'w': case 'W':
//     case 's': case 'S':
//     case 'a': case 'A':
//     case 'd': case 'D':
//     case 'q': case 'Q':
//     case 'e': case 'E':
//         leftSpeed = rightSpeed = 0.f;
//         break;
//     default: break;
//     }
// }

// // ================================================================
// //  Mouse
// // ================================================================
// static void mouse(int btn, int state, int x, int y)
// {
//     if (btn == GLUT_LEFT_BUTTON) {
//         dragging = (state == GLUT_DOWN);
//         lastX = x; lastY = y;
//     }
//     if (btn == 3) { camDist -= 0.8f; if (camDist < 3.f)  camDist = 3.f;  }
//     if (btn == 4) { camDist += 0.8f; if (camDist > 120.f) camDist = 120.f; }
//     glutPostRedisplay();
// }

// static void motion(int x, int y)
// {
//     if (dragging) {
//         camTheta -= (x - lastX) * 0.5f;
//         camPhi   += (y - lastY) * 0.5f;
//         if (camPhi >  89.f) camPhi =  89.f;
//         if (camPhi < -89.f) camPhi = -89.f;
//         lastX = x; lastY = y;
//         glutPostRedisplay();
//     }
// }

// static void reshape(int w, int h)
// {
//     W = w; H = (h > 0 ? h : 1);
//     glViewport(0, 0, W, H);
// }

// // ================================================================
// //  Main
// // ================================================================
// int main(int argc, char** argv)
// {
//     glutInit(&argc, argv);
//     glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//     glutInitWindowSize(W, H);
//     glutCreateWindow(TITLE);

//     glEnable(GL_DEPTH_TEST);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     glutDisplayFunc(display);
//     glutReshapeFunc(reshape);
//     glutKeyboardFunc(keyboard);
//     glutKeyboardUpFunc(keyboardUp);   // stop on key release
//     glutMouseFunc(mouse);
//     glutMotionFunc(motion);
//     glutIdleFunc(idle);

//     glutMainLoop();
//     return 0;
// }

// ============================================================
//  Wheel with RIM + TYRE rolling on outside of cylinder
//
//  Wheel structure:
//    TYRE – outer ring (radius TYRE_R) – contacts tank surface
//    RIM  – inner hub  (radius RIM_R)  – never touches tank
//
//  Contact constraint:
//    wheel centre = CYL_R + TYRE_R  from cylinder axis
//
//  Motion constraint:
//    wheel spins around Z axis only
//    rolling without slip:
//      dTheta = (TYRE_R / CYL_R) * dSpin_rad
//    → clockwise spin  = moves anticlockwise around tank
//    → anticlockwise   = moves clockwise
//
//  Controls:
//    W / S   – spin wheel (rolls around tank)
//    UP/DOWN – slide up / down  (pure translation along Y)
//    R       – reset
//    Drag    – orbit camera
//    Scroll  – zoom
// ============================================================
#include <GL/glut.h>
#include <cmath>
#include <cstdio>

static int W = 1000, H = 720;

// ── camera ───────────────────────────────────────────────────
static float camTheta = 40.f, camPhi = 20.f, camDist = 22.f;
static int   lastX = 0, lastY = 0;
static bool  dragging = false;

// ── tank ─────────────────────────────────────────────────────
static const float CYL_R   = 5.0f;
static const float CYL_H   = 10.0f;
static const int   CYL_SEG = 72;

// ── wheel parts ──────────────────────────────────────────────
//   TYRE : outer rubber ring  – contacts tank
//   RIM  : inner metal hub    – floats clear of tank
static const float TYRE_R  = 0.55f;   // tyre outer radius
static const float TYRE_T  = 0.40f;   // tyre thickness (width along axle Z)
static const float RIM_R   = 0.30f;   // rim radius  (< TYRE_R)
static const float RIM_T   = 0.30f;   // rim thickness (narrower than tyre)
static const int   SEG     = 48;

// ── wheel state ──────────────────────────────────────────────
static float wTheta  = 0.0f;   // azimuth of wheel around tank (rad)
static float wHeight = 0.0f;   // height along tank Y axis
static float wSpin   = 0.0f;   // spin around Z axis (deg)
static float wSpeed  = 0.0f;   // spin rate (deg/frame) – positive = clockwise

// ── geometry helpers ─────────────────────────────────────────
static void col(float r, float g, float b){ glColor3f(r,g,b); }

// Solid cylinder barrel + caps, axis = Z, centred at origin
static void drawCylZ(float r, float halfT, int seg)
{
    // barrel
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= seg; i++){
        float a = (float)i/seg * 2.f*(float)M_PI;
        float c = cosf(a), s = sinf(a);
        glNormal3f(c, s, 0);
        glVertex3f(r*c, r*s, -halfT);
        glVertex3f(r*c, r*s,  halfT);
    }
    glEnd();
    // caps
    for(int side = -1; side <= 1; side += 2){
        float zz = side * halfT;
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, (float)side);
        glVertex3f(0, 0, zz);
        for(int i = 0; i <= seg; i++){
            float a = (float)(side * i)/seg * 2.f*(float)M_PI;
            glVertex3f(r*cosf(a), r*sinf(a), zz);
        }
        glEnd();
    }
}

// Torus-like tyre cross section drawn as a thick ring
// Inner radius = RIM_R, outer radius = TYRE_R, width = TYRE_T
// Drawn as: outer barrel + inner barrel (reversed) + two annular caps
static void drawTyre(float innerR, float outerR, float halfT, int seg)
{
    // outer barrel
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= seg; i++){
        float a = (float)i/seg * 2.f*(float)M_PI;
        float c = cosf(a), s = sinf(a);
        glNormal3f(c, s, 0);
        glVertex3f(outerR*c, outerR*s, -halfT);
        glVertex3f(outerR*c, outerR*s,  halfT);
    }
    glEnd();
    // inner barrel (normal points inward)
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= seg; i++){
        float a = (float)i/seg * 2.f*(float)M_PI;
        float c = cosf(a), s = sinf(a);
        glNormal3f(-c, -s, 0);
        glVertex3f(innerR*c, innerR*s,  halfT);
        glVertex3f(innerR*c, innerR*s, -halfT);
    }
    glEnd();
    // two annular caps (+Z and -Z)
    for(int side = -1; side <= 1; side += 2){
        float zz = side * halfT;
        glBegin(GL_TRIANGLE_STRIP);
        glNormal3f(0, 0, (float)side);
        for(int i = 0; i <= seg; i++){
            float a = (float)(side * i)/seg * 2.f*(float)M_PI;
            float c = cosf(a), s = sinf(a);
            glVertex3f(outerR*c, outerR*s, zz);
            glVertex3f(innerR*c, innerR*s, zz);
        }
        glEnd();
    }
}

// ── axis arrows ──────────────────────────────────────────────
static void drawArrow(float len)
{
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.035f, 0.035f, len*0.78f, 8, 1);
    glPushMatrix();
      glTranslatef(0, 0, len*0.78f);
      gluCylinder(q, 0.09f, 0.0f, len*0.22f, 8, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}
static void drawAxes(float s)
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    col(1.f, 0.15f, 0.15f);
    glPushMatrix(); glRotatef(90,0,1,0); drawArrow(s); glPopMatrix();
    col(0.15f, 1.f, 0.15f);
    glPushMatrix(); glRotatef(-90,1,0,0); drawArrow(s); glPopMatrix();
    col(0.2f, 0.45f, 1.f);
    drawArrow(s);
    glPopAttrib();
}

// ── floor grid ───────────────────────────────────────────────
static void drawGrid()
{
    float y = -CYL_H*0.5f - 0.6f;
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for(int i = -14; i <= 14; i++){
        float f = (float)i;
        bool maj = (i % 4 == 0);
        if(maj) glColor4f(.50f,.50f,.52f,.8f);
        else    glColor4f(.28f,.28f,.30f,.4f);
        glVertex3f(f,y,-14); glVertex3f(f,y, 14);
        glVertex3f(-14,y,f); glVertex3f( 14,y,f);
    }
    glEnd();
    glPopAttrib();
}

// ── tank ─────────────────────────────────────────────────────
static void drawTank()
{
    float yB = -CYL_H*0.5f, yT = CYL_H*0.5f;

    // outer wall
    col(0.20f, 0.26f, 0.36f);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= CYL_SEG; i++){
        float a = (float)i/CYL_SEG * 2.f*(float)M_PI;
        float c = cosf(a), s = sinf(a);
        glNormal3f(c, 0, s);
        glVertex3f(CYL_R*c, yB, CYL_R*s);
        glVertex3f(CYL_R*c, yT, CYL_R*s);
    }
    glEnd();

    // top/bottom caps
    col(0.15f, 0.18f, 0.26f);
    for(int cap = 0; cap < 2; cap++){
        float yy = (cap==0) ? yB : yT;
        float ny = (cap==0) ? -1.f : 1.f;
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, ny, 0);
        glVertex3f(0, yy, 0);
        for(int i = 0; i <= CYL_SEG; i++){
            float a = (float)(cap==0 ? -i : i)/CYL_SEG * 2.f*(float)M_PI;
            glVertex3f(CYL_R*cosf(a), yy, CYL_R*sinf(a));
        }
        glEnd();
    }

    // surface grid lines
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    col(0.34f, 0.40f, 0.56f);
    glLineWidth(1.f);
    for(int j = 0; j <= 8; j++){        // latitude rings
        float yy = yB + (float)j/8.f * CYL_H;
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i <= CYL_SEG; i++){
            float a = (float)i/CYL_SEG * 2.f*(float)M_PI;
            glVertex3f((CYL_R+.01f)*cosf(a), yy, (CYL_R+.01f)*sinf(a));
        }
        glEnd();
    }
    for(int i = 0; i < 16; i++){        // longitude lines
        float a = (float)i/16.f * 2.f*(float)M_PI;
        float c = cosf(a), s = sinf(a);
        glBegin(GL_LINES);
        glVertex3f((CYL_R+.01f)*c, yB, (CYL_R+.01f)*s);
        glVertex3f((CYL_R+.01f)*c, yT, (CYL_R+.01f)*s);
        glEnd();
    }
    glPopAttrib();
}

// ════════════════════════════════════════════════════════════
//  Kinematics
//
//  wSpeed = spin rate in deg/frame (positive = clockwise from outside)
//
//  Rolling without slip at contact circle (radius = CYL_R):
//    contact arc per frame = TYRE_R * |dSpin_rad|
//    dTheta = -dSpin_rad * (TYRE_R / CYL_R)
//      sign: clockwise spin (+wSpeed) → moves anticlockwise (+theta)
//            so we use  dTheta = +dSpin_rad * TYRE_R / CYL_R
// ════════════════════════════════════════════════════════════
static void update()
{
    float dSpin_rad = wSpeed * (float)M_PI / 180.f;  // deg→rad per frame
    wSpin  += wSpeed;                                  // accumulate deg
    wTheta += dSpin_rad * (TYRE_R / CYL_R);           // roll around tank

    // wrap theta
    const float PI2 = 2.f*(float)M_PI;
    if(wTheta >  (float)M_PI) wTheta -= PI2;
    if(wTheta < -(float)M_PI) wTheta += PI2;
}

// ════════════════════════════════════════════════════════════
//  Wheel transform
//
//  Wheel centre sits at radius (CYL_R + TYRE_R) from tank axis.
//  Tyre outer edge is at CYL_R → touches tank, never penetrates.
//  Rim outer edge is at RIM_R  < TYRE_R → floats clear of tank.
//
//  1. Translate  → wheel centre on surface
//  2. RotateY(θ) → correct azimuth
//  3. RotateY(90°) → wheel Z-axis (axle) points tangentially
//  4. RotateZ(wSpin) → spin (Z only – only allowed rotation)
// ════════════════════════════════════════════════════════════
static void applyWheelTransform()
{
    const float toDeg = 180.f/(float)M_PI;
    const float R = CYL_R + TYRE_R;   // wheel centre orbit radius

    glTranslatef(R*cosf(wTheta), wHeight, R*sinf(wTheta));  // 1
    glRotatef(wTheta * toDeg, 0, 1, 0);                      // 2
    glRotatef(90.f,            0, 1, 0);                      // 3
    glRotatef(90.f, 1, 0, 0);
    glRotatef(wSpin,           0, 0, 1);                      // 4 – Z only
}

// ── draw the full wheel (rim inside tyre) ────────────────────
static void drawWheelAssembly()
{
    // TYRE  – dark rubber outer ring
    col(0.14f, 0.14f, 0.14f);
    drawTyre(RIM_R, TYRE_R, TYRE_T*0.5f, SEG);

    // RIM   – lighter metallic inner hub
    col(0.42f, 0.42f, 0.45f);
    drawCylZ(RIM_R, RIM_T*0.5f, SEG);

    // spoke mark on rim face (so spin is visible)
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    col(0.70f, 0.70f, 0.72f);
    glLineWidth(2.f);
    glBegin(GL_LINES);
    // 3 spokes at 0°, 120°, 240°
    for(int sp = 0; sp < 3; sp++){
        float a = (float)sp / 3.f * 2.f*(float)M_PI;
        float c = cosf(a)*RIM_R, s = sinf(a)*RIM_R;
        float zz = TYRE_T*0.5f + 0.002f;
        glVertex3f(0, 0, zz);
        glVertex3f(c, s, zz);
        glVertex3f(0, 0, -zz);
        glVertex3f(c, s, -zz);
    }
    glEnd();
    glPopAttrib();
}

// ── HUD ──────────────────────────────────────────────────────
static void hud()
{
    char buf[200];
    snprintf(buf, sizeof(buf),
        "W/S=clockwise/anti  UP/DN=slide Y  R=reset  |"
        "  theta=%.1f  h=%.2f  spin=%.0f deg",
        wTheta*180.f/(float)M_PI, wHeight, wSpin);

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0,W,0,H);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glColor3f(0.75f, 0.95f, 0.65f);
    glRasterPos2f(10, 10);
    for(char* p=buf; *p; p++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ── display ──────────────────────────────────────────────────
static void display()
{
    glClearColor(0.07f, 0.08f, 0.10f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0, (double)W/H, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float rT = camTheta*(float)M_PI/180.f;
    float rP = camPhi  *(float)M_PI/180.f;
    gluLookAt(camDist*cosf(rP)*sinf(rT),
              camDist*sinf(rP),
              camDist*cosf(rP)*cosf(rT),
              0,0,0, 0,1,0);

    // lighting
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_LIGHT1);
    glEnable(GL_DEPTH_TEST); glEnable(GL_NORMALIZE);
    GLfloat p0[]={10,15,10,1}, d0[]={1,.95f,.85f,1}, a0[]={.22f,.22f,.22f,1};
    GLfloat p1[]={-8, 8,-8,1}, d1[]={.3f,.4f,.6f,1};
    glLightfv(GL_LIGHT0,GL_POSITION,p0); glLightfv(GL_LIGHT0,GL_DIFFUSE,d0);
    glLightfv(GL_LIGHT0,GL_AMBIENT, a0);
    glLightfv(GL_LIGHT1,GL_POSITION,p1); glLightfv(GL_LIGHT1,GL_DIFFUSE,d1);
    GLfloat sp[]={.35f,.35f,.35f,1}, sh[]={45};
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,sp);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,sh);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // world axes + grid
    drawAxes(3.f);
    drawGrid();

    // tank
    drawTank();

    // wheel assembly on tank outer surface
    glPushMatrix();
      applyWheelTransform();
      drawWheelAssembly();
      drawAxes(1.4f);   // wheel-local TF  (Z=blue = axle/spin axis)
    glPopMatrix();

    hud();
    glutSwapBuffers();
}

static void idle(){ update(); glutPostRedisplay(); }

static void keyboard(unsigned char k, int, int)
{
    switch(k){
    case 'w': case 'W': wSpeed =  1.8f;  break;   // clockwise spin
    case 's': case 'S': wSpeed = -1.8f;  break;   // anticlockwise spin
    case 'r': case 'R':
        wTheta=wHeight=wSpin=wSpeed=0.f;
        camTheta=40; camPhi=20; camDist=22;
        break;
    case 27: exit(0);
    }
}
static void keyUp(unsigned char k, int, int)
{
    if(k=='w'||k=='W'||k=='s'||k=='S') wSpeed=0.f;
}

static void special(int k, int, int)
{
    const float SLIDE = 0.1f;
    if(k==GLUT_KEY_UP)   wHeight += SLIDE;
    if(k==GLUT_KEY_DOWN) wHeight -= SLIDE;
    // clamp to tank wall
    float maxH = CYL_H*0.5f - TYRE_R - 0.02f;
    if(wHeight >  maxH) wHeight =  maxH;
    if(wHeight < -maxH) wHeight = -maxH;
    glutPostRedisplay();
}

static void mouse(int b, int s, int x, int y)
{
    if(b==GLUT_LEFT_BUTTON){ dragging=(s==GLUT_DOWN); lastX=x; lastY=y; }
    if(b==3){ camDist-=0.7f; if(camDist<3)  camDist=3;  }
    if(b==4){ camDist+=0.7f; if(camDist>80) camDist=80; }
    glutPostRedisplay();
}
static void motion(int x, int y)
{
    if(dragging){
        camTheta -= (x-lastX)*0.5f;
        camPhi   += (y-lastY)*0.5f;
        if(camPhi> 89) camPhi= 89;
        if(camPhi<-89) camPhi=-89;
        lastX=x; lastY=y;
        glutPostRedisplay();
    }
}
static void reshape(int w, int h){ W=w; H=h?h:1; glViewport(0,0,W,H); }

int main(int argc, char** argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(W,H);
    glutCreateWindow("Wheel (tyre+rim) on Cylinder – Z-axis spin only");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutMainLoop();
}