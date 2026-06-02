

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <map>

double now_seconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

struct TFSnapshot {
    double timestamp;       // seconds
    double x, y, z;        // position  (meters)
    double roll, pitch, yaw; // orientation (radians)

    TFSnapshot(double t,
               double x=0, double y=0, double z=0,
               double roll=0, double pitch=0, double yaw=0)
        : timestamp(t), x(x), y(y), z(z),
          roll(roll), pitch(pitch), yaw(yaw) {}
};

class TFTrail {
protected:
    std::string joint_name;
    std::vector<TFSnapshot> trail;
    size_t max_history;  // limit memory usage

public:
    TFTrail(const std::string& name, size_t max_history = 10000)
        : joint_name(name), max_history(max_history) {}

    const std::string& getName() const { return joint_name; }
    size_t size() const { return trail.size(); }


    void record(const TFSnapshot& snap) {
        if (trail.size() >= max_history) {
            trail.erase(trail.begin()); // drop oldest to save memory
        }
        trail.push_back(snap);
    }


    TFSnapshot interpolate(double target_time) const {
        if (trail.empty())
            throw std::runtime_error("[" + joint_name + "] Trail is empty.");

        // Before first record
        if (target_time <= trail.front().timestamp)
            return trail.front();

        // After last record
        if (target_time >= trail.back().timestamp)
            return trail.back();

        // Find surrounding two snapshots
        for (size_t i = 0; i < trail.size() - 1; i++) {
            const TFSnapshot& A = trail[i];
            const TFSnapshot& B = trail[i + 1];

            if (A.timestamp <= target_time && target_time <= B.timestamp) {
                double alpha = (target_time - A.timestamp) /
                               (B.timestamp - A.timestamp);

                // Interpolate all fields
                return TFSnapshot(
                    target_time,
                    A.x     + alpha * (B.x     - A.x),
                    A.y     + alpha * (B.y     - A.y),
                    A.z     + alpha * (B.z     - A.z),
                    A.roll  + alpha * (B.roll  - A.roll),
                    A.pitch + alpha * (B.pitch - A.pitch),
                    A.yaw   + alpha * (B.yaw   - A.yaw)
                );
            }
        }

        throw std::runtime_error("[" + joint_name + "] Time out of range.");
    }

    void printTrail(int last_n = 5) const {
        std::cout << "\n--- Trail: " << joint_name << " ---\n";
        std::cout << std::fixed << std::setprecision(4);
        int start = std::max(0, (int)trail.size() - last_n);
        for (int i = start; i < (int)trail.size(); i++) {
            const auto& s = trail[i];
            std::cout << "  t=" << s.timestamp
                      << " | pos=(" << s.x << ", " << s.y << ", " << s.z << ")"
                      << " | rpy=(" << s.roll << ", " << s.pitch << ", " << s.yaw << ")\n";
        }
    }
};

class FixedJointTrail : public TFTrail {
public:
    FixedJointTrail(const std::string& name,
                    double x, double y, double z,
                    double roll, double pitch, double yaw)
        : TFTrail(name, 1)  // only 1 record needed
    {
        // Fixed joints are recorded once at construction
        record(TFSnapshot(now_seconds(), x, y, z, roll, pitch, yaw));
        std::cout << "[Fixed] '" << name << "' registered at startup.\n";
    }

    // Fixed joints don't change — always return static pose
    TFSnapshot getStaticPose() const {
        return trail.front();
    }
};

class ContinuousJointTrail : public TFTrail {
    double wheel_x, wheel_y, wheel_z;  // fixed mount position

public:
    ContinuousJointTrail(const std::string& name,
                         double mount_x, double mount_y, double mount_z)
        : TFTrail(name), wheel_x(mount_x), wheel_y(mount_y), wheel_z(mount_z)
    {
        std::cout << "[Continuous] '" << name << "' wheel trail ready.\n";
    }

    // Call this in your control loop
    // angle_rad: cumulative wheel angle (can exceed 2*PI, no wrapping)
    void recordAngle(double angle_rad) {
        record(TFSnapshot(now_seconds(),
                          wheel_x, wheel_y, wheel_z,
                          0, 0, angle_rad));  // yaw = wheel rotation
    }

    // Get interpolated wheel angle at target time
    double getAngleAt(double target_time) {
        return interpolate(target_time).yaw;
    }
};

class LinearActuatorTrail : public TFTrail {
    double base_x, base_y;   // fixed base mount position
    double max_extension;

public:
    LinearActuatorTrail(const std::string& name,
                        double base_x, double base_y,
                        double max_extension_m)
        : TFTrail(name), base_x(base_x), base_y(base_y),
          max_extension(max_extension_m)
    {
        std::cout << "[LinearActuator] '" << name << "' trail ready. "
                  << "Max extension: " << max_extension_m << "m\n";
    }

