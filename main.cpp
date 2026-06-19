#include"base.h"
#include"config.h"
#include"agent.h"
#include"napcat_bot.h"
#include <curl/curl.h>

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    Config::get().load("config.json");   // 启动时加载配置，失败则使用内置默认值
    /*SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);*/
    std::cout << CLR_BOLD CLR_CYAN
        << "╔══════════════════════════════════════╗\n"
        << "║        chenshuzhe'S Toolkit          ║\n"
        << "╠══════════════════════════════════════╣\n"
        << "║  [1] Agent 模式 (Deepseek 对话)      ║\n"
        << "║  [2] NapCat 模式 (QQ 机器人)         ║\n"
        << "╚══════════════════════════════════════╝\n"
        << CLR_RESET;
    std::cout << "请选择模式: ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "2") {
        Napcat napcat;
        napcat.run();
    } else {
        agent* p = new agent;
        p->run();
        delete p;
    }

#ifdef _WIN32
    system("pause");
#endif
    curl_global_cleanup();
    return 0;
}