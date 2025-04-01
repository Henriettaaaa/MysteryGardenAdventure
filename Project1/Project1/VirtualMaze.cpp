#include "VirtualMaze.h"
#include <iostream>

VirtualMaze::VirtualMaze(int physicalSize) : size(physicalSize) {
    virtualSize = 2 * size + 1;
    // 初始化虚拟迷宫，全部置为墙（0）
    maze.resize(virtualSize, std::vector<int>(virtualSize, 0));
    
    // 创建SFML窗口
    window.create(sf::VideoMode(virtualSize * CELL_SIZE, virtualSize * CELL_SIZE), 
                "Virtual Maze", sf::Style::Close);
    window.setFramerateLimit(60);
}

// 物理坐标到虚拟坐标的映射
int VirtualMaze::physicalToVirtual(int physicalRow, int physicalCol) const {
    return physicalCol * 2 + 1 + virtualSize * (physicalRow * 2 + 1);
}

// 连接两个物理格子之间的虚拟路径
void VirtualMaze::connectPhysicalCells(int cell1, int cell2) {
    // 计算物理格子的行列号
    int row1 = cell1 / size;
    int col1 = cell1 % size;
    int row2 = cell2 / size;
    int col2 = cell2 % size;
    
    // 计算虚拟迷宫中对应的位置
    int vRow1 = row1 * 2 + 1;
    int vCol1 = col1 * 2 + 1;
    int vRow2 = row2 * 2 + 1;
    int vCol2 = col2 * 2 + 1;
    
    // 设置两个端点为路
    maze[vRow1][vCol1] = 1;
    maze[vRow2][vCol2] = 1;
    
    // 设置中间的连接点为路
    maze[(vRow1 + vRow2) / 2][(vCol1 + vCol2) / 2] = 1;
}

void VirtualMaze::buildFromPhysical(const PhysicalMaze& physicalMaze) {
    // 遍历物理迷宫中的所有格子对，检查是否相连
    for (int i = 0; i < size * size; i++) {
        int row = i / size;
        int col = i % size;
        
        // 检查右边的格子
        if (col < size - 1) {
            int next = i + 1;
            if (!physicalMaze.hasWall(i, next)) {
                connectPhysicalCells(i, next);
            }
        }
        
        // 检查下边的格子
        if (row < size - 1) {
            int next = i + size;
            if (!physicalMaze.hasWall(i, next)) {
                connectPhysicalCells(i, next);
            }
        }
    }
}

void VirtualMaze::drawMaze() {
    window.clear(sf::Color::White);
    
    // 绘制迷宫
    for (int i = 0; i < virtualSize; i++) {
        for (int j = 0; j < virtualSize; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);
            
            if (maze[i][j] == 0) {  // 墙
                cell.setFillColor(sf::Color(0x4E, 0x79, 0x35));
            } else {  // 路
                cell.setFillColor(sf::Color::White);
            }
            
            window.draw(cell);
        }
    }
}

void VirtualMaze::display() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        
        drawMaze();
        window.display();
    }
}

void VirtualMaze::highlightPath(const std::vector<int>& path) {
    // 在现有迷宫的基础上，高亮显示路径
    for (int cell : path) {
        int row = cell / size;
        int col = cell % size;
        int vRow = row * 2 + 1;
        int vCol = col * 2 + 1;
        
        // 创建一个红色方块表示路径
        sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        pathCell.setPosition(vCol * CELL_SIZE, vRow * CELL_SIZE);
        pathCell.setFillColor(sf::Color::Red);
        window.draw(pathCell);
    }
    
    window.display();
} 