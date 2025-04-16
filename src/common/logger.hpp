#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace chat {

/**
 * @brief 日志记录器类，使用单例模式实现
 * 
 * 提供线程安全的日志记录功能，支持：
 * - 设置日志文件
 * - 记录带时间戳的日志消息
 * - 自动刷新日志缓冲区
 */
class Logger {
public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger实例的引用
     */
    static Logger& getInstance();
    
    /**
     * @brief 记录一条日志消息
     * @param message 要记录的日志消息
     */
    void log(const std::string& message);

    /**
     * @brief 设置日志文件
     * @param filename 日志文件路径
     */
    void setLogFile(const std::string& filename);

private:
    /**
     * @brief 私有构造函数，防止外部创建实例
     */
    Logger() = default;

    /**
     * @brief 析构函数，确保关闭日志文件
     */
    ~Logger();
    
    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::ofstream logFile;        ///< 日志文件流
    std::mutex logMutex;          ///< 用于线程同步的互斥锁
    std::string currentLogFile;   ///< 当前日志文件路径
};

} // namespace chat 