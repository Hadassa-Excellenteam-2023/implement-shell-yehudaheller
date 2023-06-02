#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Struct to hold information about a background process
struct BackgroundProcess {
    pid_t pid;
    std::string command;
};

// Function to execute a command
void executeCommand(const std::string& command, bool runInBackground, std::vector<BackgroundProcess>& backgroundProcesses);
std::vector<std::string> splitString(const std::string& input, char delimiter);
void showBackgroundProcesses(const std::vector<BackgroundProcess>& backgroundProcesses);

int main() {
    std::string input;
    std::vector<BackgroundProcess> backgroundProcesses;

    while (true) {
        std::cout << "Shell> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (input == "myjobs") {
            showBackgroundProcesses(backgroundProcesses);
            continue;
        }

        // Check if the command should be run in the background
        bool runInBackground = false;
        if (!input.empty() && input.back() == '&') {
            runInBackground = true;
            input.pop_back();
        }

        executeCommand(input, runInBackground, backgroundProcesses);
    }

    return 0;
}

void executeCommand(const std::string& command, bool runInBackground, std::vector<BackgroundProcess>& backgroundProcesses) {
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to create child process" << std::endl;
        return;
    } 
    
    else if (pid == 0) {
        // Child process
        std::vector<std::string> args = splitString(command, ' ');

        // Convert vector of strings to char* array
        std::vector<char*> cArgs;
        for (const auto& arg : args) {
            cArgs.push_back(const_cast<char*>(arg.c_str()));
        }
        cArgs.push_back(nullptr); // Add a null terminator at the end

        execvp(cArgs[0], cArgs.data());

        // execvp will only return if an error occurred
        std::cerr << "Failed to execute command: " << command << std::endl;
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        if (runInBackground) {
            BackgroundProcess process;
            process.pid = pid;
            process.command = command;
            backgroundProcesses.push_back(process);
            std::cout << "Background process started: " << command << std::endl;
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (status != 0) {
                std::cerr << "Command exited with non-zero status: " << command << std::endl;
            }
        }
    }
}

std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void showBackgroundProcesses(const std::vector<BackgroundProcess>& backgroundProcesses) {
    std::cout << "Background processes:" << std::endl;
    for (const auto& process : backgroundProcesses) {
        std::cout << "PID: " << process.pid << ", Command: " << process.command << std::endl;
    }
}
