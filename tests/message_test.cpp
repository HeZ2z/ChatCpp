#include <gtest/gtest.h>
#include "../src/common/message.hpp"
#include <chrono>
#include <thread>

// 故意制造全局内存泄漏用于测试
#ifdef ENABLE_MEMORY_LEAK_TEST
static int* global_leak = new int[50];
#endif

using namespace chat;

class MessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }

    void TearDown() override {
        // 测试后的清理
    }
};

// 测试消息的创建和基本属性
TEST_F(MessageTest, CreateMessage) {
    Message msg("alice", "Hello, World!");
    EXPECT_EQ(msg.username, "alice");
    EXPECT_EQ(msg.content, "Hello, World!");
    EXPECT_NE(msg.timestamp, 0);

    // 故意制造内存泄漏用于测试 Valgrind 流程
    #ifdef ENABLE_MEMORY_LEAK_TEST
    int* leak = new int[100];
    // 不释放内存，制造泄漏
    // 故意泄漏指针，使 AddressSanitizer 和 Valgrind 都能检测到
    leak = nullptr;  // 丢失原始指针
    #endif
}

// 测试消息的toString方法
TEST_F(MessageTest, ToString) {
    Message msg("bob", "Test message");
    std::string str = msg.toString();
    
    // 验证字符串格式
    EXPECT_TRUE(str.find("bob @ Test message | ") != std::string::npos);
    // 验证时间戳格式
    EXPECT_TRUE(str.find("202") != std::string::npos); // 年份以202开头
    EXPECT_TRUE(str.find(":") != std::string::npos);   // 包含时间分隔符
}

// 测试消息的fromString方法
TEST_F(MessageTest, FromString) {
    std::string input = "charlie @ Hello there | 2024-03-20 15:30:00";
    Message msg = Message::fromString(input);
    
    EXPECT_EQ(msg.username, "charlie");
    EXPECT_EQ(msg.content, "Hello there");
    
    // 验证时间戳
    std::tm tm = {};
    std::stringstream ss("2024-03-20 15:30:00");
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    time_t expected_time = std::mktime(&tm);
    EXPECT_EQ(msg.timestamp, expected_time);
}

// 测试无效的消息字符串
TEST_F(MessageTest, InvalidString) {
    std::string invalid_input = "invalid format";
    EXPECT_THROW(Message::fromString(invalid_input), std::runtime_error);
}

// 测试消息的时间戳更新
TEST_F(MessageTest, TimestampUpdate) {
    Message msg("dave", "Test");
    time_t initial_time = msg.timestamp;
    
    // 等待一小段时间
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 更新消息内容
    msg.setContent("Updated test");
    EXPECT_GT(msg.timestamp, initial_time);
} 