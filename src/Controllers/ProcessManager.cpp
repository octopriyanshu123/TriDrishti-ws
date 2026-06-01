#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

class ProcessManager
{
public:
    pid_t forkProcess(const std::string &executable_path)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            handleForkError("fork() failed");
        }

        if (pid == 0)
        {
            char *const argv[] = {
                const_cast<char *>(executable_path.c_str()),
                nullptr};
            execv(executable_path.c_str(), argv);
            _exit(127); 
        }

        return pid;
    }

    void terminateProcess(pid_t pid)
    {
        if (pid > 0)
        {
            kill(pid, SIGTERM);
            int status;
            waitpid(pid, &status, 0);
        }
    }

    bool isProcessRunning(pid_t pid) const
    {
        return kill(pid, 0) == 0;
    }

    void handleForkError(const std::string &context)
    {
        throw std::runtime_error(context + ": " + std::strerror(errno));
    }
};
