//#pragma once

#ifndef VIRTUAL_MAZE_H
#define VIRTUAL_MAZE_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "PhysicalMaze.h"
#include <map>
#include <utility>
#include <string>

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
    std::vector<int> absolutePath;  // 存储绝对路径的虚拟坐标
    //absolutepath是有必要的，这样不需要每次都从物理路径 path 计算，虚拟路径一直要用
    std::vector<int> relativePathA;  // A相对B的路径的虚拟坐标
    std::vector<int> relativePathB;  // B相对A的路径的虚拟坐标

    // 玩家互动相关
    bool playerInteractionEnabled;  // 是否启用玩家互动
    std::pair<int, int> playerPosition;  // 玩家当前位置（虚拟坐标）
    sf::Color playerColor;  // 玩家颜色
    std::pair<int, int> lastPlayerPosition;  // 玩家上一个位置

    // 单人模式计时器相关
    sf::Clock gameClock;          // 游戏计时器
    sf::Text timerText;           // 计时器显示文本
    bool isTimerRunning;          // 计时器是否正在运行
    float elapsedTime;            // 已经过的时间（秒）

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

    // 玩家互动相关函数
    void handleKeyPress(sf::Keyboard::Key key);
    bool canMoveTo(int newRow, int newCol) const;
    void drawPlayer();
    void updatePlayerCell();

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
};

#endif // VIRTUAL_MAZE_H