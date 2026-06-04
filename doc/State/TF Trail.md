# TF Trail System
---

## WHY — Why Do We Need TF Trail?

When a robot is moving, every joint (wheel, actuator, sensor) changes its position over time. Your control loop records these positions, but it can only sample at a fixed rate — for example, 100 times per second.

**The Problem:**

> "What was the exact position of the wheel at t = 1.0563 seconds?"

That moment was never directly recorded. It fell **between two samples**. Without a trail system, you cannot answer this — you would only have the values at t = 1.05 and t = 1.06.

**The Solution — TF Trail:** Save a time-stamped history (a "trail") of every joint's position. Then use **interpolation** to calculate the exact value at any moment in time.

###### We are saving the TF trail because we need to know the exact position of each joint at any specific point in time.Since we are recording joint states at fixed intervals, there will be moments in between recordings where we don't have direct data. So by saving a time-stamped history of every joint's position, we can use linear interpolation to calculate the exact joint position at any requested time — even if that moment was never directly recorded.
---

## WHAT — What Is a TF Trail?

A **TF Trail** is a list of snapshots saved over time for one joint.

Each snapshot contains:

```
timestamp  →  when was this recorded? (seconds)
x, y, z    →  position in meters
roll, pitch, yaw  →  orientation in radians
```

Think of it like a **flight recorder (black box)** for your robot joints. Every moment is saved. You can go back and ask: _"Where was joint X at time T?"_ — and get a precise answer.

---

## WHEN — When Is the Trail Recorded?

|Joint Type|When to Record|
|---|---|
|Fixed Joint (Lidar, IMU)|**Once at startup** — position never changes|
|Continuous Joint (Wheels)|**Every control loop cycle** — angle changes constantly|
|Linear Actuator|**Every control loop cycle** — extension changes|
|Raster Sensor|**Every scan cycle** — x, y position changes|

In your main control loop it looks like this:

```
while robot is running:
    read sensor data
    → record wheel_left angle
    → record wheel_right angle
    → record actuator extension
    → record raster position
    sleep until next cycle
```

---

## HOW — How Does Interpolation Work?

Imagine the wheel angle was recorded like this:

```
t = 1.00s  →  angle = 30°
t = 1.10s  →  angle = 50°
```

You want to know the angle at **t = 1.06s**.

**Step 1 — Find the two surrounding samples (A and B):**

```
A: t=1.00, angle=30°
B: t=1.10, angle=50°
Target: t=1.06
```

**Step 2 — Calculate alpha (how far between A and B):**

```
alpha = (target - A.time) / (B.time - A.time)
alpha = (1.06 - 1.00) / (1.10 - 1.00)
alpha = 0.06 / 0.10
alpha = 0.6   ← means "60% of the way from A to B"
```

**Step 3 — Interpolate the value:**

```
result = A.angle + alpha × (B.angle - A.angle)
result = 30 + 0.6 × (50 - 30)
result = 30 + 12
result = 42°  ← exact answer at t=1.06s
```

This same formula is applied for **all joint types** — angle, extension, or position.

---

## Joint Types in This Project

---

### 1. Fixed Joint — Lidar (×3) and IMU (×1)

**What it is:** A joint that does not move. It is bolted to the robot body at a fixed position and orientation. Lidar sensors and the IMU do not change their position relative to the robot frame.

**Why we still save it:** Even though it does not move, we need its position saved so the system knows where the sensor is mounted. This lets us transform sensor readings into the robot's coordinate frame.

**When recorded:** Once at program startup.

**Class:** `FixedJointTrail`

```
Joints:
  lidar_front   → x=0.30, y=0.00, z=0.20  (facing forward)
  lidar_left    → x=0.00, y=0.20, z=0.20  (facing left, yaw=90°)
  lidar_right   → x=0.00, y=-0.20, z=0.20 (facing right, yaw=-90°)
  imu           → x=0.00, y=0.00, z=0.15  (center of robot)
```

