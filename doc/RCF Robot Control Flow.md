i2w for the data flow on network and local
Data flow is the continious process UDP
and Control Flow is the secure Process on the tcp
we shoud Seprate the Data Flow an Control Flow
event 

It work on the both network and local 
Two type of the Action and Service

# RCF — Robot Communication Framework

C++17 library for typed request-response services and long-running actions between a robot and a control PC. Supports Unix sockets (same machine) and TCP (network).

---

## Include

```cpp
#include "rcf.hpp"    // only file you need
```

---

## Wire Types

Every struct sent over the wire must be:

- **Trivially copyable** (POD — no `std::string`, no vectors, no pointers)
- Wrapped in `#pragma pack(push,1)` / `#pragma pack(pop)`

```cpp
#pragma pack(push, 1)

struct SetPowerReq  { int32_t power; };
struct AckRes       { uint8_t ok; };      // 1 = success, 0 = rejected

struct NavGoal      { float x, y; };
struct NavFeedback  { float progress; float eta_sec; };
struct NavResult    { uint8_t reached; float final_x; float final_y; };

#pragma pack(pop)
```

Use a fixed-size char array for text:

```cpp
#pragma pack(push, 1)
struct StrVal { char data[64]{}; };       // instead of std::string
#pragma pack(pop)
```

---

## Registering Types (Explicit Instantiation)

The library ships with empty template bodies — it does not know your types. You register them once in a file called `rcf_instantiate.cpp` in your project.

**`rcf_instantiate.cpp`**

```cpp
// Pull in the template bodies
#include "service/ServiceServer.cpp"
#include "service/ServiceClient.cpp"

// For actions, also include:
// #include "action/ActionServer.cpp"
// #include "action/ActionClient.cpp"
// #include "action/GoalHandle.cpp"

#include "your_types.hpp"

namespace rcf {
    // One pair of lines per service type
    template class ServiceServer<SetPowerReq, AckRes>;
    template class ServiceClient<SetPowerReq, AckRes>;

    // One group of four lines per action type triple
    // template class ActionServer<NavGoal, NavFeedback, NavResult>;
    // template class ActionClient<NavGoal, NavFeedback, NavResult>;
    // template class GoalHandle  <NavFeedback, NavResult>;
    // template struct ClientGoalHandle<NavFeedback, NavResult>;
}
```

Add `rcf_instantiate.cpp` as a source file to every CMake target that uses the library.

---

## Local Service — Server

> Unix socket. Robot and control PC on the same machine.

**Call chain:**

```
makeUnix(path)
    └── bind(handler)
            └── spinAsync()
                    └── [accepts connections in background thread]
                            └── handler(req) → res
shutdown()
```

**Full API:**

```cpp
// 1. Create — opens the socket at the given path
static std::unique_ptr<ServiceServer<Req, Res>>
    ServiceServer<Req, Res>::makeUnix(const std::string& path);

// 2. Bind — register the handler function (must call before spinAsync)
//    Handler signature:  Res fn(const Req&)
void bind(std::function<Res(const Req&)> handler);

// 3. Start — launches the accept loop in a background thread (non-blocking)
void spinAsync();

// 4. Stop — closes the socket and joins the background thread
void shutdown();
```

**Code:**

```cpp
static AckRes on_power(const SetPowerReq& req)
{
    AckRes res;
    if (req.power < 0 || req.power > 100) { res.ok = 0; return res; }
    g_power = req.power;
    res.ok = 1;
    return res;
}

auto srv = rcf::ServiceServer<SetPowerReq, AckRes>::makeUnix("/tmp/svc_power");
srv->bind(on_power);
srv->spinAsync();

// ... run ...

srv->shutdown();
```

---

## Local Service — Client

**Call chain:**

```
ServiceClient(path)
    └── call(req)
            └── [opens socket → sends req → waits for res → closes socket]
            └── returns res
```

**Full API:**

