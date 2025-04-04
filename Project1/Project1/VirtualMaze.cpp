#include "VirtualMaze.h"
#include <iostream>
#include <string>   // 为std::string
#include <utility>  // 为std::pair

VirtualMaze::VirtualMaze(int physicalSize) : size(physicalSize), showRelativePaths(false) {
    virtualSize = 2 * size + 1;
    // 初始化虚拟迷宫，全部置为墙（0）
    maze.resize(virtualSize, std::vector<int>(virtualSize, 0));

    // 创建SFML窗口
    window.create(sf::VideoMode(virtualSize * CELL_SIZE, virtualSize * CELL_SIZE),
        "Virtual Maze", sf::Style::Close);
    window.setFramerateLimit(60);

    // 初始化按钮
    button.setSize(sf::Vector2f(200, 50));
    button.setFillColor(sf::Color::Blue);
    button.setPosition(
        (virtualSize * CELL_SIZE - button.getSize().x) / 2,
        (virtualSize * CELL_SIZE - button.getSize().y) / 2
    );

    // 加载字体
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }

    // 初始化按钮文本
    buttonText.setFont(font);
    buttonText.setString("Show Relative Paths");
    buttonText.setCharacterSize(20);
    buttonText.setFillColor(sf::Color::White);

    // 居中文本
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setPosition(
        button.getPosition().x + (button.getSize().x - textBounds.width) / 2,
        button.getPosition().y + (button.getSize().y - textBounds.height) / 2
    );
}

// 物理坐标到虚拟坐标的映射，整个盘面扩容，无改动
int VirtualMaze::physicalToVirtual(int physicalRow, int physicalCol) const {
    return physicalCol * 2 + 1 + virtualSize * (physicalRow * 2 + 1);
}

// 连接两个物理格子之间的虚拟路径，没有改动
// !代码思路可复用至最短路径
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
    //用格子的横纵坐标直接求中点坐标，比分别解决四种方向更简洁
    maze[(vRow1 + vRow2) / 2][(vCol1 + vCol2) / 2] = 1;
}

// 从物理迷宫构建虚拟迷宫，没有改动
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

void VirtualMaze::drawButton() {
    window.draw(button);
    window.draw(buttonText);
}

bool VirtualMaze::isButtonClicked(sf::Vector2i mousePos) const {
    sf::FloatRect buttonBounds = button.getGlobalBounds();
    return buttonBounds.contains(sf::Vector2f(mousePos));
}

