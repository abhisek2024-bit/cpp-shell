#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>

class Shell {
public:
    void run();
private:
    std::vector<std::string> parseInput(const std::string& input);
    void executeCommand(std::vector<std::string> args, bool background);
    void handleRedirection(std::vector<std::string>& args);
    void handlePiping(std::vector<std::string>& args);
    void launchPipe(std::vector<std::string> cmd1, std::vector<std::string> cmd2);
    void checkJobs();
    void bringToForeground(pid_t pid);
    void continueInBackground(pid_t pid);

    std::map<pid_t, std::string> jobs;
};

#endif