```cpp
// 1. Create — connects to the Unix socket at path
//    timeout_ms  — connection + receive timeout (default 5000)
explicit ServiceClient<Req, Res>(const std::string& path, int timeout_ms = 5000);

// 2. Call — blocking: opens connection, sends Req, receives Res, closes
//    Throws std::runtime_error on transport failure
Res call(const Req& req, int timeout_ms = -1);

// 3. tryCall — exception-free variant
//    Writes result into `out`, returns ServiceStatus::OK / ERROR / TIMEOUT
ServiceStatus tryCall(const Req& req, Res& out, int timeout_ms = -1);
```

**Code:**

```cpp
rcf::ServiceClient<SetPowerReq, AckRes> cli("/tmp/svc_power");

SetPowerReq req;
req.power = 75;
AckRes res = cli.call(req);
// res.ok == 1 → success

// Exception-free variant:
AckRes out;
rcf::ServiceStatus st = cli.tryCall(req, out);
if (st == rcf::ServiceStatus::TIMEOUT) { /* handle */ }
```

---

## Local Action — Server

> Unix socket. Long-running task with feedback and cancel support. Creates three sockets: `<base>_goal`, `<base>_cancel`, `<base>_result`

**Call chain:**

```
makeUnix(base)
    └── onGoalCheck(fn)          [optional — reject before executing]
    └── onExecute(fn)            [mandatory — runs in a thread per goal]
            └── spinAsync()
                    └── [accepts goals, cancel requests, result requests]
                            └── fn(goal, handle)
                                    ├── handle->publishFeedback(fb)
                                    ├── handle->isCancelRequested()
                                    ├── handle->setSucceeded(result)
                                    └── handle->setCanceled(result)
shutdown()
```

**Full API:**

```cpp
// 1. Create
static std::unique_ptr<ActionServer<Goal, Feedback, Result>>
    ActionServer<Goal, Feedback, Result>::makeUnix(const std::string& base);

// 2. Goal check (optional) — called before execute; return false to reject
void onGoalCheck(std::function<bool(const Goal&)> fn);

// 3. Execute (mandatory) — runs in a new thread for each accepted goal
//    HandlePtr is std::shared_ptr<GoalHandle<Feedback, Result>>
void onExecute(std::function<void(const Goal&, HandlePtr)> fn);

// Inside the execute function, use the handle:
void handle->publishFeedback(const Feedback& fb);  // send progress to client
bool handle->isCancelRequested();                  // true when client cancelled
void handle->setSucceeded(const Result& r);        // terminal: success
void handle->setCanceled (const Result& r);        // terminal: cancelled
void handle->setAborted  (const Result& r);        // terminal: error

// 4. Start
void spinAsync();

// 5. Stop
void shutdown();
```

**Code:**

```cpp
static bool check_nav(const NavGoal& g)
{
    return (g.x >= -20.f && g.x <= 20.f && g.y >= -20.f && g.y <= 20.f);
}

static void exec_nav(const NavGoal& goal,
                     rcf::ActionServer<NavGoal, NavFeedback, NavResult>::HandlePtr handle)
{
    for (int i = 0; i < 20; ++i) {
        if (handle->isCancelRequested()) {
            NavResult r; r.reached = 0; r.final_x = 0; r.final_y = 0;
            handle->setCanceled(r);
            return;
        }
        NavFeedback fb; fb.progress = float(i + 1) / 20; fb.eta_sec = float(20 - i) * 0.1f;
        handle->publishFeedback(fb);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    NavResult r; r.reached = 1; r.final_x = goal.x; r.final_y = goal.y;
    handle->setSucceeded(r);
}

auto srv = rcf::ActionServer<NavGoal, NavFeedback, NavResult>
               ::makeUnix("/tmp/act_nav");
srv->onGoalCheck(check_nav);
srv->onExecute(exec_nav);
srv->spinAsync();

// ...

srv->shutdown();
```

---

## Local Action — Client

**Call chain:**

