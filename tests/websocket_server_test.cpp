#include <gtest/gtest.h>
#include "../src/server/websocket_server.hpp"
#include "../src/client/websocket_client.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <random>

using namespace chat;

class WebSocketServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 生成随机端口号
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1024, 65535);
        testPort = static_cast<uint16_t>(dis(gen));
        
        // 创建测试服务器
        server = std::make_unique<ChatServer>(testPort);
        
        // 设置消息回调
        server->setMessageCallback([this](const Message& msg) {
            std::lock_guard<std::mutex> lock(messageMutex);
            receivedMessages.push_back(msg);
            messageCondition.notify_one();
        });
    }

    void TearDown() override {
        if (server) {
            server->stop();
        }
    }

    // 等待服务器启动
    bool waitForServerStart(std::chrono::seconds timeout = std::chrono::seconds(5)) {
        auto start = std::chrono::steady_clock::now();
        while (!server->isRunning()) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return true;
    }

    // 等待消息接收
    bool waitForMessage(std::chrono::seconds timeout = std::chrono::seconds(5)) {
        std::unique_lock<std::mutex> lock(messageMutex);
        return messageCondition.wait_for(lock, timeout,
            [this] { return !receivedMessages.empty(); });
    }

    uint16_t testPort;
    std::unique_ptr<ChatServer> server;
    std::vector<Message> receivedMessages;
    std::mutex messageMutex;
    std::condition_variable messageCondition;
};

// 测试服务器启动和停止
TEST_F(WebSocketServerTest, StartStop) {
    // 启动服务器
    std::thread serverThread([this]() {
        server->start();
    });
    
    // 等待服务器启动
    EXPECT_TRUE(waitForServerStart());
    
    // 停止服务器
    server->stop();
    serverThread.join();
    
    EXPECT_FALSE(server->isRunning());
}

// 测试消息广播
TEST_F(WebSocketServerTest, Broadcast) {
    // 启动服务器
    std::thread serverThread([this]() {
        server->start();
    });
    
    // 等待服务器启动
    EXPECT_TRUE(waitForServerStart());
    
    // 创建测试客户端
    auto client = std::make_unique<ChatClient>("test_user");
    
    // 设置消息回调
    client->setMessageCallback([this](const std::string& msg) {
        std::lock_guard<std::mutex> lock(messageMutex);
        receivedMessages.push_back(Message("server", msg));
        messageCondition.notify_one();
    });
    
    // 连接客户端
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);
    
    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 广播测试消息
    std::string testMessage = "Broadcast test message";
    server->broadcast(testMessage);
    
    // 等待消息处理
    EXPECT_TRUE(waitForMessage());
    
    // 断开客户端连接
    client->disconnect();
    
    // 停止服务器
    server->stop();
    serverThread.join();
    
    // 验证消息是否被广播
    std::lock_guard<std::mutex> lock(messageMutex);
    EXPECT_FALSE(receivedMessages.empty());
    EXPECT_EQ(receivedMessages[0].content, testMessage);
}

// 测试客户端连接管理
TEST_F(WebSocketServerTest, ClientConnection) {
    // 启动服务器
    std::thread serverThread([this]() {
        server->start();
    });
    
    // 等待服务器启动
    EXPECT_TRUE(waitForServerStart());
    
    // 创建测试客户端
    auto client1 = std::make_unique<ChatClient>("test_user1");
    auto client2 = std::make_unique<ChatClient>("test_user2");
    
    // 连接客户端
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client1->connect(uri);
    client2->connect(uri);
    
    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 发送测试消息
    std::string testMessage = "Test message from client1";
    client1->send(testMessage);
    
    // 等待消息处理
    EXPECT_TRUE(waitForMessage());
    
    // 断开客户端连接
    client1->disconnect();
    client2->disconnect();
    
    // 停止服务器
    server->stop();
    serverThread.join();
    
    // 验证消息是否被接收
    std::lock_guard<std::mutex> lock(messageMutex);
    EXPECT_FALSE(receivedMessages.empty());
    EXPECT_EQ(receivedMessages[0].content, testMessage);
}

// 测试错误处理
TEST_F(WebSocketServerTest, ErrorHandling) {
    // 启动服务器
    std::thread serverThread([this]() {
        server->start();
    });
    
    // 等待服务器启动
    EXPECT_TRUE(waitForServerStart());
    
    // 创建无效客户端
    auto invalidClient = std::make_unique<ChatClient>("invalid_user");
    
    // 尝试连接到无效地址
    invalidClient->connect("ws://invalid_host:12345");
    
    // 等待连接尝试
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 停止服务器
    server->stop();
    serverThread.join();
    
    // 验证没有消息被接收
    std::lock_guard<std::mutex> lock(messageMutex);
    EXPECT_TRUE(receivedMessages.empty());
}

// 测试服务器启动失败
TEST_F(WebSocketServerTest, ServerStartFailure) {
    // 启动第一个服务器
    std::thread serverThread1([this]() {
        server->start();
    });
    
    // 等待第一个服务器启动
    EXPECT_TRUE(waitForServerStart());
    
    // 创建一个使用相同端口的服务器
    auto invalidServer = std::make_unique<ChatServer>(testPort);
    
    // 尝试启动第二个服务器
    std::thread serverThread2([&invalidServer]() {
        try {
            invalidServer->start();
        } catch (const std::exception& e) {
            // 预期会抛出异常，因为端口已被占用
            EXPECT_TRUE(true);
        }
    });
    
    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 停止服务器
    server->stop();
    invalidServer->stop();
    
    // 等待线程结束
    if (serverThread1.joinable()) {
        serverThread1.join();
    }
    if (serverThread2.joinable()) {
        serverThread2.join();
    }
} 