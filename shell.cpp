#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Struct to hold information about a background process
struct BackgroundProcess {
    pid_t pid;             // Process ID
    std::string command;   // Command string
};

// Function to execute a command
void executeCommand(const std::string& command, bool runInBackground, std::vector<BackgroundProcess>& backgroundProcesses);

// Function to split a string into tokens based on a delimiter
std::vector<std::string> splitString(const std::string& input, char delimiter);

// Function to display information about background processes
void showBackgroundProcesses(const std::vector<BackgroundProcess>& backgroundProcesses);

// Function to update and remove completed background processes
void updateBackgroundProcesses(std::vector<BackgroundProcess>& backgroundProcesses);

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
            updateBackgroundProcesses(backgroundProcesses);
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

/**
 * Executes a command either in the foreground or background.
 * @param command The command to execute.
 * @param runInBackground Boolean flag indicating whether the command should be executed in the background.
 * @param backgroundProcesses A vector of background processes.
 */
void executeCommand(const std::string& command, bool runInBackground, std::vector<BackgroundProcess>& backgroundProcesses) {
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to create child process" << std::endl;
        return;
    } else if (pid == 0) {
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

/**
 * Splits a string into tokens based on a delimiter.
 * @param input The input string to split.
 * @param delimiter The delimiter character.
 * @return A vector of tokens.
 */
std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * Displays information about background processes.
 * @param backgroundProcesses A vector of background processes.
 */
void showBackgroundProcesses(const std::vector<BackgroundProcess>& backgroundProcesses) {
    std::cout << "Background processes:" << std::endl;
    for (const auto& process : backgroundProcesses) {
        std::cout << "PID: " << process.pid << ", Command: " << process.command << std::endl;
    }
}

/**
 * Updates and removes completed background processes.
 * @param backgroundProcesses A vector of background processes.
 */
void updateBackgroundProcesses(std::vector<BackgroundProcess>& backgroundProcesses) {
    // Check and remove completed background processes
    for (auto it = backgroundProcesses.begin(); it != backgroundProcesses.end();) {
        pid_t result = waitpid(it->pid, nullptr, WNOHANG);
        if (result == -1) {
            // Error occurred while waiting for the process
            std::cerr << "Error occurred while waiting for process: " << it->command << std::endl;
            it = backgroundProcesses.erase(it);
        } else if (result == 0) {
            // Process is still running, move to the next process
            ++it;
        } else {
            // Process has completed
            std::cout << "Background process completed: " << it->command << std::endl;
            it = backgroundProcesses.erase(it);
            continue;  // Skip the increment operation since we have already erased the element
        }
    }
}