//窗口展示，包含点击下一关之前和点击后，审查无问题
void VirtualMaze::drawMaze() {
    window.clear(sf::Color::White);
    //在没点击下一关按钮时，画普通迷宫（初始界面）
    if (!showRelativePaths) {
        // 绘制普通迷宫
        for (int i = 0; i < virtualSize; i++) {
            for (int j = 0; j < virtualSize; j++) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

                // 起点终点标红色
                if (i == 1 && j == 1) {  // 起点
                    cell.setFillColor(sf::Color::Red);
                }
                else if (i == virtualSize - 2 && j == virtualSize - 2) {  // 终点
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
        // 绘制按钮
        drawButton();
    }
    else {
        drawRelativePaths();
    }
}

// ! 物理路径到虚拟路径的转换（考虑中间连接点）,接下来要修改这个函数，处理最短路径从物理到虚拟图层的连接问题
// !只专注于解决最短路径
// 传入最短路径（序号表示），返回虚拟路径（序号表示），直接修改成员变量 absolutePath
void VirtualMaze::convertToVirtualPath(const std::vector<int>& physicalPath) {
    //std::vector<int> virtualPath;

    for (size_t i = 0; i < physicalPath.size(); i++) {
        // 当前物理格子的坐标
        int row = physicalPath[i] / size;
        int col = physicalPath[i] % size;
        int virtualPos = physicalToVirtual(row, col);
        absolutePath.push_back(virtualPos);

        // 如果不是最后一个格子，添加中间连接点
        if (i < physicalPath.size() - 1) {
            int nextRow = physicalPath[i + 1] / size;
            int nextCol = physicalPath[i + 1] % size;
            int nextVirtualPos = physicalToVirtual(nextRow, nextCol);

            // 计算当前虚拟格子和下一个虚拟格子的行列
            int vRow = (virtualPos / virtualSize);
            int vCol = (virtualPos % virtualSize);
            int vNextRow = (nextVirtualPos / virtualSize);
            int vNextCol = (nextVirtualPos % virtualSize);

            // 添加中间连接点
            int midRow = (vRow + vNextRow) / 2;
            int midCol = (vCol + vNextCol) / 2;
            int midPos = midRow * virtualSize + midCol;
            absolutePath.push_back(midPos);
        }
    }
    //return virtualPath;
}

// !计算A和B的相对路径，存储到成员变量的三个数组里（A相对，B相对）
// !感觉算的不太对，显示有问题，不知道是计算错误还是显示错误
// 直接对成员变量进行修改
void VirtualMaze::calculateRelativePath(const std::vector<int>& physicalPath) {
    // 清空之前的路径，保留
    relativePathA.clear();
    relativePathB.clear();

    // 转换为虚拟路径坐标
    // physicalPath是形参，传递的实参就是path，没问题
    convertToVirtualPath(physicalPath);

    // 计算路径中点
    // ?这个中点后续还要用，用于AB在迷宫中速度的输赢
    int mid = physicalPath.size() / 2;
    bool isOddLength = (physicalPath.size() % 2 == 1);  //判断是1221还是121的情况

    // !计算A相对B的路径（从起点到中点）
    /*
    for (size_t i = 0; i <= mid; i++) {
        // 物理格子的坐标
        int row = physicalPath[i] / size;
        int col = physicalPath[i] % size;
        int vPos = physicalToVirtual(row, col);
        relativePathA.push_back(vPos);

        // 如果不是最后一个格子，添加中间连接点
        if (i < mid) {
            int nextRow = physicalPath[i + 1] / size;
            int nextCol = physicalPath[i + 1] % size;
            int vNextPos = physicalToVirtual(nextRow, nextCol);

            // 计算当前虚拟格子和下一个虚拟格子的行列
            int vRow = (vPos / virtualSize);
            int vCol = (vPos % virtualSize);
            int vNextRow = (vNextPos / virtualSize);
            int vNextCol = (vNextPos % virtualSize);

            // 添加中间连接点
            int midRow = (vRow + vNextRow) / 2;
            int midCol = (vCol + vNextCol) / 2;
            int midPos = midRow * virtualSize + midCol;
            relativePathA.push_back(midPos);
        }
    }

    // 计算B相对A的路径（从终点到中点）
    for (int i = physicalPath.size() - 1; i >= mid; i--) {
        // 物理格子的坐标
        int row = physicalPath[i] / size;
        int col = physicalPath[i] % size;
        int vPos = physicalToVirtual(row, col);
        relativePathB.push_back(vPos);

        // 如果不是最后一个格子，添加中间连接点
        if (i > mid) {
            int nextRow = physicalPath[i - 1] / size;
            int nextCol = physicalPath[i - 1] % size;
            int vNextPos = physicalToVirtual(nextRow, nextCol);

            // 计算当前虚拟格子和下一个虚拟格子的行列
            int vRow = (vPos / virtualSize);
            int vCol = (vPos % virtualSize);
            int vNextRow = (vNextPos / virtualSize);
            int vNextCol = (vNextPos % virtualSize);

            // 添加中间连接点
            int midRow = (vRow + vNextRow) / 2;
            int midCol = (vCol + vNextCol) / 2;
            int midPos = midRow * virtualSize + midCol;
            relativePathB.push_back(midPos);
        }
    }
    */
    int A = 1, B = absolutePath.size() - 2;  //头尾两个指针，分别同时走A和B的路径
    while (A <= B) {  //还没相遇时
        int moveRowA = (absolutePath[A] - absolutePath[A - 1]) / size;  //A绝对移动的行数
        int moveColA = (absolutePath[A] - absolutePath[A - 1]) % size;  //A绝对移动的列数
        int moveRowB = (absolutePath[B] - absolutePath[B + 1]) / size;  //B绝对移动的行数
        int moveColB = (absolutePath[B] - absolutePath[B + 1]) % size;  //B绝对移动的列数

        //物理层相对移动一格，相当于虚拟层相对移动两格
        //以A为参考，B的移动
        std::pair<int, int> releB = std::make_pair(absolutePath[B + 1] / size + moveRowB - moveRowA,
            absolutePath[B + 1] % size + moveColB - moveColA);
        //存的还是物理坐标
        int moveB = releB.first * size + releB.second;
        relativePathB.push_back(moveB);

        //以B为参考，A的移动
        std::pair<int, int> releA = std::make_pair(absolutePath[A - 1] / size + moveRowA - moveRowB,
            absolutePath[A - 1] % size + moveColA - moveColB);
        //存的还是物理坐标
        int moveA = releA.first * size + releA.second;
        relativePathA.push_back(moveA);

        //更新A和B，A往前走，B往后走
        A++;
        B--;
    }

    // 处理A和B的相遇点（根据路径长度的奇偶性）
    /*
    if (!isOddLength) {
        // 偶数长度，A和B在相邻的两个格子上相遇
        int midARow = physicalPath[mid] / size;
        int midACol = physicalPath[mid] % size;
        int midBRow = physicalPath[mid + 1] / size;
        int midBCol = physicalPath[mid + 1] % size;

        int vPosA = physicalToVirtual(midARow, midACol);
        int vPosB = physicalToVirtual(midBRow, midBCol);

        // 计算当前虚拟格子和下一个虚拟格子的行列
        int vRowA = (vPosA / virtualSize);
        int vColA = (vPosA % virtualSize);
        int vRowB = (vPosB / virtualSize);
        int vColB = (vPosB % virtualSize);

        // 添加连接点
        int midRow = (vRowA + vRowB) / 2;
        int midCol = (vColA + vColB) / 2;
        int connectionPoint = midRow * virtualSize + midCol;

        // 确保连接点在两个路径中都存在
        bool inA = false, inB = false;
        for (int point : relativePathA) {
            if (point == connectionPoint) {
                inA = true;
                break;
            }
        }

        for (int point : relativePathB) {
            if (point == connectionPoint) {
                inB = true;
                break;
            }
        }

        if (!inA) relativePathA.push_back(connectionPoint);
        if (!inB) relativePathB.push_back(connectionPoint);
    }
    */
}

// 画绝对路径和相对路径，已修改
// !格子标浩需要结合算AB相对路径的算法修正，现在不对
void VirtualMaze::drawRelativePaths() {
    window.clear(sf::Color::White);
    
    // 清空之前的数字记录
    cellNumbers.clear();
    int counter = 1;  // 数字计数器

    // 绘制最短路径，现在画的是所有路线，已更改
    for (int pos : absolutePath) {
        int row = pos / virtualSize;  //虚拟迷宫格子的横坐标
        int col = pos % virtualSize;  //虚拟迷宫格子的纵坐标
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        cell.setFillColor(sf::Color(200, 200, 200));  // 浅灰色
        window.draw(cell);
        
        // 记录数字和颜色
        cellNumbers[{row, col}].push_back({counter++, sf::Color(100, 100, 100)});  // 深灰色数字
    }

    counter = 1;  // 重置计数器
    // 绘制A的相对路径（蓝色）
    for (int pos : relativePathA) {
        int row = pos / virtualSize;
        int col = pos % virtualSize;
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        cell.setFillColor(sf::Color::Blue);
        window.draw(cell);
        
        // 记录数字和颜色
        cellNumbers[{row, col}].push_back({counter++, sf::Color::Blue});
    }

    counter = 1;  // 重置计数器
    // 绘制B的相对路径（绿色）
    for (int pos : relativePathB) {
        int row = pos / virtualSize;
        int col = pos % virtualSize;
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        cell.setFillColor(sf::Color::Green);
        window.draw(cell);
        
        // 记录数字和颜色
        cellNumbers[{row, col}].push_back({counter++, sf::Color::Green});
    }

    // 绘制起点和终点（红色）
    sf::RectangleShape startCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    startCell.setPosition(CELL_SIZE, CELL_SIZE);
    startCell.setFillColor(sf::Color::Red);
    window.draw(startCell);

    sf::RectangleShape endCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    endCell.setPosition((virtualSize - 2) * CELL_SIZE, (virtualSize - 2) * CELL_SIZE);
    endCell.setFillColor(sf::Color::Red);
    window.draw(endCell);

    // 绘制所有数字
    for (const auto& [pos, numbers] : cellNumbers) {
        std::string text;
        for (size_t i = 0; i < numbers.size(); ++i) {
            text += std::to_string(numbers[i].first);
            if (i < numbers.size() - 1) {
                text += ",";
            }
        }
        
        sf::Text numberText;
        numberText.setFont(font);
        numberText.setString(text);
        numberText.setCharacterSize(12);
        numberText.setFillColor(sf::Color::Black);  // 统一使用黑色显示数字
        
        // 计算文本位置（居中）
        sf::FloatRect textBounds = numberText.getLocalBounds();
        float x = pos.second * CELL_SIZE + (CELL_SIZE - textBounds.width) / 2;
        float y = pos.first * CELL_SIZE + (CELL_SIZE - textBounds.height) / 2;
        numberText.setPosition(x, y);
        
        window.draw(numberText);
    }
}

void VirtualMaze::setPath(const std::vector<int>& path) {
    calculateRelativePath(path);
}

// 窗口显示，添加点击下一关的按钮
void VirtualMaze::display() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            //新增点击下一关的按钮
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (isButtonClicked(mousePos)) {
                        showRelativePaths = !showRelativePaths;
                    }
                }
            }
        }

        drawMaze();
        window.display();
    }
}

// 高亮显示路径，无改动，但是在新旧代码中都不知道是干什么的
/*
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
*/