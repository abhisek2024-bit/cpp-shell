#include "shell.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <csignal>
#include <algorithm>

using namespace std;

void Shell::run() {
    string input;
    while (true) {
        cout << "myshell> ";
        getline(cin, input);
        if (input.empty()) continue;
        if (input == "exit") break;

        vector<string> args = parseInput(input);
        if (args.empty()) continue;

        if (args[0] == "jobs") {
            checkJobs();
            continue;
        } else if (args[0] == "fg" && args.size() == 2) {
            pid_t pid = static_cast<pid_t>(stoi(args[1]));
            bringToForeground(pid);
            continue;
        } else if (args[0] == "bg" && args.size() == 2) {
            pid_t pid = static_cast<pid_t>(stoi(args[1]));
            continueInBackground(pid);
            continue;
        }

        bool background = false;
        if (args.back() == "&") {
            background = true;
            args.pop_back();
        }

        if (find(args.begin(), args.end(), "|") != args.end()) {
            handlePiping(args);
        } else {
            handleRedirection(args);
            executeCommand(args, background);
        }
    }
}

vector<string> Shell::parseInput(const string& input) {
    istringstream iss(input);
    string token;
    vector<string> tokens;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

void Shell::executeCommand(vector<string> args, bool background) {
    pid_t pid = fork();
    if (pid == 0) {
        vector<char*> c_args;
        for (auto& arg : args) c_args.push_back(&arg[0]);
        c_args.push_back(nullptr);
        execvp(c_args[0], c_args.data());
        perror("exec failed");
        exit(1);
    } else if (pid > 0) {
        if (background) {
            string full_cmd;
            for (const auto& arg : args) full_cmd += arg + " ";
            jobs[pid] = full_cmd;
            cout << "Started background job [" << pid << "]\n";
        } else {
            waitpid(pid, nullptr, 0);
        }
    } else {
        perror("fork failed");
    }
}

void Shell::handleRedirection(vector<string>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") {
            int fd = open(args[i+1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open for output failed");
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            break;
        } else if (args[i] == "<") {
            int fd = open(args[i+1].c_str(), O_RDONLY);
            if (fd < 0) {
                perror("open for input failed");
                return;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            break;
        }
    }
}

void Shell::handlePiping(vector<string>& args) {
    auto it = find(args.begin(), args.end(), "|");
    vector<string> cmd1(args.begin(), it);
    vector<string> cmd2(it + 1, args.end());
    launchPipe(cmd1, cmd2);
}

void Shell::launchPipe(vector<string> cmd1, vector<string> cmd2) {
    int pipefd[2];
    pipe(pipefd);
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        vector<char*> c_args;
        for (auto& arg : cmd1) c_args.push_back(&arg[0]);
        c_args.push_back(nullptr);
        execvp(c_args[0], c_args.data());
        perror("pipe exec 1 failed");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        vector<char*> c_args;
        for (auto& arg : cmd2) c_args.push_back(&arg[0]);
        c_args.push_back(nullptr);
        execvp(c_args[0], c_args.data());
        perror("pipe exec 2 failed");
        exit(1);
    }

    close(pipefd[0]); close(pipefd[1]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}

void Shell::checkJobs() {
    for (auto it = jobs.begin(); it != jobs.end();) {
        if (waitpid(it->first, nullptr, WNOHANG) == 0) {
            cout << "[" << it->first << "] " << it->second << " (running)\n";
            ++it;
        } else {
            it = jobs.erase(it);
        }
    }
}

void Shell::bringToForeground(pid_t pid) {
    cout << "fg requested for PID = " << pid << endl;
    if (jobs.count(pid)) {
        kill(pid, SIGCONT);
        waitpid(pid, nullptr, 0);
        jobs.erase(pid);
    } else {
        cout << "No such job\n";
    }
}

void Shell::continueInBackground(pid_t pid) {
    cout << "bg requested for PID = " << pid << endl;
    if (jobs.count(pid)) {
        kill(pid, SIGCONT);
        cout << "Continued job [" << pid << "] in background\n";
    } else {
        cout << "No such job\n";
    }
}
