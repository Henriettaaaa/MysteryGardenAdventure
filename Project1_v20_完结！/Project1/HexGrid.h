#pragma once
#include "SingleHex.h"
#include <chrono> // 添加计时器所需的头文件

// 六边形网格类，封装网格生成、寻路和显示
class HexGrid {
public:
    // 构造函数
    HexGrid(int radius);
    
    // 初始化网格
    void initialize();
    
    // 运行网格生成和寻路逻辑
    void run();
    
    // 显示网格
    void display();
    
    // 处理键盘输入
    void handleInput(sf::Event event);
    
    // 更新游戏状态
    void update();
    
private:
    // 网格半径
    int gridRadius;

    int check_A = 1;  //维护一个指针，用户走对一个格子，指针就往后移动一下
    
    // 网格数据
    std::unordered_map<HexCoord, CellState> gridData;
    
    // 保存每个格子的原始状态（用于玩家移动后恢复）
    std::unordered_map<HexCoord, CellState> originalGridState;
    
    // 存储每个格子的序号向量
    std::unordered_map<HexCoord, std::vector<int>> gridNumbers;
    
    // 边界单元格
    std::vector<HexCoord> boundaryCells;
    
    // 起点和终点
    HexCoord startHex, endHex;
    
    // 路径单元格
    std::vector<HexCoord> pathCells;

    //A的绝对路径
    std::vector<HexCoord> pathCells_A;

    //B的绝对路径
    std::vector<HexCoord> pathCells_B;

    //相对路径单元格
    std::vector<HexCoord> relativePathCells;
    
    // 已访问节点
    std::unordered_set<HexCoord> visited;
    
    // 路径来源
    std::unordered_map<HexCoord, HexCoord> cameFrom;
    
    // 随机数生成器
    std::mt19937 rng;
    
    // SFML窗口
    sf::RenderWindow window;
    
    // 网格原点
    sf::Vector2f gridOrigin;
    
    // 玩家位置
    HexCoord playerPos;
    
    // 游戏是否结束
    bool gameOver;
    
    // 计时器相关变量
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime; // 计时开始时间
    std::chrono::duration<double> elapsedTime; // 已经过时间
    bool timerRunning = false; // 计时器是否在运行
    
    // 绘制计时器
    void drawTimer();
}; 