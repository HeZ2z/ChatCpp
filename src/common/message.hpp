#pragma once

#include <string>
#include <ctime>

namespace chat {

/**
 * @brief 聊天消息类，用于封装聊天消息的各个属性
 * 
 * 消息格式：username @ content | timestamp
 * 例如：alice @ 你好 | 2025-04-16 10:00:00
 */
struct Message {
    std::string username;    ///< 发送者的用户名
    std::string content;     ///< 消息内容
    time_t timestamp;        ///< 消息发送时间戳

    /**
     * @brief 构造函数
     * @param user 用户名
     * @param msg 消息内容
     */
    Message(const std::string& user, const std::string& msg)
        : username(user), content(msg), timestamp(std::time(nullptr)) {}

    /**
     * @brief 将消息转换为字符串格式
     * @return 格式化后的消息字符串
     */
    std::string toString() const;

    /**
     * @brief 从字符串解析消息
     * @param str 格式化的消息字符串
     * @return 解析后的Message对象
     */
    static Message fromString(const std::string& str);
};

} // namespace chat 