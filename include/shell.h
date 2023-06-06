#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

class Shell {
public:
    Shell();
    void run();

private:
    struct BackgroundProcess {
        pid_t pid;             // Process ID
        std::string command;   // Command string
    };

    void executeCommand(const std::string& command, bool runInBackground);
    std::vector<std::string> splitString(const std::string& input, char delimiter);
    void showBackgroundProcesses();
    void updateBackgroundProcesses();

    std::vector<BackgroundProcess> backgroundProcesses;
};