---

### 2. Continuous Joint — Left Wheel and Right Wheel (×2)

**What it is:** A joint that rotates without any limits — it can spin 360° and keep going. The angle grows continuously (e.g., 0 → 6.28 → 12.56 → ...). There is no maximum angle — it never wraps back to zero.

**Why we save it:** By recording wheel angle over time, we can calculate:

- How far the robot has travelled (odometry)
- Exact wheel angle at any past moment for sync with other sensors
- Speed and acceleration of each wheel

**When recorded:** Every control loop cycle.

**Class:** `ContinuousJointTrail`

```
Joints:
  wheel_left    → mounted at y=+0.15 (left side of robot)
  wheel_right   → mounted at y=-0.15 (right side of robot)
```

---

### 3. Linear Actuator (×1)

**What it is:** A joint that moves in a straight line — it extends and retracts along one axis (Z axis in this project). The value is the extension length in meters, from 0.0 (fully retracted) to max_extension (fully extended).

**Why we save it:**

- Know the exact height/extension at any past moment
- Sync actuator position with camera or sensor readings
- Detect if the actuator moved too fast or skipped a position

**When recorded:** Every control loop cycle.

**Class:** `LinearActuatorTrail`

```
Joint:
  linear_actuator → base at (0, 0), max extension = 0.50m
```

---

### 4. Raster Sensor (×1)

**What it is:** A sensor that sweeps across a 2D surface (like a scanner or a camera moving on a rail). It has an x and y position that changes over time, but stays at a fixed height (z is constant).

**Why we save it:**

- Know exactly where the sensor was scanning at any moment
- Match scan data to the correct position during post-processing
- Reconstruct the full scan map with correct spatial alignment

**When recorded:** Every scan cycle.

**Class:** `RasterSensorTrail`

```
Joint:
  raster_sensor → fixed height z=0.30m, sweeps in x and y
```

---

## File Structure

```
tf_trail.cpp
│
├── now_seconds()           → utility to get current time
├── TFSnapshot              → one recorded moment (t, x, y, z, rpy)
├── TFTrail (base class)    → stores trail + interpolation logic
│   ├── record()            → save a new snapshot
│   ├── interpolate()       → get value at any time using linear interp
│   └── printTrail()        → debug — print last N snapshots
│
├── FixedJointTrail         → for Lidar x3, IMU
├── ContinuousJointTrail    → for Wheel Left, Wheel Right
├── LinearActuatorTrail     → for Linear Actuator
├── RasterSensorTrail       → for Raster Sensor
│
└── TFTrailManager          → holds ALL joints in one object
```

---

## How to Build and Run

```bash
# Compile
g++ -std=c++17 -o tf_trail tf_trail.cpp

# Run
./tf_trail
```

**Requirements:**

- Linux (Ubuntu 20.04 or newer recommended)
- g++ with C++17 support (`sudo apt install g++`)
- No external libraries needed

---

## Memory Management

Each trail has a `max_history` limit (default: 10,000 snapshots). When the trail is full, the oldest snapshot is dropped automatically.

At 100Hz (100 recordings per second):

```
10,000 snapshots ÷ 100 Hz = 100 seconds of history stored
```

You can increase or decrease this limit per joint:

```cpp
ContinuousJointTrail wheel_left {"wheel_left", 0.0, 0.15, 0.05, 50000};
//                                                                ^^^^^
//                                                         50,000 snapshots
```

---

## Summary Table

|Joint|Type|Recorded|Value Stored|Class|
|---|---|---|---|---|
|lidar_front|Fixed|Once at startup|Static pose|FixedJointTrail|
|lidar_left|Fixed|Once at startup|Static pose|FixedJointTrail|
|lidar_right|Fixed|Once at startup|Static pose|FixedJointTrail|
|imu|Fixed|Once at startup|Static pose|FixedJointTrail|
|wheel_left|Continuous|Every loop|Angle (radians)|ContinuousJointTrail|
|wheel_right|Continuous|Every loop|Angle (radians)|ContinuousJointTrail|
|linear_actuator|Linear|Every loop|Extension (meters)|LinearActuatorTrail|
|raster_sensor|Raster|Every scan|x, y position|RasterSensorTrail|

