#include "websocket_server.hpp"
#include "../common/logger.hpp"
#include <iostream>
#include <thread>

namespace chat {

/**
 * @brief 构造函数
 * 
 * 初始化WebSocket服务器，设置事件处理器
 * 
 * @param port 服务器监听端口
 */
ChatServer::ChatServer(uint16_t port) : port(port), running(false) {
    // 设置日志级别
    server.set_access_channels(websocketpp::log::alevel::none);
    server.set_error_channels(websocketpp::log::elevel::fatal);
    
    // 初始化ASIO
    server.init_asio();
    
    // 设置事件处理器
    server.set_open_handler(std::bind(&ChatServer::onOpen, this, std::placeholders::_1));
    server.set_close_handler(std::bind(&ChatServer::onClose, this, std::placeholders::_1));
    server.set_message_handler(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief 析构函数
 * 
 * 确保服务器正确关闭
 */
ChatServer::~ChatServer() {
    stop();
}

/**
 * @brief 启动服务器
 * 
 * 在新线程中运行服务器事件循环
 */
void ChatServer::start() {
    if (running) return;
    
    running = true;
    server.listen(port);
    server.start_accept();
    
    // 在新线程中运行服务器
    std::thread([this]() { run(); }).detach();
    
    Logger::getInstance().log("Server started on port " + std::to_string(port));
}

/**
 * @brief 停止服务器
 * 
 * 关闭所有连接并停止服务器
 */
void ChatServer::stop() {
    if (!running) return;
    
    running = false;
    server.stop();
    server.stop_listening();
    
    // 关闭所有连接
    for (auto& hdl : connections) {
        server.close(hdl, websocketpp::close::status::normal, "Server shutting down");
    }
    connections.clear();
    
    Logger::getInstance().log("Server stopped");
}

/**
 * @brief 广播消息给所有连接的客户端
 * 
 * @param message 要广播的消息
 */
void ChatServer::broadcast(const std::string& message) {
    for (auto& hdl : connections) {
        try {
            server.send(hdl, message, websocketpp::frame::opcode::text);
        } catch (const std::exception& e) {
            Logger::getInstance().log("Error broadcasting message: " + std::string(e.what()));
        }
    }
}

/**
 * @brief 设置消息处理回调函数
 * 
 * @param callback 消息处理函数
 */
void ChatServer::setMessageCallback(std::function<void(const Message&)> callback) {
    messageCallback = callback;
}

/**
 * @brief 处理新的客户端连接
 * 
 * @param hdl 连接句柄
 */
void ChatServer::onOpen(ConnectionHdl hdl) {
    connections.insert(hdl);
    Logger::getInstance().log("New connection established");
}

/**
 * @brief 处理客户端断开连接
 * 
 * @param hdl 连接句柄
 */
void ChatServer::onClose(ConnectionHdl hdl) {
    connections.erase(hdl);
    Logger::getInstance().log("Connection closed");
}

/**
 * @brief 处理接收到的消息
 * 
 * 解析消息并调用回调函数
 * 
 * @param hdl 连接句柄
 * @param msg 消息内容
 */
void ChatServer::onMessage(ConnectionHdl hdl, WebSocketServer::message_ptr msg) {
    try {
        Message message = Message::fromString(msg->get_payload());
        if (messageCallback) {
            messageCallback(message);
        }
        broadcast(message.toString());
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error processing message: " + std::string(e.what()));
    }
}

/**
 * @brief 运行服务器事件循环
 * 
 * 处理WebSocket事件
 */
void ChatServer::run() {
    try {
        server.run();
    } catch (const std::exception& e) {
        Logger::getInstance().log("Server error: " + std::string(e.what()));
    }
}

} // namespace chat 