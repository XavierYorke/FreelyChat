/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>

using std::string;

class Logger {
public:
    enum LogTarget {FILE, TERMINAL, BOTH};
    enum LogLevel {DEBUG, INFO, WARNING, ERROR};

    Logger();
    Logger(LogTarget target, LogLevel level, string path);
    ~Logger();


    static string getCurrentTime();
    string Debug(string text);
    string Info(string text);
    string Warning(string text);
    string Error(string text);

private:
    string Logging(string text, LogLevel level);
    string getLevel(LogLevel level);
    LogTarget target_;
    LogLevel level_;
    std::ofstream outfile_;
    string path_;
};