---

_Built for a custom robot platform using pure C++ on Linux._ _No ROS, No ROS2, No external dependencies._

```cpp
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

double timestamp; // seconds

double x, y, z; // position (meters)

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

size_t max_history; // limit memory usage

  

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

A.x + alpha * (B.x - A.x),

A.y + alpha * (B.y - A.y),

A.z + alpha * (B.z - A.z),

A.roll + alpha * (B.roll - A.roll),

A.pitch + alpha * (B.pitch - A.pitch),

A.yaw + alpha * (B.yaw - A.yaw)

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

std::cout << " t=" << s.timestamp

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

: TFTrail(name, 1) // only 1 record needed

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

double wheel_x, wheel_y, wheel_z; // fixed mount position

  

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

0, 0, angle_rad)); // yaw = wheel rotation

}

  

// Get interpolated wheel angle at target time

double getAngleAt(double target_time) {

return interpolate(target_time).yaw;

}

};

  

class LinearActuatorTrail : public TFTrail {

double base_x, base_y; // fixed base mount position

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

double fixed_z; // sensor stays at constant height

  

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

FixedJointTrail lidar_front {"lidar_front", 0.30, 0.00, 0.20, 0, 0, 0};

FixedJointTrail lidar_left {"lidar_left", 0.00, 0.20, 0.20, 0, 0, 1.5708};

FixedJointTrail lidar_right {"lidar_right", 0.00, -0.20, 0.20, 0, 0, -1.5708};

FixedJointTrail imu {"imu", 0.00, 0.00, 0.15, 0, 0, 0};

  

// Continuous Joints (Wheels)

ContinuousJointTrail wheel_left {"wheel_left", 0.00, 0.15, 0.05};

ContinuousJointTrail wheel_right {"wheel_right", 0.00, -0.15, 0.05};

  

// Linear Actuator

LinearActuatorTrail actuator {"linear_actuator", 0.0, 0.0, 0.50};

  

// Raster Sensor

RasterSensorTrail raster {"raster_sensor", 0.30};

  

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

robot.wheel_left.recordAngle(0.5 * i); // 0.5 rad per step

robot.wheel_right.recordAngle(0.48 * i); // slightly different

  

// Actuator extending slowly

robot.actuator.recordExtension(0.05 * i); // 5cm per step

  

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

double trail_end = now_seconds();

double query_time = (trail_start + trail_end) / 2.0; // midpoint

  

std::cout << "\nQuerying all joints at time t=" << query_time << "s\n";

  

try {

double lw = robot.wheel_left.getAngleAt(query_time);

double rw = robot.wheel_right.getAngleAt(query_time);

double ext = robot.actuator.getExtensionAt(query_time);

auto [rx, ry] = robot.raster.getPositionAt(query_time);

  

std::cout << " Wheel Left angle : " << lw << " rad\n";

std::cout << " Wheel Right angle : " << rw << " rad\n";

std::cout << " Actuator extension: " << ext << " m\n";

std::cout << " Raster position : (" << rx << ", " << ry << ") m\n";

  

std::cout << "\n Fixed joints (static — no interpolation needed):\n";

auto lidar = robot.lidar_front.getStaticPose();

std::cout << " Lidar Front pose : ("

<< lidar.x << ", " << lidar.y << ", " << lidar.z << ")\n";

  

} catch (const std::exception& e) {

std::cerr << "Error: " << e.what() << "\n";

}

  

robot.printAllTrails();

  

std::cout << "\n====== TF Trail System OK ======\n";

return 0;

}

```