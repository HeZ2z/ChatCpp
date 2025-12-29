# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(ChatCpp)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AddressSanitizer 编译选项
if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

# 启用测试
enable_testing()

# AddressSanitizer 选项 (默认启用用于内存泄漏检测)
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

# 内存泄漏测试选项 (用于演示 Valgrind 流程)
option(ENABLE_MEMORY_LEAK_TEST "Enable intentional memory leak for testing Valgrind" OFF)

# 查找必要的依赖包
find_package(OpenSSL QUIET)  # OpenSSL库，用于WebSocket的加密通信 (optional)
find_package(Threads REQUIRED)  # 线程库，用于多线程支持

# 查找websocketpp (使用pkg-config或手动查找)
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(WEBSOCKETPP websocketpp)
endif()

if(NOT WEBSOCKETPP_FOUND)
    # 如果pkg-config找不到，尝试手动查找websocketpp头文件
    find_path(WEBSOCKETPP_INCLUDE_DIR websocketpp/server.hpp
        PATHS /usr/include /usr/local/include
        PATH_SUFFIXES websocketpp)
    if(WEBSOCKETPP_INCLUDE_DIR)
        set(WEBSOCKETPP_FOUND TRUE)
        set(WEBSOCKETPP_INCLUDE_DIRS ${WEBSOCKETPP_INCLUDE_DIR})
    endif()
endif()

# 添加 Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# 添加源文件
# 服务器端源文件
set(SERVER_SOURCES
    src/server/main.cpp          # 服务器主程序
    src/server/websocket_server.cpp  # WebSocket服务器实现
    src/common/message.cpp       # 消息处理
    src/common/logger.cpp        # 日志记录
)

# 客户端源文件
set(CLIENT_SOURCES
    src/client/main.cpp          # 客户端主程序
    src/client/websocket_client.cpp  # WebSocket客户端实现
    src/common/message.cpp       # 消息处理
    src/common/logger.cpp        # 日志记录
)

# 创建可执行文件
add_executable(server ${SERVER_SOURCES})  # 创建服务器可执行文件
add_executable(client ${CLIENT_SOURCES})  # 创建客户端可执行文件

# 设置包含目录
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${WEBSOCKETPP_INCLUDE_DIRS}
)

target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${WEBSOCKETPP_INCLUDE_DIRS}
)

# 创建基本的测试可执行文件（不包含需要OpenSSL的WebSocket测试）
add_executable(chat_tests
    tests/message_test.cpp
    tests/logger_test.cpp
    tests/history_test.cpp
    src/common/message.cpp
    src/common/logger.cpp
)

# 设置包含目录
target_include_directories(chat_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# 添加编译定义
if(ENABLE_MEMORY_LEAK_TEST)
    target_compile_definitions(chat_tests PRIVATE ENABLE_MEMORY_LEAK_TEST)
endif()

# 链接必要的库
# 服务器端链接库
target_link_libraries(server
    PRIVATE
    Threads::Threads    # 线程库
)

# 客户端链接库
target_link_libraries(client
    PRIVATE
    Threads::Threads    # 线程库
)

# 测试链接库 (基本测试不需要OpenSSL)
target_link_libraries(chat_tests
    PRIVATE
    GTest::gtest_main
    GTest::gmock_main
    Threads::Threads    # 线程库
)

# 添加测试
include(GoogleTest)
gtest_discover_tests(chat_tests)