#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <ctime>
#include "ProcessManager.cpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

class STMController
{
public:
    explicit STMController(const std::string &driver_path)
        : driver_path_(driver_path), stm_pid_(-1), pipe_write_fd_(-1) {}

    pid_t init()
    {
        std::cout << " [STMController] init()\n";
        ProcessManager pm;
        pid_t pid = pm.forkProcess(driver_path_);
        if (pid == 0) return 0;
        stm_pid_ = pid;
        return pid;
    }

    void config(){

    }
    void shutdown()
    {
        std::cout << " [STMController] shutdown()\n";

        if (pipe_write_fd_ >= 0)
        {
            close(pipe_write_fd_); // EOF → child exits cleanly
            pipe_write_fd_ = -1;
        }
        if (stm_pid_ > 0)
        {
            int status = 0;
            waitpid(stm_pid_, &status, 0);
            stm_pid_ = -1;
        }
    }

private:
    std::string driver_path_;
    pid_t stm_pid_;
    int pipe_write_fd_;

    // LinearActuatorController lac;
    // ProximityController psc;
    // StepperController smc;
    
};

// class LinearActuatorController
// {
// public:
//     explicit LinearActuatorController(STMController &stm) : stm_(stm) {}

//     void init()
//     {
//         std::cout << "  [LinearActuator] init()\n";
//     }

//     void set_position(double mm)
//     {
//         std::ostringstream cmd;
//         cmd << "LINEAR_ACTUATOR SET_POSITION " << mm;
//         stm_.send_command(cmd.str());
//     }

//     void stop()
//     {
//         stm_.send_command("LINEAR_ACTUATOR STOP");
//     }

// private:
// };

// class StepperController
// {
// public:
//     explicit StepperController(STMController &stm) : stm_(stm) {}

//     void init()
//     {
//         std::cout << "  [Stepper] init()\n";
//     }

//     void move_steps(int steps, int rpm)
//     {
//         std::ostringstream cmd;
//         cmd << "STEPPER MOVE " << steps << " " << rpm;
//         stm_.send_command(cmd.str());
//     }

//     void stop()
//     {
//         stm_.send_command("STEPPER STOP");
//     }

// private:
// };

// class ProximityController
// {
// public:
//     explicit ProximityController(STMController &stm) : stm_(stm) {}

//     void init()
//     {
//         std::cout << "  [Proximity] init()\n";
//     }

//     double read_distance_mm()
//     {
//         stm_.send_command("PROXIMITY READ");
//         // Real code: read response from driver via response pipe / shared memory
//         double val = 42.0; // placeholder
//         return val;
//     }

// private:
// };
