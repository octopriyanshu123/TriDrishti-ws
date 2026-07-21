#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/prctl.h>

#include <atomic>
#include <csignal>
#include <iostream>

class ProcessManager
{
public:
    static pid_t start(const std::string &exePath)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            // Execute the real program
            execl(exePath.c_str(), exePath.c_str(), nullptr);
            perror("execl");
            _exit(1);
        }
        return pid;
    }

    static pid_t findByName(const std::string &targetName)
    {
        for (const auto &entry :
             std::filesystem::directory_iterator("/proc"))
        {
            if (!entry.is_directory())
                continue;

            std::string pidStr =
                entry.path().filename().string();

            if (!std::all_of(pidStr.begin(),
                             pidStr.end(),
                             ::isdigit))
                continue;

            std::ifstream file(entry.path() / "comm");

            if (!file)
                continue;

            std::string processName;
            std::getline(file, processName);

            if (processName == targetName)
                return std::stoi(pidStr);
        }

        return -1;
    }

    static pid_t findByExecutable(const std::string &targetExe)
    {
        char linkPath[PATH_MAX];
        char exePath[PATH_MAX];

        for (const auto &entry :
             std::filesystem::directory_iterator("/proc"))
        {
            if (!entry.is_directory())
                continue;

            std::string pidStr =
                entry.path().filename().string();

            if (!std::all_of(pidStr.begin(),
                             pidStr.end(),
                             ::isdigit))
                continue;

            snprintf(linkPath,
                     sizeof(linkPath),
                     "/proc/%s/exe",
                     pidStr.c_str());

            ssize_t len =
                readlink(linkPath,
                         exePath,
                         sizeof(exePath) - 1);

            if (len <= 0)
                continue;

            exePath[len] = '\0';

            if (targetExe == exePath)
                return std::stoi(pidStr);
        }

        return -1;
    }

    static bool stop(pid_t pid)
    {
        return kill(pid, SIGTERM) == 0;
    }

    static bool forceStop(pid_t pid)
    {
        return kill(pid, SIGKILL) == 0;
    }

    static bool stopByName(const std::string &name)
    {
        pid_t pid = findByName(name);

        if (pid <= 0)
            return false;

        return stop(pid);
    }

    static void printInfo(pid_t pid)
    {
        std::string base =
            "/proc/" + std::to_string(pid);

        std::ifstream file(base + "/comm");

        std::string name;
        std::getline(file, name);

        char exePath[PATH_MAX];

        ssize_t len =
            readlink((base + "/exe").c_str(),
                     exePath,
                     sizeof(exePath) - 1);

        if (len > 0)
            exePath[len] = '\0';
        else
            strcpy(exePath, "Unknown");

        std::cout << "PID  : " << pid << '\n';
        std::cout << "Name : " << name << '\n';
        std::cout << "Exe  : " << exePath << '\n';
    }
};

std::atomic<bool> running{true};
static void onSignal(int) { running.store(false); }

int main()
{

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    pid_t pid = ProcessManager::start("/usr/bin/gedit");
    std::cout << "Created PID: "
              << pid
              << std::endl;

    while (running)
    {
        sleep(1);
    }

    ProcessManager::printInfo(pid);

    pid_t found = ProcessManager::findByExecutable(
        "/usr/bin/gedit");

    std::cout << "Found PID: "
              << found
              << std::endl;

    ProcessManager::stop(pid);

    return 0;
}