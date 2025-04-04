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

    // 物理坐标到虚拟坐标的映射
    int physicalToVirtual(int physicalRow, int physicalCol) const;
    // !物理路径到虚拟路径的转换（考虑中间连接点），新加的, 传入物理最短路径，返回虚拟最短路径
    void convertToVirtualPath(const std::vector<int>& physicalPath);
    // 连接两个物理格子之间的虚拟路径
    void connectPhysicalCells(int cell1, int cell2);
    // 绘制迷宫
    void drawMaze();
    // 绘制按钮
    void drawButton();
    // 检查点击是否在按钮上
    bool isButtonClicked(sf::Vector2i mousePos) const;
    // 绘制相对路径
    void drawRelativePaths();
    // 计算相对路径
    void calculateRelativePath(const std::vector<int>& physicalPath);

public:
    VirtualMaze(int physicalSize);
    // 从物理迷宫构建虚拟迷宫
    void buildFromPhysical(const PhysicalMaze& physicalMaze);
    // 显示迷宫
    void display();
    // 设置路径
    void setPath(const std::vector<int>& path);
};

#endif // VIRTUAL_MAZE_H