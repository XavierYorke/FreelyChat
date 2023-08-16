/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#include "logger.h"
#include <filesystem>

using std::ios;
using std::string;
using std::cout;
using std::endl;

Logger::Logger() : target_(TERMINAL), level_(DEBUG), path_("") {
    string msg = "Start Logging";
    Info(msg);
}

Logger::Logger(LogTarget target, LogLevel level, string path)
    : target_(target), level_(level), path_(path) {
    
    if (target_ != TERMINAL) {
        if (!std::filesystem::exists(path_)) {
            std::filesystem::create_directory(path_);
        }
        path_ = path_ + "/" + getCurrentTime() + ".log";
        outfile_.open(path_, ios::out | ios::app);
    }
    string info = "Start Logging";
    Info(info);
}

Logger::~Logger() {

}

string Logger::getLevel(LogLevel level) {
    switch(level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERRNO";
        default:
            return "UNKNOW";
    }
}

string Logger::Logging(string text, LogLevel level) {
    string prefix = "[" + getLevel(level) + "]";
    prefix.append(8 - prefix.size(), ' ');
    string curtime = getCurrentTime();
    string msg = curtime + " " + prefix + text;
    if (!msg.empty() && msg.back() != '\n') msg += '\n';

    // The console only prints logs of level level_ and above
    if (level_ <= level && target_ != FILE) {
        cout << msg;
    }
    if (target_ != TERMINAL) {
        outfile_ << msg;
        outfile_.flush();
    }

    return curtime + " " + text;
}

string Logger::Debug(string text) {
    return Logging(text, DEBUG);
}

string Logger::Info(string text) {
    return Logging(text, INFO);
}

string Logger::Warning(string text) {
    return Logging(text, WARNING);
}

string Logger::Error(string text) {
    return Logging(text, ERROR);
}

string Logger::getCurrentTime() {
    // 获取当前系统时间
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();

    // 转换为时间结构体
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // 格式化 年-月-日 时:分:秒
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&currentTime));

    return string(buffer);
}

// int main() {
//     std::filesystem::path currPath =  std::filesystem::current_path() / "log";
//     Logger logger(Logger::BOTH, Logger::INFO, currPath);
//     logger.Info("Hello World!\n");
//     while (1) {

//     }
//     return 0;
// }