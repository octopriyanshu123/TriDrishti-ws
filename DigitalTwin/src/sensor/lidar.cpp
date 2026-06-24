#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <limits>


static const float LIDAR_RANGE      = 1.0f;   // units
static const float ANGLE_RESOLUTION = 30.0f;   // degrees per ray
static const int   NUM_RAYS         = (int)(360.0f / ANGLE_RESOLUTION);
static const float DEG2RAD          = 3.14159265358979f / 180.0f;

//  Wall / obstacle representation
struct Segment {
    float x1, y1, x2, y2;
};

//  Ray-hit result
struct Hit {
    float x, y;      // world-space hit position
    float dist;      // distance from origin
    bool  valid;     // did we actually hit something?
};


struct Lidar2D {
    float posX, posY;          // sensor origin in world space
    float headingDeg;          // sensor heading (0 = +X axis)

    std::vector<Segment> world;   // obstacles
    std::vector<Hit>     hits;    // last scan results

    // ── helpers ──────────────────────────────

    // Ray vs finite segment intersection.
    // Ray: origin (ox,oy), unit direction (dx,dy), max length maxLen.
    // Returns parametric t along the ray (or -1 if no hit).
    static float raySegmentIntersect(
        float ox, float oy, float dx, float dy, float maxLen,
        float x1, float y1, float x2, float y2)
    {
        float ex = x2 - x1, ey = y2 - y1;   // segment direction
        float denom = dx * ey - dy * ex;
        if (std::fabs(denom) < 1e-9f) return -1.0f; // parallel

        float fx = x1 - ox, fy = y1 - oy;
        float t = (fx * ey - fy * ex) / denom;
        float u = (fx * dy - fy * dx) / denom;

        if (t < 0.0f || t > maxLen) return -1.0f;
        if (u < 0.0f || u > 1.0f)  return -1.0f;
        return t;
    }

    // ── public API ───────────────────────────

    void init(float x, float y, float headingDeg_ = 0.0f)
    {
        posX       = x;
        posY       = y;
        headingDeg = headingDeg_;
        hits.resize(NUM_RAYS);

        // Default environment: a room + one internal box obstacle
        world.clear();

        // Outer walls (8 × 8 room centred at origin)
        world.push_back({-4,-4,  4,-4});   // bottom
        world.push_back({ 4,-4,  4, 4});   // right
        world.push_back({ 4, 4, -4, 4});   // top
        world.push_back({-4, 4, -4,-4});   // left

        // Internal box (centred at (1.5, 1.0))
        world.push_back({ 1.0f, 0.5f,  2.0f, 0.5f});
        world.push_back({ 2.0f, 0.5f,  2.0f, 1.5f});
        world.push_back({ 2.0f, 1.5f,  1.0f, 1.5f});
        world.push_back({ 1.0f, 1.5f,  1.0f, 0.5f});

        // Diagonal wall
        world.push_back({-3.5f, 1.0f, -1.5f, 3.0f});
    }

    void scan()
    {
        for (int i = 0; i < NUM_RAYS; ++i)
        {
            float angleDeg = headingDeg + i * ANGLE_RESOLUTION;
            float angleRad = angleDeg * DEG2RAD;
            float dx = std::cos(angleRad);
            float dy = std::sin(angleRad);

            float bestT = LIDAR_RANGE;
            bool  hit   = false;

            for (const Segment& seg : world)
            {
                float t = raySegmentIntersect(
                    posX, posY, dx, dy, LIDAR_RANGE,
                    seg.x1, seg.y1, seg.x2, seg.y2);
                if (t > 0.0f && t < bestT) {
                    bestT = t;
                    hit   = true;
                }
            }

            hits[i].dist  = bestT;
            hits[i].valid = hit;
            hits[i].x     = posX + dx * bestT;
            hits[i].y     = posY + dy * bestT;
        }
    }

    void setPosition(float x, float y) { posX = x; posY = y; }
    void setHeading(float deg)         { headingDeg = deg; }

    // ── drawing ──────────────────────────────

    void draw() const
    {
        // 1. Draw world obstacles
        glLineWidth(2.0f);
        glColor3f(0.9f, 0.9f, 0.9f);
        glBegin(GL_LINES);
        for (const Segment& seg : world) {
            glVertex2f(seg.x1, seg.y1);
            glVertex2f(seg.x2, seg.y2);
        }
        glEnd();

        // 2. Draw LiDAR range circle (dashed look via many small arcs)
        glLineWidth(1.0f);
        glColor4f(0.2f, 0.5f, 0.2f, 0.4f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 360; ++i) {
            float a = i * DEG2RAD;
            glVertex2f(posX + LIDAR_RANGE * std::cos(a),
                       posY + LIDAR_RANGE * std::sin(a));
        }
        glEnd();

        // 3. Draw rays: hits in green, max-range misses faint
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (const Hit& h : hits) {
            if (h.valid) {
                // Gradient: bright near sensor, dimmer at hit
                float intensity = 1.0f - (h.dist / LIDAR_RANGE) * 0.6f;
                glColor4f(0.0f, intensity, 0.2f, 0.55f);
            } else {
                glColor4f(0.15f, 0.35f, 0.15f, 0.18f);
            }
            glVertex2f(posX, posY);
            glVertex2f(h.x, h.y);
        }
        glEnd();

        // 4. Draw hit points
        glPointSize(3.5f);
        glBegin(GL_POINTS);
        for (const Hit& h : hits) {
            if (h.valid) {
                glColor3f(0.0f, 1.0f, 0.3f);
                glVertex2f(h.x, h.y);
            }
        }
        glEnd();

        // 5. Draw sensor body
        glPointSize(8.0f);
        glColor3f(1.0f, 0.8f, 0.0f);
        glBegin(GL_POINTS);
            glVertex2f(posX, posY);
        glEnd();

        // 6. Draw heading indicator
        float headRad = headingDeg * DEG2RAD;
        glLineWidth(2.5f);
        glColor3f(1.0f, 0.6f, 0.0f);
        glBegin(GL_LINES);
            glVertex2f(posX, posY);
            glVertex2f(posX + 0.3f * std::cos(headRad),
                       posY + 0.3f * std::sin(headRad));
        glEnd();
    }
};