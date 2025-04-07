//双人迷宫游戏的单人走迷宫交互
#include "VirtualMaze.h"
#include <iostream>
#include <string>   // 为std::string
#include <utility>  // 为std::pair

// 新增：启用走迷宫的交互模式（单人）
// ! 一会要改成点击开始按钮 
/*
void VirtualMaze::enableSinglePlayerMode() {
    singlePlayerMode = true;
    playerPosition = {1, 1};  // 设置玩家位置到起点
}
*/
// 走迷宫模式下，处理按键事件，控制方块的移动
// 传入方向按键，更新玩家位置
void VirtualMaze::handleKeyPress(sf::Keyboard::Key key) {
    // 保存当前位置作为上一个位置
    lastPlayerPosition = playerPosition;
    
    // 保存当前位置
    std::pair<int, int> newPosition = playerPosition;
    
    // 根据按键更新位置
    switch (key) {
        case sf::Keyboard::Up:
            newPosition.first--;
            break;
        case sf::Keyboard::Down:
            newPosition.first++;
            break;
        case sf::Keyboard::Left:
            newPosition.second--;
            break;
        case sf::Keyboard::Right:
            newPosition.second++;
            break;
        default:
            return;  // 忽略其他键
    }
    
    // 检查新位置是否合法
    if (canMoveTo(newPosition.first, newPosition.second)) {
        playerPosition = newPosition;
        
        //如果合法
        // 检查是否到达终点
        if (playerPosition.first == virtualSize - 2 && playerPosition.second == virtualSize - 2) {
            std::cout << "恭喜！你到达了终点！" << std::endl;
        }
    }
}

// 检查是否可以移动到指定位置（碰撞检测）
// 返回一个bool，新位置是否合法
bool VirtualMaze::canMoveTo(int newRow, int newCol) const {
    // 检查是否在迷宫范围内
    if (newRow < 0 || newRow >= virtualSize || newCol < 0 || newCol >= virtualSize) {
        return false;
    }
    
    // 检查是否是墙
    return maze[newRow][newCol] != 0;
}

// 绘制玩家位置（迷宫在drawMaze()中已经绘制）
void VirtualMaze::drawPlayer() {
    sf::RectangleShape playerCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    playerCell.setPosition(playerPosition.second * CELL_SIZE, playerPosition.first * CELL_SIZE);
    playerCell.setFillColor(playerColor);
    window.draw(playerCell);
}

// 新增：绘制单人模式界面
// !这是啥玩意，好像重复了
/*
void VirtualMaze::drawSinglePlayerMode() {
    window.clear(sf::Color::White);
    
    // 先绘制迷宫
    
    for (int i = 0; i < virtualSize; i++) {
        for (int j = 0; j < virtualSize; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

            // 终点标红色
            if (i == virtualSize - 2 && j == virtualSize - 2) {  // 终点
                cell.setFillColor(sf::Color::Red);
            }
            else if (maze[i][j] == 0) {  // 墙
                cell.setFillColor(sf::Color(0x4E, 0x79, 0x35));
            }
            else {  // 路
                cell.setFillColor(sf::Color::White);
            }

            window.draw(cell);
        }
    }
    
    
    // 再绘制玩家
    drawPlayer();
}
*/

// 新增：单人模式的显示循环
/*
void VirtualMaze::displaySinglePlayerMode() {
    // 初始化玩家位置
    playerPosition = {1, 1};
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
                
                // 检查是否到达终点
                if (playerPosition.first == virtualSize - 2 && playerPosition.second == virtualSize - 2) {
                    std::cout << "恭喜！你到达了终点！" << std::endl;
                    // 这里可以添加游戏胜利逻辑
                }
            }
        }
        
        drawSinglePlayerMode();
        window.display();
    }
}
*/

// 玩家互动相关函数
void VirtualMaze::enablePlayerInteraction() {
    playerInteractionEnabled = true;
    playerPosition = {1, 1};  // 重置玩家位置到起点
    lastPlayerPosition = {1, 1};  // 初始化上一个位置
}

//传入合法的新位置，重新绘制玩家所在位置
void VirtualMaze::updatePlayerCell() {
    // 如果玩家移动了，需要将上一个位置的格子重新绘制为白色
    if (lastPlayerPosition != playerPosition) {
        sf::RectangleShape lastCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        lastCell.setPosition(lastPlayerPosition.second * CELL_SIZE, lastPlayerPosition.first * CELL_SIZE);
        
        // 如果是起点或终点，恢复为红色
        if ((lastPlayerPosition.first == 1 && lastPlayerPosition.second == 1) || 
            (lastPlayerPosition.first == virtualSize - 2 && lastPlayerPosition.second == virtualSize - 2)) {
            lastCell.setFillColor(sf::Color::Red);
        } else {
            // 否则恢复为路径的白色
            lastCell.setFillColor(sf::Color::White);
        }
        
        window.draw(lastCell);
    }
}