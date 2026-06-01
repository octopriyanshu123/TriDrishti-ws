
## Roles in a Multi-Sensor / Multi-Controller Architecture

---

### Controller (e.g. `LinearActuatorController`, `prosimitySensorController`)

Each controller is responsible for **one specific hardware device or sensor**. Its job:

- **Init** — set up the device (open port, configure params, fork/exec driver process)
- **Read / Write** — send commands to or read data from that one device
- **State** — track the current state of that device (position, speed, error, etc.)
- **Error handling** — handle faults specific to that device (timeout, out-of-range, comm failure)
- **Shutdown** — safely stop and release that device

> Think of it as: _"I only know about my one device and I do it well."_

---

### ControllerManager

The manager sits **above all controllers** and knows nothing about hardware directly. Its job:

- **Lifecycle management** — calls `init()`, `start()`, `stop()` on all controllers in the right order
- **Coordination** — if Sensor A's reading affects Actuator B, the manager enforces that logic
- **Health monitoring** — polls controllers for errors and decides what to do (retry, stop, alert)
- **Single entry point** — the rest of your application talks only to the manager, never to individual controllers directly
- **Sequencing** — ensures controllers start/stop in the correct dependency order

> Think of it as: _"I don't touch hardware. I manage who does, when, and in what order."_

---

### Visual breakdown

```
         ┌──────────────────────────────┐
         │       ControllerManager      │  ← your app talks to this only
         │  - init() all controllers    │
         │  - coordinate between them   │
         │  - monitor health            │
         └──────┬──────────┬────────────┘
                │          │          │
     ┌──────────▼──┐  ┌────▼──────┐  ┌▼─────────────┐
     │  LinearAct  │  │  TempSens │  │  PressureSens │
     │  Controller │  │Controller │  │  Controller   │
     │  - 1 device │  │ - 1 device│  │  - 1 device   │
     └─────────────┘  └───────────┘  └───────────────┘
           │                │                │
        Actuator         Temp Sensor     Pressure Sensor
       (hardware)        (hardware)       (hardware)
```

---

### Simple rule of thumb

|Question|Who answers it|
|---|---|
|"Is the actuator at position X?"|`LinearActuatorController`|
|"What is the temperature?"|`TemperatureSensorController`|
|"Are all systems ready to start?"|`ControllerManager`|
|"If pressure is too high, stop the actuator"|`ControllerManager`|
|"Shut everything down safely"|`ControllerManager`|

---

So in your code: **controllers own devices, the manager owns controllers.**

[[Linear Actuator Controller]]




WHEN WE STARTY THE rOBOT ONLY oNE PROGRAM START 


# ControllerManager Api 
For the Manual and Auto Mode


| ControllerManager.init()      | Stm.init() | Fock New Process |
| ----------------------------- | ---------- | ---------------- |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
|                               |            |                  |
| ControllerManager.shutdown(); |            |                  |


