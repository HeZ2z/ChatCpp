#include <gtest/gtest.h>
#include "../src/common/message.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <thread>
#include <mutex>
#include <future>
#include <random>
#include <sys/stat.h>

using namespace chat;

class HistoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试历史文件
        testHistoryFile = "test_history.txt";
        // 确保测试文件不存在
        if (std::filesystem::exists(testHistoryFile)) {
            std::filesystem::remove(testHistoryFile);
        }
    }

    void TearDown() override {
        // 清理测试文件
        if (std::filesystem::exists(testHistoryFile)) {
            std::filesystem::remove(testHistoryFile);
        }
    }

    // RAII文件包装器
    class FileWrapper {
    public:
        FileWrapper(const std::string& filename, std::ios_base::openmode mode)
            : file(filename, mode) {}
        ~FileWrapper() { file.close(); }
        std::fstream& get() { return file; }
    private:
        std::fstream file;
    };

    std::string testHistoryFile;
};

// 测试保存历史记录
TEST_F(HistoryTest, SaveHistory) {
    // 创建测试消息
    Message msg1("alice", "Hello, World!");
    Message msg2("bob", "Hi there!");
    
    // 使用RAII方式保存消息到历史文件
    {
        FileWrapper file(testHistoryFile, std::ios::out);
        file.get() << msg1.toString() << std::endl;
        file.get() << msg2.toString() << std::endl;
    }
    
    // 验证文件内容
    FileWrapper readFile(testHistoryFile, std::ios::in);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(readFile.get(), line)) {
        lines.push_back(line);
    }
    
    EXPECT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[0].find("alice @ Hello, World!") != std::string::npos);
    EXPECT_TRUE(lines[1].find("bob @ Hi there!") != std::string::npos);
}

// 测试加载历史记录
TEST_F(HistoryTest, LoadHistory) {
    // 创建测试历史文件
    {
        FileWrapper file(testHistoryFile, std::ios::out);
        file.get() << "alice @ Hello | 2024-03-20 10:00:00" << std::endl;
        file.get() << "bob @ Hi | 2024-03-20 10:01:00" << std::endl;
    }
    
    // 加载历史记录
    std::vector<Message> history;
    FileWrapper readFile(testHistoryFile, std::ios::in);
    std::string line;
    
    while (std::getline(readFile.get(), line)) {
        try {
            history.push_back(Message::fromString(line));
        } catch (const std::exception& e) {
            FAIL() << "Failed to parse history line: " << e.what();
        }
    }
    
    // 验证加载的消息
    EXPECT_EQ(history.size(), 2);
    EXPECT_EQ(history[0].username, "alice");
    EXPECT_EQ(history[0].content, "Hello");
    EXPECT_EQ(history[1].username, "bob");
    EXPECT_EQ(history[1].content, "Hi");
}

// 测试无效的历史记录格式
TEST_F(HistoryTest, InvalidHistoryFormat) {
    // 创建包含无效格式的测试文件
    {
        FileWrapper file(testHistoryFile, std::ios::out);
        file.get() << "invalid format" << std::endl;
        file.get() << "missing @ symbol" << std::endl;
    }
    
    // 尝试加载历史记录
    FileWrapper readFile(testHistoryFile, std::ios::in);
    std::string line;
    
    while (std::getline(readFile.get(), line)) {
        EXPECT_THROW(Message::fromString(line), std::runtime_error);
    }
}

// 测试历史记录时间戳
TEST_F(HistoryTest, HistoryTimestamp) {
    // 创建测试消息
    Message msg("test_user", "Test message");
    std::string msgStr = msg.toString();
    
    // 从字符串解析消息
    Message parsedMsg = Message::fromString(msgStr);
    
    // 验证时间戳
    EXPECT_EQ(parsedMsg.timestamp, msg.timestamp);
}

// 测试历史记录追加
TEST_F(HistoryTest, AppendHistory) {
    // 创建初始消息
    Message msg1("user1", "First message");
    
    // 保存初始消息
    {
        FileWrapper file(testHistoryFile, std::ios::out);
        file.get() << msg1.toString() << std::endl;
    }
    
    // 追加新消息
    Message msg2("user2", "Second message");
    {
        FileWrapper appendFile(testHistoryFile, std::ios::app);
        appendFile.get() << msg2.toString() << std::endl;
    }
    
    // 验证文件内容
    FileWrapper readFile(testHistoryFile, std::ios::in);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(readFile.get(), line)) {
        lines.push_back(line);
    }
    
    EXPECT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[0].find("user1 @ First message") != std::string::npos);
    EXPECT_TRUE(lines[1].find("user2 @ Second message") != std::string::npos);
}

// 测试并发访问
TEST_F(HistoryTest, ConcurrentAccess) {
    const int numThreads = 10;
    const int messagesPerThread = 100;
    std::vector<std::future<void>> futures;
    std::mutex fileMutex;
    
    // 创建多个线程同时写入文件
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, messagesPerThread, &fileMutex]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                Message msg("user" + std::to_string(i), 
                          "Message " + std::to_string(j));
                std::lock_guard<std::mutex> lock(fileMutex);
                FileWrapper file(testHistoryFile, std::ios::app);
                file.get() << msg.toString() << std::endl;
            }
        }));
    }
    
    // 等待所有线程完成
    for (auto& future : futures) {
        future.wait();
    }
    
    // 验证文件内容
    FileWrapper readFile(testHistoryFile, std::ios::in);
    std::string line;
    int messageCount = 0;
    
    while (std::getline(readFile.get(), line)) {
        messageCount++;
        try {
            Message msg = Message::fromString(line);
            EXPECT_TRUE(msg.username.find("user") != std::string::npos);
            EXPECT_TRUE(msg.content.find("Message") != std::string::npos);
        } catch (const std::exception& e) {
            FAIL() << "Failed to parse message: " << e.what();
        }
    }
    
    EXPECT_EQ(messageCount, numThreads * messagesPerThread);
}

// 测试文件权限
TEST_F(HistoryTest, FilePermissions) {
    // 创建历史记录
    std::vector<Message> messages = {
        Message("user1", "Hello"),
        Message("user2", "Hi there")
    };
    
    // 保存历史记录
    std::string filename = "test_history.json";
    EXPECT_TRUE(history->saveHistory(messages, filename));
    
    // 验证文件权限
    struct stat st;
    EXPECT_EQ(stat(filename.c_str(), &st), 0);
    EXPECT_EQ(st.st_mode & S_IRUSR, S_IRUSR);  // 用户可读
    EXPECT_EQ(st.st_mode & S_IWUSR, S_IWUSR);  // 用户可写
    
    // 清理测试文件
    std::remove(filename.c_str());
} 