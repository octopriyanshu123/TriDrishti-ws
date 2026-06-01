#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

// ─────────────────────────────────────────────
//  LinearActuatorController
// ─────────────────────────────────────────────
class LinearActuatorController {
public:
    explicit LinearActuatorController(const std::string& program_path)
        : program_path_(program_path), child_pid_(-1) {}

    // Initialise: fork a child process and exec the program at program_path_
    void init() {
        std::cout << "[LinearActuatorController] init() called. "
                  << "Forking and executing: " << program_path_ << "\n";

        pid_t pid = fork();

        if (pid < 0) {
            // fork failed
            throw std::runtime_error(
                std::string("[LinearActuatorController] fork() failed: ") +
                std::strerror(errno));
        }

        if (pid == 0) {
            // ── Child process ──────────────────────────────────────────
            // Replace this process image with the target program.
            // Pass the program path as argv[0]; add more args as needed.
            char* const argv[] = {
                const_cast<char*>(program_path_.c_str()),
                nullptr
            };

            execv(program_path_.c_str(), argv);

            // execv only returns on error
            std::cerr << "[child] execv failed: " << std::strerror(errno) << "\n";
            _exit(EXIT_FAILURE);          // use _exit in child after failed exec
        }

        // ── Parent process ─────────────────────────────────────────────
        child_pid_ = pid;
        std::cout << "[LinearActuatorController] Child PID: " << child_pid_ << "\n";
    }

    // Wait for the child to finish and return its exit code
    int wait_for_child() {
        if (child_pid_ < 0) {
            std::cerr << "[LinearActuatorController] No child process to wait for.\n";
            return -1;
        }

        int status = 0;
        pid_t result = waitpid(child_pid_, &status, 0);

        if (result < 0) {
            throw std::runtime_error(
                std::string("[LinearActuatorController] waitpid() failed: ") +
                std::strerror(errno));
        }

        child_pid_ = -1;

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            std::cout << "[LinearActuatorController] Child exited with code: "
                      << exit_code << "\n";
            return exit_code;
        }

        if (WIFSIGNALED(status)) {
            std::cout << "[LinearActuatorController] Child killed by signal: "
                      << WTERMSIG(status) << "\n";
            return -1;
        }

        return -1;
    }

    pid_t child_pid() const { return child_pid_; }

private:
    std::string program_path_;
    pid_t       child_pid_;
};


// ─────────────────────────────────────────────
//  ControllerManager
// ─────────────────────────────────────────────
class ControllerManager {
public:
    // program_path: absolute path to the executable to launch
    explicit ControllerManager(const std::string& program_path)
        : actuator_ctrl_(program_path) {}

    // Call init() on all managed controllers
    void init() {
        std::cout << "[ControllerManager] Initialising controllers...\n";
        actuator_ctrl_.init();
        std::cout << "[ControllerManager] All controllers initialised.\n";
    }

    // Block until the launched program finishes
    int run() {
        return actuator_ctrl_.wait_for_child();
    }

    LinearActuatorController& actuator_controller() {
        return actuator_ctrl_;
    }

private:
    LinearActuatorController actuator_ctrl_;
};


