#include <gtest/gtest.h>
#include "../src/common/logger.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

using namespace chat;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试日志文件
        testLogFile = "test_log.txt";
        Logger::getInstance().setLogFile(testLogFile);
    }

    void TearDown() override {
        // 清理测试文件
        std::remove(testLogFile.c_str());
    }

    std::string testLogFile;
};

// 测试单例模式
TEST_F(LoggerTest, Singleton) {
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();
    EXPECT_EQ(&logger1, &logger2);
}

// 测试日志文件设置
TEST_F(LoggerTest, SetLogFile) {
    std::string newLogFile = "new_test_log.txt";
    Logger::getInstance().setLogFile(newLogFile);
    
    // 验证文件是否被创建
    std::ifstream file(newLogFile);
    EXPECT_TRUE(file.good());
    file.close();
    
    // 清理
    std::remove(newLogFile.c_str());
}

// 测试日志记录
TEST_F(LoggerTest, LogMessage) {
    std::string testMessage = "Test log message";
    Logger::getInstance().log(testMessage);
    
    // 读取日志文件内容
    std::ifstream logFile(testLogFile);
    std::string line;
    std::getline(logFile, line);
    
    // 验证日志格式
    EXPECT_TRUE(line.find(testMessage) != std::string::npos);
    EXPECT_TRUE(line.find("[") != std::string::npos);
    EXPECT_TRUE(line.find("]") != std::string::npos);
}

// 测试多线程日志记录
TEST_F(LoggerTest, MultiThreadedLogging) {
    const int numThreads = 10;
    const int messagesPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                std::string message = "Thread " + std::to_string(i) + " Message " + std::to_string(j);
                Logger::getInstance().log(message);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证日志文件中的消息数量
    std::ifstream logFile(testLogFile);
    std::string line;
    int messageCount = 0;
    while (std::getline(logFile, line)) {
        messageCount++;
    }
    
    EXPECT_EQ(messageCount, numThreads * messagesPerThread);
}

// 测试日志文件切换
TEST_F(LoggerTest, LogFileSwitch) {
    std::string firstMessage = "First log file message";
    Logger::getInstance().log(firstMessage);
    
    std::string newLogFile = "new_test_log.txt";
    Logger::getInstance().setLogFile(newLogFile);
    
    std::string secondMessage = "Second log file message";
    Logger::getInstance().log(secondMessage);
    
    // 验证第一个日志文件
    std::ifstream firstLogFile(testLogFile);
    std::string line;
    std::getline(firstLogFile, line);
    EXPECT_TRUE(line.find(firstMessage) != std::string::npos);
    
    // 验证第二个日志文件
    std::ifstream secondLogFile(newLogFile);
    std::getline(secondLogFile, line);
    EXPECT_TRUE(line.find(secondMessage) != std::string::npos);
    
    // 清理
    std::remove(newLogFile.c_str());
} 