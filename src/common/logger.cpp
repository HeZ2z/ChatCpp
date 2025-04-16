#include "logger.hpp"
#include <iostream>
#include <ctime>

namespace chat {

/**
 * @brief 获取Logger单例实例
 * 
 * 使用局部静态变量确保线程安全的单例模式
 */
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

/**
 * @brief 析构函数
 * 
 * 确保在对象销毁时关闭日志文件
 */
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief 设置日志文件
 * 
 * 如果已经打开了一个日志文件，会先关闭它
 * 然后打开新的日志文件
 * 
 * @param filename 日志文件路径
 */
void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
    currentLogFile = filename;
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

/**
 * @brief 记录一条日志消息
 * 
 * 消息格式：[YYYY-MM-DD HH:MM:SS] message
 * 
 * @param message 要记录的日志消息
 */
void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (!logFile.is_open()) {
        std::cerr << "Log file not open" << std::endl;
        return;
    }
    
    // 获取当前时间并格式化为字符串
    char timeStr[20];
    time_t now = time(nullptr);
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    
    // 写入日志消息
    logFile << "[" << timeStr << "] " << message << std::endl;
    // 立即刷新缓冲区，确保日志被写入文件
    logFile.flush();
}

} // namespace chat 