    // extension_m: current extension in meters
    void recordExtension(double extension_m) {
        if (extension_m < 0 || extension_m > max_extension) {
            std::cerr << "[WARN] " << joint_name
                      << " extension out of range: " << extension_m << "\n";
        }
        // z = extension along vertical axis
        record(TFSnapshot(now_seconds(), base_x, base_y, extension_m));
    }

    // Get interpolated extension (z) at target time
    double getExtensionAt(double target_time) {
        return interpolate(target_time).z;
    }
};

class RasterSensorTrail : public TFTrail {
    double fixed_z;  // sensor stays at constant height

public:
    RasterSensorTrail(const std::string& name, double sensor_height_z)
        : TFTrail(name), fixed_z(sensor_height_z)
    {
        std::cout << "[RasterSensor] '" << name << "' trail ready. "
                  << "Height z=" << sensor_height_z << "m\n";
    }

    // Record current scan position (x, y in meters)
    void recordPosition(double x, double y) {
        record(TFSnapshot(now_seconds(), x, y, fixed_z));
    }

    // Get interpolated (x, y) position at target time
    std::pair<double, double> getPositionAt(double target_time) {
        auto snap = interpolate(target_time);
        return {snap.x, snap.y};
    }
};

class TFTrailManager {
public:
    // Fixed Joints
    FixedJointTrail lidar_front  {"lidar_front",  0.30,  0.00, 0.20, 0, 0, 0};
    FixedJointTrail lidar_left   {"lidar_left",   0.00,  0.20, 0.20, 0, 0,  1.5708};
    FixedJointTrail lidar_right  {"lidar_right",  0.00, -0.20, 0.20, 0, 0, -1.5708};
    FixedJointTrail imu          {"imu",          0.00,  0.00, 0.15, 0, 0, 0};

    // Continuous Joints (Wheels)
    ContinuousJointTrail wheel_left  {"wheel_left",  0.00,  0.15, 0.05};
    ContinuousJointTrail wheel_right {"wheel_right", 0.00, -0.15, 0.05};

    // Linear Actuator
    LinearActuatorTrail  actuator    {"linear_actuator", 0.0, 0.0, 0.50};

    // Raster Sensor
    RasterSensorTrail    raster      {"raster_sensor", 0.30};

    void printAllTrails() {
        lidar_front.printTrail(3);
        lidar_left.printTrail(3);
        lidar_right.printTrail(3);
        imu.printTrail(3);
        wheel_left.printTrail(3);
        wheel_right.printTrail(3);
        actuator.printTrail(3);
        raster.printTrail(3);
    }
};


int main() {
    TFTrailManager robot;

  
    double t0 = now_seconds();

    for (int i = 0; i < 5; i++) {
        double t = now_seconds();

        // Wheels rotating at different speeds
        robot.wheel_left.recordAngle(0.5 * i);       // 0.5 rad per step
        robot.wheel_right.recordAngle(0.48 * i);     // slightly different

        // Actuator extending slowly
        robot.actuator.recordExtension(0.05 * i);    // 5cm per step

        // Raster sensor sweeping left to right
        robot.raster.recordPosition(0.1 * i, 0.02 * i);

        // Small delay to separate timestamps
        struct timespec ts = {0, 5000000}; // 5ms
        nanosleep(&ts, nullptr);
    }

    // --- Interpolation Demo ---
    std::cout << "\n--- Interpolation Demo ---\n";
    std::cout << std::fixed << std::setprecision(5);

    double trail_start = t0;
    double trail_end   = now_seconds();
    double query_time  = (trail_start + trail_end) / 2.0; // midpoint

    std::cout << "\nQuerying all joints at time t=" << query_time << "s\n";

    try {
        double lw = robot.wheel_left.getAngleAt(query_time);
        double rw = robot.wheel_right.getAngleAt(query_time);
        double ext = robot.actuator.getExtensionAt(query_time);
        auto [rx, ry] = robot.raster.getPositionAt(query_time);

        std::cout << "  Wheel Left  angle : " << lw  << " rad\n";
        std::cout << "  Wheel Right angle : " << rw  << " rad\n";
        std::cout << "  Actuator extension: " << ext << " m\n";
        std::cout << "  Raster position   : (" << rx << ", " << ry << ") m\n";

        std::cout << "\n  Fixed joints (static — no interpolation needed):\n";
        auto lidar = robot.lidar_front.getStaticPose();
        std::cout << "  Lidar Front pose  : ("
                  << lidar.x << ", " << lidar.y << ", " << lidar.z << ")\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    robot.printAllTrails();

    std::cout << "\n====== TF Trail System OK ======\n";
    return 0;
}
