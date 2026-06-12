# GYOJ Windows 考试客户端

这是一个 Qt/C++ 原生客户端原型，面向 Windows OI 竞赛场景。

## 功能

- 全屏置顶考试界面，单实例运行。
- 低级键盘钩子拦截常见切屏快捷键。
- 剪贴板监控与违规上报。
- 进程黑名单轮询。
- 可选 Windows 防火墙白名单。
- 内置 MinGW/g++ 本地样例测评。
- 调用 OnlineJudge API 获取题目、提交代码、轮询状态。

## 依赖

- Qt 5.15+ 或 Qt 6.x，模块：Widgets、Network。
- Windows SDK。
- 可选：QScintilla。未安装时自动使用 `QPlainTextEdit`。
- 内置编译器路径默认：`client/gyoj-client/tools/mingw/bin/g++.exe`。

## 构建

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64"
cmake --build build
```

首次演示可先不启用防火墙白名单，避免异常退出影响网络；最终机房部署应配套管理员权限和恢复脚本。

## 本地预览

没有真实 OJ 后端或管理员权限时，可以先用窗口化演示模式启动：

```powershell
gyoj-client.exe --demo
```

演示模式不会安装键盘钩子、不会全屏锁定、不会连接远程 OJ，只用于检查界面和本地样例测评流程。

## QingdaoU/OnlineJudge 集成使用

客户端顶部工具栏用于把 OnlineJudge 和本地编辑器合在一个软件里：

- `OJ`：OnlineJudge 服务地址，例如 `https://oj.example.com`。
- `Cookie`：已登录浏览器中的 Cookie，例如 `sessionid=...; csrftoken=...`。
- `Contest`：比赛 ID。
- `Problem`：比赛题目展示 ID，例如 `A`、`B`。
- `Load OJ Problem`：调用 `/api/contest/problem` 拉取真实题面、样例和模板。
- `Run Sample`：用本地 MinGW 编译运行当前代码和样例输入。
- `Submit Remote`：调用 `/api/submission` 提交到远程 OnlineJudge 判题。
