#include "ConsoleUICYJ.h"
#include <filesystem>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#ifdef _WIN32
int wmain(int argc,wchar_t** argv) {
#else
int main(int argc,char** argv) {
#endif
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); SetConsoleCP(CP_UTF8);
#endif
    try {
        // Windows receives native UTF-16 arguments; do not reinterpret ANSI argv as UTF-8.
#ifdef _WIN32
        const auto directory=argc>1?std::filesystem::path(argv[1]):std::filesystem::path("data");
#else
        const auto directory=argc>1?std::filesystem::u8path(argv[1]):std::filesystem::path("data");
#endif
        PlatformControllerCYJ controller(directory);
        const bool loaded=controller.initialize();
        std::cout<<(loaded?"已加载本地数据\n":"首次启动，将创建本地数据\n")<<"数据目录: "<<directory.u8string()<<'\n';
        ConsoleUICYJ(controller).run(); return 0;
    } catch(const std::exception& e) { std::cerr<<"错误: "<<e.what()<<"\n已停止运行，请检查数据文件或目录权限。\n"; return 1; }
}
