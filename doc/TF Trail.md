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