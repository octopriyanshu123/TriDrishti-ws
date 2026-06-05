#include <GL/glut.h>
#include <cmath>
#include <cstring>

class Pose
{
public:
    float x;      // world X position
    float y;      // world Y position  
    float z;      // world Z position
    float roll;   // rotation around X axis (degrees)
    float pitch;  // rotation around Y axis (degrees)
    float yaw;    // rotation around Z axis (degrees)

    // ── constructors ─────────────────────────────────────────
    Pose() : x(0), y(0), z(0), roll(0), pitch(0), yaw(0) {}
    
    Pose(float x_, float y_, float z_) 
        : x(x_), y(y_), z(z_), roll(0), pitch(0), yaw(0) {}
    
    Pose(float x_, float y_, float z_, float roll_, float pitch_, float yaw_) 
        : x(x_), y(y_), z(z_), roll(roll_), pitch(pitch_), yaw(yaw_) {}
    
    // ── setters ──────────────────────────────────────────────
    void setPosition(float x_, float y_, float z_)
    {
        x = x_; y = y_; z = z_;
    }
    
    void setOrientation(float roll_, float pitch_, float yaw_)
    {
        roll = roll_; pitch = pitch_; yaw = yaw_;
    }
    
    void setTransform(float x_, float y_, float z_, float roll_, float pitch_, float yaw_)
    {
        x = x_; y = y_; z = z_;
        roll = roll_; pitch = pitch_; yaw = yaw_;
    }
    
    // ── getters ──────────────────────────────────────────────
    void getPosition(float &x_, float &y_, float &z_) const
    {
        x_ = x; y_ = y; z_ = z;
    }
    
    void getOrientation(float &roll_, float &pitch_, float &yaw_) const
    {
        roll_ = roll; pitch_ = pitch; yaw_ = yaw;
    }
    
    // ── apply OpenGL transformation ──────────────────────────
    void applyGLTransform() const
    {
        glTranslatef(x, y, z);
        
        // Apply rotations in ZYX order (yaw -> pitch -> roll)
        if (yaw != 0.0f)   glRotatef(yaw,   0.0f, 0.0f, 1.0f);
        if (pitch != 0.0f) glRotatef(pitch, 0.0f, 1.0f, 0.0f);
        if (roll != 0.0f)  glRotatef(roll,  1.0f, 0.0f, 0.0f);
    }
    
    // ── transform a point from local to world coordinates ────
    void transformPoint(float localX, float localY, float localZ, 
                        float &worldX, float &worldY, float &worldZ) const
    {
        // Convert angles to radians
        float r = roll * M_PI / 180.0f;
        float p = pitch * M_PI / 180.0f;
        float y = yaw * M_PI / 180.0f;
        
        // Rotation matrix ZYX (yaw, pitch, roll)
        float cr = cos(r), sr = sin(r);
        float cp = cos(p), sp = sin(p);
        float cy = cos(y), sy = sin(y);
        
        // Apply rotation
        float rotatedX = cp * cy * localX + (sr * sp * cy - cr * sy) * localY + (cr * sp * cy + sr * sy) * localZ;
        float rotatedY = cp * sy * localX + (sr * sp * sy + cr * cy) * localY + (cr * sp * sy - sr * cy) * localZ;
        float rotatedZ = -sp * localX + sr * cp * localY + cr * cp * localZ;
        
        // Apply translation
        worldX = x + rotatedX;
        worldY = y + rotatedY;
        worldZ = z + rotatedZ;
    }
    
    // ── transform direction vector (no translation) ──────────
    void transformDirection(float localX, float localY, float localZ,
                            float &worldX, float &worldY, float &worldZ) const
    {
        // Convert angles to radians
        float r = roll * M_PI / 180.0f;
        float p = pitch * M_PI / 180.0f;
        float y = yaw * M_PI / 180.0f;
        
        float cr = cos(r), sr = sin(r);
        float cp = cos(p), sp = sin(p);
        float cy = cos(y), sy = sin(y);
        
        // Apply rotation only (no translation)
        worldX = cp * cy * localX + (sr * sp * cy - cr * sy) * localY + (cr * sp * cy + sr * sy) * localZ;
        worldY = cp * sy * localX + (sr * sp * sy + cr * cy) * localY + (cr * sp * sy - sr * cy) * localZ;
        worldZ = -sp * localX + sr * cp * localY + cr * cp * localZ;
    }
    
