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
    
    struct PipeCommand {
		std::string command;
		bool runInBackground;
	};

    void executeCommand(const std::string& command, bool runInBackground);
    std::vector<std::string> splitString(const std::string& input, char delimiter);
    void showBackgroundProcesses();
    void updateBackgroundProcesses();

    std::vector<BackgroundProcess> backgroundProcesses;

    // New helper function to execute piped commands
    void executePipedCommands(const std::string& command);

    // New helper function to handle writing to the pipe
    void writeToPipe(int pipefd[], int inputFd);

    // New helper function to handle reading from the pipe
    void readFromPipe(int pipefd[], int outputFd);
    
    void executePipeline(const std::vector<PipeCommand>& commands, std::vector<BackgroundProcess>& backgroundProcesses) ;

};