```
ActionClient(base)
    └── sendGoal(goal, feedbackCb, statusCb)
            └── [sends goal → receives ACK → returns handle immediately]
            └── [background thread waits for result]
                    └── feedbackCb(id, fb)   [called on each publishFeedback]
                    └── statusCb(id, status) [called once on terminal state]
    └── cancel(handle)           [optional — sends cancel request]
    └── handle->waitForResult(timeout_ms)
            └── returns std::pair<GoalStatus, Result>
```

**Full API:**

```cpp
// 1. Create — connects to base_goal, base_cancel, base_result
explicit ActionClient<Goal, Feedback, Result>(const std::string& base,
                                               int timeout_ms = 5000);

// 2. Send goal — non-blocking, returns handle immediately
//    fb_cb   optional: void fn(const GoalID&, const Feedback&)
//    st_cb   optional: void fn(const GoalID&, GoalStatus)
CGHPtr sendGoal(const Goal& goal,
                FeedbackCb  fb_cb = nullptr,
                StatusCb    st_cb = nullptr);

// 3. Cancel — sends cancel request; returns true if server acknowledged
bool cancel(CGHPtr handle);

// 4. Wait — blocks until SUCCEEDED / CANCELED / ABORTED
//    Returns std::pair<GoalStatus, Result>
//    Throws std::runtime_error("waitForResult timeout") on timeout
std::pair<GoalStatus, Result> handle->waitForResult(int timeout_ms = -1);

// 5. Poll status without blocking
GoalStatus handle->getStatus();
```

**Code:**

```cpp
static void on_feedback(const rcf::GoalID& id, const NavFeedback& fb)
{
    printf("progress %.0f%%  eta %.1fs\n", fb.progress * 100.f, fb.eta_sec);
}

static void on_status(const rcf::GoalID& id, rcf::GoalStatus s)
{
    printf("final: %s\n", rcf::goalStatusStr(s));
}

rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli("/tmp/act_nav");

NavGoal goal; goal.x = 5.f; goal.y = 3.f;

rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
    cli.sendGoal(goal, on_feedback, on_status);

// optional cancel:
// cli.cancel(h);

std::pair<rcf::GoalStatus, NavResult> r = h->waitForResult(15000);
printf("reached=%s\n", r.second.reached ? "yes" : "no");
```

---

## Network Service — Server

> TCP. Robot binds a port; control PC connects from any machine. Identical to Local Service — only the factory changes.

**Difference from local:**

```cpp
// Local
rcf::ServiceServer<SetPowerReq, AckRes>::makeUnix("/tmp/svc_power");

// Network
rcf::ServiceServer<SetPowerReq, AckRes>::makeTcp(9100);
```

**Full API:**

```cpp
// Creates a TCP server on the given port
static std::unique_ptr<ServiceServer<Req, Res>>
    ServiceServer<Req, Res>::makeTcp(uint16_t port);

// bind / spinAsync / shutdown — identical to local service
```

**Code:**

```cpp
auto srv = rcf::ServiceServer<SetPowerReq, AckRes>::makeTcp(9100);
srv->bind(on_power);
srv->spinAsync();
// ...
srv->shutdown();
```

---

## Network Service — Client

**Difference from local:**

```cpp
// Local
rcf::ServiceClient<SetPowerReq, AckRes> cli("/tmp/svc_power");

// Network
rcf::ServiceClient<SetPowerReq, AckRes> cli("192.168.1.50", 9100);
```

**Full API:**

```cpp
// Connects to host:port over TCP
ServiceClient<Req, Res>(const std::string& host, uint16_t port,
                         int timeout_ms = 5000);

// call / tryCall — identical to local service
```

**Code:**

```cpp
rcf::ServiceClient<SetPowerReq, AckRes> cli("192.168.1.50", 9100);

SetPowerReq req; req.power = 75;
AckRes res = cli.call(req);
```

---

## Network Action — Server

> TCP. Three consecutive ports are used automatically. `port_base` = goal, `port_base+1` = cancel, `port_base+2` = result

**Difference from local:**

