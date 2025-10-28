#ifndef HEXGAMERUNNER_H
#define HEXGAMERUNNER_H

#include <string>

// 前向声明
class UserManager;

// 界面选择模式结果
struct ModeSelection {
bool valid = false;
bool isServer = false;
std::string serverIP = "";
unsigned short port = 54000;
};

// 声明一个函数：图形界面选择模式
ModeSelection selectGameMode();

// 声明真正运行游戏逻辑的函数
// 新增：接受UserManager参数
int Hexgame_run(int argc, char* argv[], UserManager* userManager = nullptr);

#endif // HEXGAMERUNNER_H