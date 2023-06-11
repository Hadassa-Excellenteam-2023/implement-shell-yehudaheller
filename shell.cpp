#include "shell.h"
#include <fcntl.h>  // for file descriptor manipulation

Shell::Shell() {}

void Shell::run() {
    std::string input;

    while (true) {
        std::cout << "Shell> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (input == "myjobs") {
            updateBackgroundProcesses();
            showBackgroundProcesses();
            continue;
        }

        // Check if the command should be run in the background
        bool runInBackground = false;
        if (!input.empty() && input.back() == '&') {
            runInBackground = true;
            input.pop_back();
        }

        executeCommand(input, runInBackground);
    }
}

void Shell::executeCommand(const std::string& command, bool runInBackground) {
    std::vector<std::string> args = splitString(command, ' ');

    // Check if the command should be run in the background
    if (!args.empty() && args.back() == "&") {
        runInBackground = true;
        args.pop_back();
    }

    // Check if input/output redirection is requested
    std::string inputFile;
    std::string outputFile;
    bool inputRedirection = false;
    bool outputRedirection = false;
    size_t inputIndex = 0;
    size_t outputIndex = 0;

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "<") {
            if (i + 1 < args.size()) {
                inputRedirection = true;
                inputFile = args[i + 1];
                inputIndex = i;
            }
        } else if (args[i] == ">") {
            if (i + 1 < args.size()) {
                outputRedirection = true;
                outputFile = args[i + 1];
                outputIndex = i;
            }
        }
    }

    // Remove the redirection symbols and file names from the arguments
    if (inputRedirection) {
        args.erase(args.begin() + inputIndex, args.begin() + inputIndex + 2);
    }
    if (outputRedirection) {
        args.erase(args.begin() + outputIndex, args.begin() + outputIndex + 2);
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to create child process" << std::endl;
        return;
    } else if (pid == 0) {
        // Child process
        // Handle input redirection
        if (inputRedirection) {
            int inputFileDescriptor = open(inputFile.c_str(), O_RDONLY);
            if (inputFileDescriptor == -1) {
                std::cerr << "Failed to open input file: " << inputFile << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(inputFileDescriptor, STDIN_FILENO);
            close(inputFileDescriptor);
        }

        // Handle output redirection
        if (outputRedirection) {
            int outputFileDescriptor = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outputFileDescriptor == -1) {
                std::cerr << "Failed to open output file: " << outputFile << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(outputFileDescriptor, STDOUT_FILENO);
            close(outputFileDescriptor);
        }

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

std::vector<std::string> Shell::splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void Shell::showBackgroundProcesses() {
    std::cout << "Background processes:" << std::endl;
    for (const auto& process : backgroundProcesses) {
        std::cout << "PID: " << process.pid << ", Command: " << process.command << std::endl;
    }
}

void Shell::updateBackgroundProcesses() {
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
