/**
 * Custom Linux Shell Implementation
 * Supports piping, redirection, background jobs, and command chaining
 * Author: Komari Venkata Ramana
 */

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>

using namespace std;

struct Command {
    vector<string> args;
    string input_file;
    string output_file;
    bool append;
    bool background;
};

Command parse_single_command(const string& input) {
    Command cmd;
    cmd.append = false;
    cmd.background = false;
    vector<string> tokens;
    string token;
    bool in_quotes = false;
    for (char c : input) {
        if (c == '"' || c == '\'') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }

    // Now parse tokens for redirections
    vector<string> args;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "<" && i + 1 < tokens.size()) {
            cmd.input_file = tokens[i + 1];
            ++i;
        } else if (tokens[i] == ">" && i + 1 < tokens.size()) {
            cmd.output_file = tokens[i + 1];
            cmd.append = false;
            ++i;
        } else if (tokens[i] == ">>" && i + 1 < tokens.size()) {
            cmd.output_file = tokens[i + 1];
            cmd.append = true;
            ++i;
        } else if (tokens[i] == "&") {
            cmd.background = true;
        } else {
            args.push_back(tokens[i]);
        }
    }
    cmd.args = args;
    return cmd;
}

vector<Command> parse_commands(const string& input) {
    vector<string> parts;
    string part;
    bool in_quotes = false;
    for (char c : input) {
        if (c == '"' || c == '\'') {
            in_quotes = !in_quotes;
        } else if (c == '|' && !in_quotes) {
            if (!part.empty()) {
                parts.push_back(part);
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        parts.push_back(part);
    }

    vector<Command> cmds;
    for (auto& p : parts) {
        cmds.push_back(parse_single_command(p));
    }
    return cmds;
}

vector<string> split_by_semicolon(const string& input) {
    vector<string> parts;
    string part;
    bool in_quotes = false;
    for (char c : input) {
        if (c == '"' || c == '\'') {
            in_quotes = !in_quotes;
        } else if (c == ';' && !in_quotes) {
            if (!part.empty()) {
                parts.push_back(part);
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        parts.push_back(part);
    }
    return parts;
}

int main() {
    string input;
    while (true) {
        cout << "myshell> ";
        getline(cin, input);
        if (input.empty()) continue;

        vector<string> chained = split_by_semicolon(input);
        for (auto& part : chained) {
            vector<Command> cmds = parse_commands(part);
            if (cmds.empty() || cmds[0].args.empty()) continue;

            if (cmds.size() == 1) {
                auto& cmd = cmds[0];
                if (cmd.args[0] == "exit") {
                    return 0;  // exit the shell
                } else if (cmd.args[0] == "cd") {
                    if (cmd.args.size() > 1) {
                        if (chdir(cmd.args[1].c_str()) != 0) {
                            perror("cd");
                        }
                    } else {
                        const char* home = getenv("HOME");
                        if (home) {
                            chdir(home);
                        }
                    }
                    continue;
                } else if (cmd.args[0] == "pwd") {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                        cout << cwd << endl;
                    } else {
                        perror("pwd");
                    }
                    continue;
                }

                // Execute single command
                pid_t pid = fork();
                if (pid == 0) {
                    // Handle redirections
                    if (!cmd.input_file.empty()) {
                        int fd = open(cmd.input_file.c_str(), O_RDONLY);
                        if (fd == -1) {
                            perror("input redirection");
                            exit(1);
                        }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                    }
                    if (!cmd.output_file.empty()) {
                        int flags = O_WRONLY | O_CREAT | (cmd.append ? O_APPEND : O_TRUNC);
                        int fd = open(cmd.output_file.c_str(), flags, 0644);
                        if (fd == -1) {
                            perror("output redirection");
                            exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }

                    vector<char*> c_args;
                    for (auto& arg : cmd.args) {
                        c_args.push_back(const_cast<char*>(arg.c_str()));
                    }
                    c_args.push_back(nullptr);
                    execvp(c_args[0], c_args.data());
                    cerr << "Command not found: " << cmd.args[0] << endl;
                    exit(1);
                } else if (pid > 0) {
                    if (!cmd.background) {
                        waitpid(pid, nullptr, 0);
                    }
                } else {
                    cerr << "Fork failed" << endl;
                }
            } else {
                // Handle piping
                int num_cmds = cmds.size();
                vector<int> pipes((num_cmds - 1) * 2);
                for (int i = 0; i < num_cmds - 1; ++i) {
                    if (pipe(pipes.data() + i * 2) == -1) {
                        perror("pipe");
                        continue;
                    }
                }

                vector<pid_t> pids;
                for (int i = 0; i < num_cmds; ++i) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        if (i > 0) {
                            dup2(pipes[(i - 1) * 2], STDIN_FILENO);
                        }
                        if (i < num_cmds - 1) {
                            dup2(pipes[i * 2 + 1], STDOUT_FILENO);
                        }
                        // Handle redirections for first and last
                        if (i == 0 && !cmds[i].input_file.empty()) {
                            int fd = open(cmds[i].input_file.c_str(), O_RDONLY);
                            if (fd != -1) {
                                dup2(fd, STDIN_FILENO);
                                close(fd);
                            }
                        }
                        if (i == num_cmds - 1 && !cmds[i].output_file.empty()) {
                            int flags = O_WRONLY | O_CREAT | (cmds[i].append ? O_APPEND : O_TRUNC);
                            int fd = open(cmds[i].output_file.c_str(), flags, 0644);
                            if (fd != -1) {
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                            }
                        }
                        // Close all pipes
                        for (int p : pipes) close(p);
                        vector<char*> c_args;
                        for (auto& arg : cmds[i].args) {
                            c_args.push_back(const_cast<char*>(arg.c_str()));
                        }
                        c_args.push_back(nullptr);
                        execvp(c_args[0], c_args.data());
                        exit(1);
                    } else if (pid > 0) {
                        pids.push_back(pid);
                    } else {
                        perror("fork");
                    }
                }
                // Parent close pipes
                for (int p : pipes) close(p);
                if (!cmds.back().background) {
                    for (pid_t p : pids) {
                        waitpid(p, nullptr, 0);
                    }
                }
            }
        }
    }
    return 0;
}
