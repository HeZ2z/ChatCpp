#pragma once

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <functional>
#include <string>
#include "../common/message.hpp"

namespace chat {

// 定义WebSocket客户端类型，使用无TLS的ASIO配置
using WebSocketClient = websocketpp::client<websocketpp::config::asio_client>;
using ConnectionHdl = websocketpp::connection_hdl;

/**
 * @brief WebSocket聊天客户端类
 * 
 * 提供以下功能：
 * - 连接到WebSocket服务器
 * - 发送和接收消息
 * - 处理连接状态
 * - 自动重连
 */
class ChatClient {
public:
    /**
     * @brief 构造函数
     * @param username 客户端用户名
     */
    ChatClient(const std::string& username);

    /**
     * @brief 析构函数
     */
    ~ChatClient();

    /**
     * @brief 连接到WebSocket服务器
     * @param uri 服务器URI（格式：ws://host:port）
     */
    void connect(const std::string& uri);

    /**
     * @brief 断开与服务器的连接
     */
    void disconnect();

    /**
     * @brief 发送消息到服务器
     * @param message 要发送的消息内容
     */
    void send(const std::string& message);

    /**
     * @brief 设置消息处理回调函数
     * @param callback 消息处理函数
     */
    void setMessageCallback(std::function<void(const Message&)> callback);

    /**
     * @brief 检查是否已连接到服务器
     * @return 如果已连接返回true，否则返回false
     */
    bool isConnected() const { return connected; }

private:
    /**
     * @brief 处理连接建立事件
     * @param hdl 连接句柄
     */
    void onOpen(ConnectionHdl hdl);

    /**
     * @brief 处理连接关闭事件
     * @param hdl 连接句柄
     */
    void onClose(ConnectionHdl hdl);

    /**
     * @brief 处理接收到的消息
     * @param hdl 连接句柄
     * @param msg 消息内容
     */
    void onMessage(ConnectionHdl hdl, WebSocketClient::message_ptr msg);

    /**
     * @brief 运行客户端事件循环
     */
    void run();

    WebSocketClient client;           ///< WebSocket客户端实例
    ConnectionHdl connection;         ///< 当前连接句柄
    std::string username;            ///< 客户端用户名
    std::function<void(const Message&)> messageCallback;  ///< 消息处理回调函数
    bool connected;                  ///< 连接状态
};

} // namespace chat 