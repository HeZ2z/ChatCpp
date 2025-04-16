#include "websocket_client.hpp"
#include "../common/logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

using namespace chat;

/**
 * @brief 主函数
 * 
 * 程序入口点，负责：
 * 1. 解析命令行参数
 * 2. 初始化日志系统
 * 3. 创建和连接聊天客户端
 * 4. 处理用户输入
 * 5. 显示接收到的消息
 */
int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <username> [server_ip] [port]" << std::endl;
        return 1;
    }
    
    // 解析命令行参数
    std::string username = argv[1];
    std::string serverIp = (argc > 2) ? argv[2] : "127.0.0.1";
    uint16_t port = (argc > 3) ? static_cast<uint16_t>(std::stoi(argv[3])) : 8080;
    
    // 初始化日志系统
    Logger::getInstance().setLogFile("chat_client.log");
    Logger::getInstance().log("Client starting...");
    
    // 创建聊天客户端
    ChatClient client(username);
    
    // 设置消息处理回调
    client.setMessageCallback([](const Message& msg) {
        std::cout << msg.toString() << std::endl;
    });
    
    // 构建服务器URI并连接
    std::string uri = "ws://" + serverIp + ":" + std::to_string(port);
    client.connect(uri);
    
    // 显示连接信息和使用说明
    std::cout << "Connected to " << uri << std::endl;
    std::cout << "Type your message and press Enter to send" << std::endl;
    std::cout << "Type \\quit or \\exit to quit" << std::endl;
    
    // 处理用户输入
    std::string input;
    while (std::getline(std::cin, input)) {
        // 检查退出命令
        if (input == "\\quit" || input == "\\exit") {
            break;
        }
        
        // 发送非空消息
        if (!input.empty()) {
            client.send(input);
        }
    }
    
    // 断开连接
    client.disconnect();
    return 0;
} 