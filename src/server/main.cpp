#include "websocket_server.hpp"
#include "../common/logger.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace chat;

/**
 * @brief 从文件加载聊天历史记录
 * 
 * @param filename 历史记录文件名
 * @param history 用于存储加载的历史记录的向量
 */
void loadHistory(const std::string& filename, std::vector<Message>& history) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        try {
            history.push_back(Message::fromString(line));
        } catch (const std::exception& e) {
            Logger::getInstance().log("Error loading history: " + std::string(e.what()));
        }
    }
}

/**
 * @brief 保存消息到历史记录文件
 * 
 * @param filename 历史记录文件名
 * @param message 要保存的消息
 */
void saveHistory(const std::string& filename, const Message& message) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        Logger::getInstance().log("Error saving history: Cannot open file");
        return;
    }
    
    file << message.toString() << std::endl;
}

/**
 * @brief 主函数
 * 
 * 程序入口点，负责：
 * 1. 解析命令行参数
 * 2. 初始化日志系统
 * 3. 加载历史记录
 * 4. 创建和启动聊天服务器
 * 5. 处理服务器生命周期
 */
int main(int argc, char* argv[]) {
    // 解析命令行参数
    uint16_t port = 10808;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }
    
    // 初始化日志系统
    Logger::getInstance().setLogFile("chat_server.log");
    Logger::getInstance().log("Server starting...");
    
    // 加载历史记录
    std::vector<Message> history;
    loadHistory("chat_history.txt", history);
    
    // 创建聊天服务器
    ChatServer server(port);
    
    // 设置消息处理回调
    server.setMessageCallback([&history](const Message& msg) {
        saveHistory("chat_history.txt", msg);
        history.push_back(msg);
    });
    
    // 启动服务器
    server.start();
    
    // 显示服务器信息
    std::cout << "Chat server running on port " << port << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    // 主循环
    try {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        Logger::getInstance().log("Server error: " + std::string(e.what()));
    }
    
    // 停止服务器
    server.stop();
    return 0;
} 