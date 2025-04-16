#include "websocket_client.hpp"
#include "../common/logger.hpp"
#include <iostream>
#include <thread>

namespace chat {

/**
 * @brief 构造函数
 * 
 * 初始化WebSocket客户端，设置事件处理器
 * 
 * @param username 客户端用户名
 */
ChatClient::ChatClient(const std::string& username)
    : username(username), connected(false) {
    // 设置日志级别
    client.set_access_channels(websocketpp::log::alevel::none);
    client.set_error_channels(websocketpp::log::elevel::fatal);
    
    // 初始化ASIO
    client.init_asio();
    
    // 设置事件处理器
    client.set_open_handler(std::bind(&ChatClient::onOpen, this, std::placeholders::_1));
    client.set_close_handler(std::bind(&ChatClient::onClose, this, std::placeholders::_1));
    client.set_message_handler(std::bind(&ChatClient::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief 析构函数
 * 
 * 确保客户端正确断开连接
 */
ChatClient::~ChatClient() {
    disconnect();
}

/**
 * @brief 连接到WebSocket服务器
 * 
 * 在新线程中运行客户端事件循环
 * 
 * @param uri 服务器URI（格式：ws://host:port）
 */
void ChatClient::connect(const std::string& uri) {
    if (connected) return;
    
    // 创建连接
    websocketpp::lib::error_code ec;
    WebSocketClient::connection_ptr con = client.get_connection(uri, ec);
    if (ec) {
        Logger::getInstance().log("Error connecting: " + ec.message());
        return;
    }
    
    // 建立连接
    client.connect(con);
    
    // 在新线程中运行客户端
    std::thread([this]() { run(); }).detach();
}

/**
 * @brief 断开与服务器的连接
 */
void ChatClient::disconnect() {
    if (!connected) return;
    
    client.close(connection, websocketpp::close::status::normal, "Client disconnecting");
    connected = false;
}

/**
 * @brief 发送消息到服务器
 * 
 * @param message 要发送的消息内容
 */
void ChatClient::send(const std::string& message) {
    if (!connected) {
        Logger::getInstance().log("Not connected to server");
        return;
    }
    
    try {
        // 创建消息对象并发送
        Message msg(username, message);
        client.send(connection, msg.toString(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error sending message: " + std::string(e.what()));
    }
}

/**
 * @brief 设置消息处理回调函数
 * 
 * @param callback 消息处理函数
 */
void ChatClient::setMessageCallback(std::function<void(const Message&)> callback) {
    messageCallback = callback;
}

/**
 * @brief 处理连接建立事件
 * 
 * @param hdl 连接句柄
 */
void ChatClient::onOpen(ConnectionHdl hdl) {
    connection = hdl;
    connected = true;
    Logger::getInstance().log("Connected to server");
}

/**
 * @brief 处理连接关闭事件
 * 
 * @param hdl 连接句柄
 */
void ChatClient::onClose(ConnectionHdl hdl) {
    connected = false;
    Logger::getInstance().log("Disconnected from server");
}

/**
 * @brief 处理接收到的消息
 * 
 * 解析消息并调用回调函数
 * 
 * @param hdl 连接句柄
 * @param msg 消息内容
 */
void ChatClient::onMessage(ConnectionHdl hdl, WebSocketClient::message_ptr msg) {
    try {
        Message message = Message::fromString(msg->get_payload());
        if (messageCallback) {
            messageCallback(message);
        }
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error processing message: " + std::string(e.what()));
    }
}

/**
 * @brief 运行客户端事件循环
 * 
 * 处理WebSocket事件
 */
void ChatClient::run() {
    try {
        client.run();
    } catch (const std::exception& e) {
        Logger::getInstance().log("Client error: " + std::string(e.what()));
    }
}

} // namespace chat 