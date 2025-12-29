#include <gtest/gtest.h>
#include "../src/client/websocket_client.hpp"
#include "../src/server/websocket_server.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <future>
#include <condition_variable>
#include <random>

using namespace chat;

class WebSocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 生成随机端口号
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1024, 65535);
        testPort = static_cast<uint16_t>(dis(gen));
        
        // 创建测试服务器
        server = std::make_unique<ChatServer>(testPort);
        
        // 启动服务器
        serverThread = std::thread([this]() {
            server->start();
        });
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    void TearDown() override {
        if (server) {
            server->stop();
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    // 等待连接建立
    bool waitForConnection(std::chrono::seconds timeout = std::chrono::seconds(5)) {
        auto start = std::chrono::steady_clock::now();
        while (!connected.load()) {
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
            [this] { return !receivedMessage.empty(); });
    }

    uint16_t testPort;
    std::unique_ptr<ChatServer> server;
    std::thread serverThread;
    std::atomic<bool> connected{false};
    std::string receivedMessage;
    std::mutex messageMutex;
    std::condition_variable messageCondition;
};

// 测试连接
TEST_F(WebSocketClientTest, Connection) {
    auto client = std::make_unique<ChatClient>("test_user");

    // 连接服务器
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(client->isConnected());

    // 断开连接
    client->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(client->isConnected());
}

// 测试发送消息
TEST_F(WebSocketClientTest, SendMessage) {
    auto client = std::make_unique<ChatClient>("test_user");
    
    // 设置消息回调
    client->setMessageCallback([this](const Message& msg) {
        std::lock_guard<std::mutex> lock(messageMutex);
        receivedMessage = msg.content;
        messageCondition.notify_one();
    });

    // 连接服务器
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(client->isConnected());
    
    // 发送测试消息
    std::string testMessage = "Hello, WebSocket!";
    client->send(testMessage);
    
    // 等待消息接收
    EXPECT_TRUE(waitForMessage());
    EXPECT_EQ(receivedMessage, testMessage);
    
    // 断开连接
    client->disconnect();
}

// 测试断开连接
TEST_F(WebSocketClientTest, Disconnection) {
    auto client = std::make_unique<ChatClient>("test_user");

    // 连接服务器
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(client->isConnected());

    // 断开连接
    client->disconnect();

    // 等待连接状态更新
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(client->isConnected());
}

// 测试无效URI
TEST_F(WebSocketClientTest, InvalidUri) {
    auto client = std::make_unique<ChatClient>("test_user");
    
    // 尝试连接到无效地址
    client->connect("ws://invalid_host:12345");

    // 等待连接尝试
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_FALSE(client->isConnected());
}

// 测试重连
TEST_F(WebSocketClientTest, Reconnection) {
    auto client = std::make_unique<ChatClient>("test_user");
    
    // 连接服务器
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(client->isConnected());

    // 断开连接
    client->disconnect();

    // 等待连接状态更新
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(client->isConnected());

    // 重新连接
    client->connect(uri);

    // 等待连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(client->isConnected());
    
    // 断开连接
    client->disconnect();
}

// 测试服务器未运行
TEST_F(WebSocketClientTest, ServerNotRunning) {
    // 停止服务器
    server->stop();
    serverThread.join();
    
    auto client = std::make_unique<ChatClient>("test_user");

    // 尝试连接
    std::string uri = "ws://localhost:" + std::to_string(testPort);
    client->connect(uri);

    // 等待连接尝试
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_FALSE(client->isConnected());
} 