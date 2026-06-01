[[Lifecycle Base Class]]
# Hardware Control System Architecture

### Akshay — Hardware Layer (STM-Based Embedded System)

**Responsibilities:**

* Development of the low-level embedded hardware control layer on STM microcontrollers
* Implementation of UART-based ModBus communication interface
* Hardware communication optimization and reliability handling
* Real-time actuator and sensor interfacing

**Communication Interface:**

* UART ModBus
* Maximum baud rate: **115200**

---

### Priyanshu — Hardware Abstraction Layer (HAL)  (Jetson Platform)

**Responsibilities:**

* Development of the Hardware Abstraction Layer (HAL)  on NVIDIA Jetson
* Creation of unified APIs for upper management/control layers
* Device abstraction and hardware-independent control architecture
* Continuous feedback acquisition and monitoring

**Managed Hardware Components:**

* Stepper Motor
* Linear Actuator
* Proximity Sensor

---

# System Objectives

## 1\. Unified API for the Management Layer

Provide a standardized and extensible software interface for higher-level applications and control systems to interact with hardware devices without direct dependency on embedded implementation details.

## 2\. Hardware Control and Coordination

Enable reliable command transmission and execution for:

* Motion control
* Actuator management
* Sensor integration
* Device state synchronization

## 3\. Continuous Feedback and Monitoring

Implement continuous bidirectional communication to:

* Monitor hardware states in real time
* Track actuator positions and sensor status
* Detect faults and communication failures
* Provide telemetry and diagnostics to upper layers

---

# Proposed System Flow

```text
Management Layer
        │
        ▼
Hardware Abstraction Layer (Jetson)
        │
 UART ModBus (115200 Baud)
        │
        ▼
STM Hardware Layer
        │
 ┌───────────────┬───────────────┬
 ▼               ▼               ▼
Stepper Motor  Linear Actuator  Proximity Sensor
```

# Potential Hardware Management APIs

\**Note: APIs and interfaces are subject to change after development, hardware integration, and validation testing.*

## 1\. Linear Actuator API

Control linear actuator position.

```cpp
bool set(float percentage);
bool set(int id, float percentage);
float getPosition();
float getPosition(int id);
bool isMoving(int id);
```

Continuous streaming feedback containing:

* Current position
* Motion state
* Error state
* Limit switch status
* Communication status

---

# 2\. Stepper Motor API

Control Stepper Motor Steps.

```cpp
bool set(int steps);
bool set(int id, int steps);
bool setVelocity(int rpm);
bool stop(int id);
bool home(int id);
int getPosition(int id);
int getVelocity(int id);
bool isRunning(int id);
```

Continuous streaming feedback containing:

```cpp
MotorFeedback subscribeFeedback();
```

* Current position
* Velocity
* Direction
* Driver status
* Error codes
* Motion completion state

---

# 3\. Proximity Sensor API

Continuous environment/object detection feedback.

```cpp
bool getState(int id);
float getDistance(int id);
```

Continuous streaming feedback containing:

```cpp
SensorFeedback subscribeFeedback();
```

* Detection state
* Distance
* Signal quality
* Sensor health
* Communication status

---

# 4\. Common System APIs

## Device Management

```cpp
bool initialize();             // Initialize hardware communication layer.
bool shutdown();               //Safely stop all hardware devices.
bool resetDevice(int id);      // Reset selected hardware module.
DeviceStatus getStatus(int id);// Returns device health and communication state.
emergencyStop();               //Safety APIs
onFeedbackReceived()           // Event Callbacks
onError() 
onConnectionLost()
asyncSet()                     //Async Control
asyncHome()
```

---

# 5\. Challenges

* UART ModBus RTU
* Baud Rate: `115200`
* Continuous bidirectional feedback
* Non-blocking communication
* Device-level fault isolation
* Timeout and retry handling
* Scalable multi-device architecture
* Hardware integration validation
* Communication stability testing
* Real-time feedback verification
* Stress and fault-condition testing
* Multi-device synchronization testing
* API behavior standardization
* Embedded and Jetson interoperability validation

---