    // ── get local axis directions in world coordinates ───────
    void getLocalXAxis(float &wx, float &wy, float &wz) const
    {
        transformDirection(1.0f, 0.0f, 0.0f, wx, wy, wz);
    }
    
    void getLocalYAxis(float &wx, float &wy, float &wz) const
    {
        transformDirection(0.0f, 1.0f, 0.0f, wx, wy, wz);
    }
    
    void getLocalZAxis(float &wx, float &wy, float &wz) const
    {
        transformDirection(0.0f, 0.0f, 1.0f, wx, wy, wz);
    }
    
    // ── matrix operations ────────────────────────────────────
    void getGLMatrix(float matrix[16]) const
    {
        // Identity matrix
        for (int i = 0; i < 16; i++) matrix[i] = 0;
        matrix[15] = 1.0f;
        
        // Convert angles to radians
        float r = roll * M_PI / 180.0f;
        float p = pitch * M_PI / 180.0f;
        float y = yaw * M_PI / 180.0f;
        
        float cr = cos(r), sr = sin(r);
        float cp = cos(p), sp = sin(p);
        float cy = cos(y), sy = sin(y);
        
        // Rotation matrix (ZYX order)
        matrix[0] = cp * cy;
        matrix[1] = cp * sy;
        matrix[2] = -sp;
        matrix[3] = 0;
        
        matrix[4] = sr * sp * cy - cr * sy;
        matrix[5] = sr * sp * sy + cr * cy;
        matrix[6] = sr * cp;
        matrix[7] = 0;
        
        matrix[8] = cr * sp * cy + sr * sy;
        matrix[9] = cr * sp * sy - sr * cy;
        matrix[10] = cr * cp;
        matrix[11] = 0;
        
        // Translation
        matrix[12] = x;
        matrix[13] = y;
        matrix[14] = z;
    }
    
    // ── composition (combine poses) ─────────────────────────
    Pose compose(const Pose &other) const
    {
        // Transform other's position relative to this pose
        float newX, newY, newZ;
        transformPoint(other.x, other.y, other.z, newX, newY, newZ);
        
        // Combine rotations (simple Euler addition - approximate)
        // For precise composition, use quaternions
        float newRoll = roll + other.roll;
        float newPitch = pitch + other.pitch;
        float newYaw = yaw + other.yaw;
        
        // Normalize angles to [-180, 180]
        auto normalize = [](float angle) {
            angle = fmod(angle, 360.0f);
            if (angle > 180.0f) angle -= 360.0f;
            if (angle < -180.0f) angle += 360.0f;
            return angle;
        };
        
        return Pose(newX, newY, newZ, normalize(newRoll), normalize(newPitch), normalize(newYaw));
    }
    
    // ── invert pose (world -> local transformation) ──────────
    Pose inverse() const
    {
        // Inverse rotation is negative angles
        float invRoll = -roll;
        float invPitch = -pitch;
        float invYaw = -yaw;
        
        // Inverse translation: rotate negative position by inverse rotation
        float invX, invY, invZ;
        float r = invRoll * M_PI / 180.0f;
        float p = invPitch * M_PI / 180.0f;
        float y = invYaw * M_PI / 180.0f;
        
        float cr = cos(r), sr = sin(r);
        float cp = cos(p), sp = sin(p);
        float cy = cos(y), sy = sin(y);
        
        // Apply inverse rotation to negative position
        invX = -(x * cp * cy + y * (sr * sp * cy - cr * sy) + z * (cr * sp * cy + sr * sy));
        invY = -(x * cp * sy + y * (sr * sp * sy + cr * cy) + z * (cr * sp * sy - sr * cy));
        invZ = -(-x * sp + y * sr * cp + z * cr * cp);
        
        return Pose(invX, invY, invZ, invRoll, invPitch, invYaw);
    }
    
    // ── utility functions ────────────────────────────────────
    void reset()
    {
        x = y = z = 0;
        roll = pitch = yaw = 0;
    }
    
    void print() const
    {
        printf("Pose: pos(%.2f, %.2f, %.2f) rot(%.1f°, %.1f°, %.1f°)\n", 
               x, y, z, roll, pitch, yaw);
    }
    
    bool isEqual(const Pose &other, float epsilon = 0.001f) const
    {
        return (fabs(x - other.x) < epsilon &&
                fabs(y - other.y) < epsilon &&
                fabs(z - other.z) < epsilon &&
                fabs(roll - other.roll) < epsilon &&
                fabs(pitch - other.pitch) < epsilon &&
                fabs(yaw - other.yaw) < epsilon);
    }
};