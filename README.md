# 模拟即时通信系统

C++17 控制台项目。业务类使用 `CYJ` 后缀，一类一文件。

## 编译与运行（Visual Studio / CMake）

在「Developer PowerShell for VS」或已配置好的终端中：

请先打开 **“Developer PowerShell for VS”**（开始菜单搜 Visual Studio），不要用普通 PowerShell（里面通常没有 `cmake`）。

```powershell
cd "C:\Users\BF2024\Desktop\作业\大三上\课设\InstantMessageSystem"
cmake -S . -B out/build/x64-Debug
cmake --build out/build/x64-Debug --config Debug --parallel 4
.\out\build\x64-Debug\Debug\InstantMessageSystem.exe .\data
```

也可编译后在项目根目录双击 `run.cmd`。

数据目录：`data\platform.txt`。

## 使用要点

- 统一用户 ID（如 `UID1`）与服务号码（如 QQ `10001`、微信 `wx8001`）是两层 ID。
- 登录后选定「当前操作服务」；其它已开通服务可同时在线。
- 好友/群操作默认针对当前服务账号。

## 目录

- `include/`、`src/`、`main.cpp`：源码
- `tests/`：业务与菜单测试
- `data/`：持久化数据
- `CMakeLists.txt`、`build.ps1`、`run.cmd`：构建与启动
