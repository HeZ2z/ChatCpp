# ChatCpp - C++ WebSocket Chat System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-blue.svg)](https://cmake.org/)

一个高性能的基于C++和WebSocket的聊天系统，支持多用户实时通信、消息持久化、日志记录和完整的测试套件。

## 📋 功能特点

### 核心功能
- 🚀 **实时通信**: 基于WebSocket的多用户实时聊天
- 📝 **消息持久化**: 自动保存聊天历史记录
- 📊 **日志记录**: 完整的消息和系统日志
- 🖥️ **命令行客户端**: 简洁的CLI界面
- 🔒 **内存安全**: 集成AddressSanitizer内存泄漏检测

### 开发与测试
- 🧪 **单元测试**: 基于Google Test的完整测试套件
- 📈 **测试管理**: JIRA/Xray集成，支持测试用例管理和报告
- 🔍 **内存泄漏检测**: Valgrind兼容的内存测试
- 📦 **现代化构建**: CMake构建系统，C++17标准

## 🏗️ 项目结构

```
ChatCpp/
├── src/                    # 源代码目录
│   ├── client/            # 客户端实现
│   ├── server/            # 服务器实现
│   └── common/            # 公共组件
├── tests/                 # 测试文件
├── test-management/       # 测试管理
│   ├── reports/          # 测试报告
│   ├── test-cases/       # 测试用例
│   ├── test-executions/  # 测试执行记录
│   └── test-plans/       # 测试计划
├── scripts/              # 构建和测试脚本
├── config/               # 配置文件
└── build/                # 构建输出目录 (自动生成)
```

## 🛠️ 环境要求

### 系统要求
- **操作系统**: Linux/macOS/Windows (WSL2推荐)
- **编译器**: GCC 7+ / Clang 5+ / MSVC 2019+
- **构建工具**: CMake 3.10+

### 依赖库
- **OpenSSL**: WebSocket加密通信 (可选)
- **Threads**: POSIX线程支持
- **Google Test**: 单元测试框架 (自动下载)

## 🚀 快速开始

### 克隆项目
```bash
git clone git@github.com:HeZ2z/ChatCpp.git
cd ChatCpp
```

### 构建项目

#### 标准构建
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译项目
make -j$(nproc)
```

#### AddressSanitizer构建 (推荐用于开发)
```bash
mkdir build_asan && cd build_asan
cmake -DENABLE_ASAN=ON ..
make -j$(nproc)
```

#### 内存泄漏测试构建
```bash
mkdir build_leak && cd build_leak
cmake -DENABLE_ASAN=OFF -DENABLE_MEMORY_LEAK_TEST=ON ..
make -j$(nproc)
```

### 运行测试
```bash
# 在构建目录中运行所有测试
ctest --output-on-failure

# 或直接运行测试可执行文件
./chat_tests
```

### JIRA/Xray测试管理
```bash
# 运行测试并生成Xray报告
python3 scripts/run_tests_xray.py

# 生成Xray兼容的测试报告
python3 scripts/generate_xray_report.py
```

## 💻 使用方法

> **注意**: 当前版本主要包含测试套件，完整聊天功能正在开发中。

### 运行单元测试
```bash
cd build
./chat_tests --gtest_output=xml:test_results.xml
```

### 内存泄漏检测 (使用Valgrind)
```bash
cd build_leak
valgrind --leak-check=full --show-leak-kinds=all ./chat_tests
```

## 🔧 开发设置

### IDE配置
- **VS Code**: 推荐使用C++扩展
- **CLion**: 原生CMake支持
- **Cursor**: AI辅助开发环境

### 代码风格
- 使用C++17标准
- Google C++代码风格
- 启用所有编译器警告

## 📊 测试覆盖

项目包含以下测试类型：
- **单元测试**: 消息处理、日志记录、历史记录
- **内存测试**: AddressSanitizer和Valgrind集成
- **集成测试**: WebSocket通信测试 (开发中)

## 🤝 贡献指南

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 开发流程
- 所有代码都需要单元测试
- 确保通过所有CI检查
- 更新相关文档

## 📝 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙋‍♂️ 支持

如果您有问题或建议：

1. 查看[故障排除指南](#故障排除)
2. 提交 [Issue](https://github.com/your-repo/issues)
3. 联系维护者

## 🔍 故障排除

### 常见问题

**构建失败**
- 确保安装了CMake 3.10+
- 检查编译器是否支持C++17

**测试失败**
- 确保所有依赖都正确安装
- 检查构建配置是否正确

**内存泄漏检测**
- AddressSanitizer构建用于开发环境
- Valgrind用于生产环境内存分析

### 调试技巧
- 使用 `-DENABLE_ASAN=ON` 启用内存检测
- 查看 `test-management/reports/` 中的测试报告
- 检查构建日志中的详细错误信息

---

**开发状态**: 🚧 核心功能开发中 - 欢迎贡献！
