#pragma once

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <set>
#include <memory>
#include <functional>
#include <string>
#include "../common/message.hpp"

namespace chat {

// 定义WebSocket服务器类型，使用无TLS的ASIO配置
using WebSocketServer = websocketpp::server<websocketpp::config::asio>;
using ConnectionPtr = WebSocketServer::connection_ptr;
using ConnectionHdl = websocketpp::connection_hdl;

/**
 * @brief WebSocket聊天服务器类
 * 
 * 提供以下功能：
 * - 接受客户端连接
 * - 广播消息给所有连接的客户端
 * - 处理客户端消息
 * - 管理连接生命周期
 */
class ChatServer {
public:
    /**
     * @brief 构造函数
     * @param port 服务器监听端口
     */
    ChatServer(uint16_t port = 10808);

    /**
     * @brief 析构函数
     */
    ~ChatServer();

    /**
     * @brief 启动服务器
     */
    void start();

    /**
     * @brief 停止服务器
     */
    void stop();

    /**
     * @brief 广播消息给所有连接的客户端
     * @param message 要广播的消息
     */
    void broadcast(const std::string& message);

    /**
     * @brief 设置消息处理回调函数
     * @param callback 消息处理函数
     */
    void setMessageCallback(std::function<void(const Message&)> callback);

    /**
     * @brief 检查服务器是否正在运行
     * @return 如果服务器正在运行返回true，否则返回false
     */
    bool isRunning() const { return running; }

private:
    /**
     * @brief 处理新的客户端连接
     * @param hdl 连接句柄
     */
    void onOpen(ConnectionHdl hdl);

    /**
     * @brief 处理客户端断开连接
     * @param hdl 连接句柄
     */
    void onClose(ConnectionHdl hdl);

    /**
     * @brief 处理接收到的消息
     * @param hdl 连接句柄
     * @param msg 消息内容
     */
    void onMessage(ConnectionHdl hdl, WebSocketServer::message_ptr msg);

    /**
     * @brief 运行服务器事件循环
     */
    void run();

    WebSocketServer server;                    ///< WebSocket服务器实例
    std::set<ConnectionHdl, std::owner_less<ConnectionHdl>> connections;  ///< 当前连接的客户端集合
    std::function<void(const Message&)> messageCallback;  ///< 消息处理回调函数
    bool running;                             ///< 服务器运行状态
    uint16_t port;                           ///< 服务器监听端口
};

} // namespace chat 