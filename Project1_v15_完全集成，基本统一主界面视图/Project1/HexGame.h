#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <sstream>
#include <thread>
#include "SingleHex.h"
#include "NetworkManager.h"

// 游戏状态结构体
struct GameState {
    std::unordered_map<HexCoord, CellState> gridData;
    std::unordered_map<HexCoord, std::vector<int>> gridNumbers; // 格子标号数据
    HexCoord playerPos;
    HexCoord otherPlayerPos;
    HexCoord startHex;
    HexCoord endHex;
    bool isServer;
    bool gameStarted;
    bool clientReady; // 客户端就绪标志
    bool gameTimeSynced; // 游戏时间同步标志
    float gameTime;
    bool gameEnded;
    std::mt19937 rng; // 随机数生成器
    int checkA; // A路径检查点
    std::vector<HexCoord> pathA; // 用于检查玩家位置的A路径
    int checkB; // 第二个玩家的B路径检查点
    std::vector<HexCoord> pathB; // 第二个玩家的B路径
    bool isSecondPlayerChecked; // 是否对第二个玩家进行点位检查
    // 添加服务器端检查点数据字段
    int serverCheckA;    // 从服务器接收的checkA值
    int serverCheckB;    // 从服务器接收的checkB值
    int serverPathSize;  // 从服务器接收的pathA.size()值
};

// 六边形游戏类
class HexGame {
public:
    // 构造函数
    HexGame(bool isServer, const std::string& serverIP = "", unsigned short port = 54000);
    
    // 析构函数
    ~HexGame();
    
    // 初始化游戏
    bool initialize();
    
    // 运行游戏
    void run();
    
    // 处理游戏事件
    void processEvents();
    
    // 更新游戏状态
    void update();
    
    // 渲染游戏
    void render();
    
    // 游戏是否运行中
    bool isRunning() const;

private:
    // 初始化游戏状态
    void initializeGameState();
    
    // 设置网络回调
    void setupNetworkCallbacks();
    
    // 向客户端发送客户端就绪消息
    void sendClientReady();
    
    // 服务器发送游戏开始信号
    void sendGameStartSignal();
    
    // 处理网络消息
    void handleNetworkMessages();
    
    // 检查玩家移动是否正确
    bool checkPlayerMovement(const HexCoord& newPos);
    
    // 检查第二个玩家的移动是否正确
    bool checkOtherPlayerMovement(const HexCoord& newPos);
    
    // 处理键盘输入
    void handleKeyPress(sf::Keyboard::Key key);

private:
    GameState state;                 // 游戏状态
    NetworkManager& networkManager;  // 网络管理器
    sf::RenderWindow window;         // 渲染窗口
    sf::Vector2f gridOrigin;         // 网格原点
    sf::Clock gameClock;             // 游戏时钟
    sf::Clock checkpointClock;       // 检查点更新时钟
    bool clockStarted;               // 时钟启动标志
    
    std::string serverIP;            // 服务器IP
    unsigned short port;             // 端口号
}; 