```cpp
// Local
rcf::ActionServer<NavGoal, NavFeedback, NavResult>::makeUnix("/tmp/act_nav");

// Network
rcf::ActionServer<NavGoal, NavFeedback, NavResult>::makeTcp(9200);
// opens 9200 (goal) / 9201 (cancel) / 9202 (result)
```

**Full API:**

```cpp
// Creates three TCP servers at port_base, port_base+1, port_base+2
static std::unique_ptr<ActionServer<Goal, Feedback, Result>>
    ActionServer<Goal, Feedback, Result>::makeTcp(uint16_t port_base);

// onGoalCheck / onExecute / spinAsync / shutdown — identical to local action
```

**Code:**

```cpp
auto srv = rcf::ActionServer<NavGoal, NavFeedback, NavResult>::makeTcp(9200);
srv->onGoalCheck(check_nav);
srv->onExecute(exec_nav);
srv->spinAsync();
// ...
srv->shutdown();
```

---

## Network Action — Client

**Difference from local:**

```cpp
// Local
rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli("/tmp/act_nav");

// Network
rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli("192.168.1.50", 9200);
// connects to 9200 (goal) / 9201 (cancel) / 9202 (result)
```

**Full API:**

```cpp
// Connects to host at port_base, port_base+1, port_base+2
ActionClient<Goal, Feedback, Result>(const std::string& host, uint16_t port_base,
                                      int timeout_ms = 5000);

// sendGoal / cancel / waitForResult — identical to local action
```

**Code:**

```cpp
rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli("192.168.1.50", 9200);

NavGoal goal; goal.x = 5.f; goal.y = 3.f;

rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
    cli.sendGoal(goal, on_feedback, on_status);

std::pair<rcf::GoalStatus, NavResult> r = h->waitForResult(15000);
```

---

## GoalStatus values

|Value|Meaning|
|---|---|
|`IDLE`|Not yet sent|
|`ACCEPTED`|Server accepted the goal|
|`REJECTED`|Server rejected in `onGoalCheck`|
|`EXECUTING`|Worker thread is running|
|`CANCELING`|Cancel requested, worker hasn't confirmed yet|
|`SUCCEEDED`|Worker called `setSucceeded`|
|`CANCELED`|Worker called `setCanceled`|
|`ABORTED`|Worker threw an exception|

```cpp
const char* rcf::goalStatusStr(rcf::GoalStatus s);    // "SUCCEEDED" etc.
```

---

## ServiceStatus values

|Value|Meaning|
|---|---|
|`OK`|Call completed successfully|
|`ERROR`|Transport or handler error|
|`TIMEOUT`|No response within timeout_ms|

---

## Rules

- Every wire struct must be trivially copyable — no `std::string`, no vectors, no pointers.
- Use `#pragma pack(push,1)` / `#pragma pack(pop)` around all wire structs.
- Always call `bind()` before `spinAsync()` — violating this throws `std::runtime_error`.
- Always call `onExecute()` before `spinAsync()` on `ActionServer` — same rule.
- The execute function (`onExecute`) runs in a **separate thread per goal** — use thread-safe access to shared state.
- `ActionClient::sendGoal()` is non-blocking — it returns a handle immediately and waits for the result in a background thread.
- For text fields use a fixed-size char array (`char data[64]`) not `std::string`.

spin()
│
├── Guard — throws if bind() was never called
│
├── srv_->listen()
│       └── opens the socket file on disk, starts accepting connections
│
├── running_ = true
│
└── while(running_)
        │
        ├── srv_->accept()          ← BLOCKS here until a client connects
        │       └── returns a unique_ptr<ITransport> for that one client
        │
        ├── if !running_ → break    ← shutdown() closed the socket, wake up
        │
        └── std::thread(...).detach()
                └── spawns a new thread for this client
                        └── handle(client)
                                ├── recvFrame()     ← read the request bytes
                                ├── decodeStruct<Req>()
                                ├── handler_(req)   ← your function runs here
                                ├── encodeStruct(res)
                                └── sendFrame()     ← send response back

        [main loop goes back to accept() — waits for next client]
│
└── srv_->close()    ← runs when running_ becomes false