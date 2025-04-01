#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "PhysicalMaze.h"

class VirtualMaze {
private:
    int size;  // 物理迷宫的大小
    int virtualSize;  // 虚拟迷宫的大小 (2*size + 1)
    std::vector<std::vector<int>> maze;  // 0表示墙，1表示路
    sf::RenderWindow window;  // SFML窗口
    const float CELL_SIZE = 30.0f;  // 每个格子的像素大小
    
    // 物理坐标到虚拟坐标的映射
    int physicalToVirtual(int physicalRow, int physicalCol) const;
    // 连接两个物理格子之间的虚拟路径
    void connectPhysicalCells(int cell1, int cell2);
    // 绘制迷宫
    void drawMaze();

public:
    VirtualMaze(int physicalSize);
    // 从物理迷宫构建虚拟迷宫
    void buildFromPhysical(const PhysicalMaze& physicalMaze);
    // 显示迷宫
    void display();
    // 高亮显示路径
    void highlightPath(const std::vector<int>& path);
};
