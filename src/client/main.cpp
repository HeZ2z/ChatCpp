#include "websocket_client.hpp"
#include "../common/logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

using namespace chat;

// 获取单个字符输入
char getch() {
    struct termios old_settings, new_settings;
    char ch;
    
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    return ch;
}

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
    uint16_t port = (argc > 3) ? static_cast<uint16_t>(std::stoi(argv[3])) : 10808;
    
    // 初始化日志系统
    Logger::getInstance().setLogFile("chat_client.log");
    Logger::getInstance().log("Client starting...");
    
    // 创建聊天客户端
    ChatClient client(username);
    
    // 设置消息处理回调
    client.setMessageCallback([username](const Message& msg) {
        // 只有当消息不是自己发送的时才显示
        if (msg.username != username) {
            std::cout << msg.toString() << std::endl;
            std::cout << "💬: ";  // 重新显示输入提示
            std::cout.flush();
        }
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
    while (true) {
        std::cout << "💬: ";  // 显示输入提示
        std::cout.flush();
        
        input.clear();
        char ch;
        while ((ch = getch()) != '\n') {
            if (ch == '\b') {  // 退格键
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";  // 删除一个字符
                }
            } else {
                input += ch;
                std::cout << ch;
            }
            std::cout.flush();
        }
        std::cout << std::endl;
        
        // 检查退出命令
        if (input == "\\quit" || input == "\\exit") {
            break;
        }
        
        // 发送非空消息
        if (!input.empty()) {
            client.send(input);
            // 不在这里显示输入提示，因为消息处理回调会处理
        } else {
            // 如果消息为空，重新显示输入提示
            std::cout << "💬: ";
            std::cout.flush();
        }
    }
    
    // 断开连接
    client.disconnect();
    return 0;
} 