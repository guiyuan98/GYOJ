# Windows 考试客户端设计说明

## 当前主客户端

当前主客户端位于：

```text
client/oj-shell
```

它使用 Electron 实现。主窗口直接加载自部署 OnlineJudge 页面，保证学生看到的就是原 OJ 界面；本地 C++ 编辑器作为工具窗口自动打开。

## 核心模块

- `main.js`：Electron 主进程，负责窗口、监考限制、本地编译运行、进程检测。
- `tools.html` / `tools.js`：本地 C++ 编辑器和样例运行器。
- `lock.html` / `lock.js`：考试锁定界面和教师密码解锁。
- `preload.js`：安全暴露 IPC 方法。
- `gyoj-shell.json`：OJ 地址、编译器路径、监考策略配置。

## 默认启动行为

正式模式执行：

```powershell
npm start
```

默认行为：

1. 打开 `serverBaseUrl` 指向的自部署 OJ。
2. 设置中文语言偏好。
3. 自动打开本地 C++ 编辑器。
4. 全屏并置顶。
5. 禁止右键菜单和复制粘贴快捷键。
6. 周期性清空剪贴板。
7. 离开客户端后锁定考试。
8. 检测黑名单进程。

开发调试模式执行：

```powershell
npm run start:dev
```

开发模式不会启用锁屏和剪贴板清理，方便调试。

## 本地样例运行

本地运行流程：

1. 将编辑器代码写入临时目录 `main.cpp`。
2. 调用 MinGW/g++ 编译：

   ```text
   g++ -std=c++14 -O2 -pipe main.cpp -o main.exe
   ```

3. 将样例输入写入程序标准输入。
4. 捕获标准输出、标准错误、退出码和耗时。
5. 超时后终止进程。

本地样例运行只用于调试，不替代远程隐藏数据判题。

## 监考策略

当前客户端实现：

- 复制粘贴拦截：`Ctrl+C`、`Ctrl+V`、`Ctrl+X`、`Shift+Insert`。
- 右键菜单禁用。
- 剪贴板定时清空。
- 失焦锁定。
- `Alt+Tab` 和 `PrintScreen` 检测后锁定。
- 黑名单进程检测后锁定。
- 教师密码解锁。

配置文件示例：

```json
{
  "proctor": {
    "enabled": true,
    "teacherUnlockPassword": "123456",
    "clearClipboard": true,
    "lockOnBlur": true,
    "processBlacklist": ["chrome.exe", "msedge.exe", "wechat.exe", "code.exe"]
  }
}
```

## 后续增强

- 将违规事件通过 `/api/proctoring/event` 上报服务端。
- 将心跳通过 `/api/proctoring/session/heartbeat` 上报服务端。
- 增加网络白名单和考试结束恢复逻辑。
- 增加客户端签名和自动更新。
