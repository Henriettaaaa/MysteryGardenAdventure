//#pragma once

#ifndef VIRTUAL_MAZE_H
#define VIRTUAL_MAZE_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "PhysicalMaze.h"
#include <map>
#include <utility>
#include <string>

// 前向声明
class UserManager;
enum class Difficulty;

//虚拟迷宫的底层是二维数组，用横纵坐标确定位置
class VirtualMaze {
private:
    int size;  // 物理迷宫的大小
    int virtualSize;  // 虚拟迷宫的大小 (2*size + 1)
    std::vector<std::vector<int>> maze;  // 0表示墙，1表示路
    sf::RenderWindow window;  // SFML窗口
    const float CELL_SIZE = 30.0f;  // 每个格子的像素大小

    // 新增：用于存储每个位置的所有数字
    std::map<std::pair<int, int>, std::vector<std::pair<int, sf::Color>>> cellNumbers;  // 存储每个位置的数字和对应的颜色

    // 按钮相关
    sf::RectangleShape button;
    sf::Text buttonText;
    sf::Font font;
    bool showRelativePaths;  // 是否显示相对路径

    // 路径相关
    std::vector<int> absolutePathA;  //A自己走过的绝对路径，要在B玩时显示
    std::vector<int> absolutePathB;  //B自己走过的绝对路径，要在A玩时显示
    
    std::vector<int> absolutePath;  // 存储绝对路径的虚拟坐标
    //absolutepath是有必要的，这样不需要每次都从物理路径 path 计算，虚拟路径一直要用
    
    

    std::vector<int> relativePathA;  // A相对B的路径的虚拟坐标
    std::vector<int> relativePathB;  // B相对A的路径的虚拟坐标

    // 玩家互动相关
    bool playerInteractionEnabled;  // 是否启用玩家互动
    std::pair<int, int> playerPositionA;  // 玩家A当前位置（虚拟坐标）
    std::pair<int, int> playerPositionB;  // 玩家B当前位置（虚拟坐标）
    sf::Color playerColorA;  // 玩家A颜色
    sf::Color playerColorB;  // 玩家B颜色
    std::pair<int, int> lastPlayerPositionA;  // 玩家A上一个位置
    std::pair<int, int> lastPlayerPositionB;  // 玩家B上一个位置
    bool playersHaveMet;  // 玩家是否已相遇，用于触发第二阶段
    int mazefaster;  // 0表示A跑得快，1表示B跑得快

    // 单人模式计时器相关
    sf::Clock gameClock;          // 游戏计时器
    sf::Text timerText;           // 计时器显示文本
    bool isTimerRunning;          // 计时器是否正在运行
    float elapsedTime;            // 已经过的时间（秒）
    
    // 道具和绝对路径按钮相关
    sf::RectangleShape itemButtonA;     // 玩家A的道具按钮
    sf::RectangleShape itemButtonB;     // 玩家B的道具按钮
    sf::Text itemTextA;                 // 玩家A的道具文本
    sf::Text itemTextB;                 // 玩家B的道具文本
    sf::RectangleShape absPathButtonA;  // 玩家A的绝对路径按钮
    sf::RectangleShape absPathButtonB;  // 玩家B的绝对路径按钮
    sf::Text absPathTextA;              // 玩家A的绝对路径文本
    sf::Text absPathTextB;              // 玩家B的绝对路径文本
    bool playerAHasItem;                // 玩家A是否拥有道具
    bool playerBHasItem;                // 玩家B是否拥有道具
    bool playerAUsedItem;               // 玩家A是否使用了道具
    bool playerBUsedItem;               // 玩家B是否使用了道具
    sf::Clock itemEffectClock;          // 道具效果计时器
    bool playerADelayed;                // 玩家A是否被延迟
    bool playerBDelayed;                // 玩家B是否被延迟
    float delayDuration;                // 延迟持续时间(秒)
    bool showAbsolutePathA;             // 是否显示玩家A的绝对路径
    bool showAbsolutePathB;             // 是否显示玩家B的绝对路径

    // 物理坐标到虚拟坐标的映射
    int physicalToVirtual(int physicalRow, int physicalCol) const;
    // !物理路径到虚拟路径的转换（考虑中间连接点），新加的, 传入物理最短路径，返回虚拟最短路径
    void convertToVirtualPath(const std::vector<int>& physicalPath);
    // 连接两个物理格子之间的虚拟路径
    void connectPhysicalCells(int cell1, int cell2);
    // 绘制双人模式的迷宫
    void drawMaze();
    // 绘制单人模式的地图
    void drawSingleMap();
    // 绘制按钮
    void drawButton();
    // 检查点击是否在按钮上
    bool isButtonClicked(sf::Vector2i mousePos) const;
    // 绘制相对路径
    void drawRelativePaths();
    // 计算相对路径
    void calculateRelativePath(const std::vector<int>& physicalPath);

    int checkpoint = 1;  //单人模式中，维护一个指针，用户走对一个格子，指针就往后移动一下

    // 玩家互动相关函数
    void handleKeyPressA(sf::Keyboard::Key key);
    void handleKeyPressB(sf::Keyboard::Key key);
    void handleKeyPressA_free(sf::Keyboard::Key key);
    bool canMoveTo(int newRow, int newCol) const;
    bool canMoveTo_free(int newRow, int newCol) const;
    void drawPlayerA();
    void drawPlayerB();
    void updatePlayerCellA();
    void updatePlayerCellB();
    
    // 道具和绝对路径按钮相关函数
    void initializeItemButtons();                           // 初始化道具按钮
    void initializeAbsPathButtons();                        // 初始化绝对路径按钮
    bool isItemButtonAClicked(sf::Vector2i mousePos) const; // 检查A道具按钮是否被点击
    bool isItemButtonBClicked(sf::Vector2i mousePos) const; // 检查B道具按钮是否被点击
    bool isAbsPathButtonAClicked(sf::Vector2i mousePos) const; // 检查A绝对路径按钮是否被点击
    bool isAbsPathButtonBClicked(sf::Vector2i mousePos) const; // 检查B绝对路径按钮是否被点击

public:
    VirtualMaze(int physicalSize);
    // 从物理迷宫构建虚拟迷宫
    void buildFromPhysical(const PhysicalMaze& physicalMaze);
    // 显示双人模式的迷宫
    void display();
    // 显示单人模式的地图
    void displaySingle();
    // 设置路径
    void setPath(const std::vector<int>& path);
    // 启用玩家互动
    void enablePlayerInteraction();
    // 获取游戏时间
    float getElapsedTime() const { return elapsedTime; }
    // 新增：用于双人模式分屏显示的函数声明
    void displayMultiPlayer();
    // 新增：用于相对路径阶段的分屏显示函数声明
    void displayRelativeMaze();
};

#endif // VIRTUAL_MAZE_H