#include "message.hpp"
#include <sstream>
#include <iomanip>

namespace chat {

/**
 * @brief 将消息转换为字符串格式
 * 
 * 格式：username @ content | YYYY-MM-DD HH:MM:SS
 * 例如：alice @ 你好 | 2025-04-16 10:00:00
 */
std::string Message::toString() const {
    std::stringstream ss;
    char timeStr[20];
    // 将时间戳转换为可读的日期时间字符串
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
    // 按照指定格式组合消息
    ss << username << " @ " << content << " | " << timeStr;
    return ss.str();
}

/**
 * @brief 从字符串解析消息
 * 
 * 解析格式：username @ content | YYYY-MM-DD HH:MM:SS
 * 例如：alice @ 你好 | 2025-04-16 10:00:00
 * 
 * @param str 格式化的消息字符串
 * @return 解析后的Message对象
 * @throw std::runtime_error 当字符串格式不正确时抛出异常
 */
Message Message::fromString(const std::string& str) {
    // 验证基本格式
    if (str.empty() || str.find('@') == std::string::npos || str.find('|') == std::string::npos) {
        throw std::runtime_error("Invalid message format: missing required separators");
    }

    std::stringstream ss(str);
    std::string username, content, timeStr;
    
    // 解析用户名（@符号前的部分）
    std::getline(ss, username, '@');
    if (username.empty()) {
        throw std::runtime_error("Invalid message format: empty username");
    }
    // 去除用户名前后的空白字符
    username = username.substr(0, username.find_last_not_of(" \t") + 1);
    
    // 解析消息内容（@和|之间的部分）
    std::getline(ss, content, '|');
    if (content.empty()) {
        throw std::runtime_error("Invalid message format: empty content");
    }
    // 去除内容前后的空白字符
    content = content.substr(content.find_first_not_of(" \t"), 
                           content.find_last_not_of(" \t") - content.find_first_not_of(" \t") + 1);
    
    // 解析时间戳（|后的部分）
    std::getline(ss, timeStr);
    if (timeStr.empty()) {
        throw std::runtime_error("Invalid message format: empty timestamp");
    }
    timeStr = timeStr.substr(timeStr.find_first_not_of(" \t"));
    
    // 创建消息对象
    Message msg(username, content);
    
    // 解析时间字符串为time_t类型
    std::tm tm = {};
    std::stringstream timeStream(timeStr);
    timeStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (timeStream.fail()) {
        throw std::runtime_error("Invalid message format: invalid timestamp format");
    }
    msg.timestamp = std::mktime(&tm);
    
    return msg;
}

} // namespace chat 