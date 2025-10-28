#include "VirtualMaze.h"
#include <iostream>
#include <string>   // 为std::string
#include <utility>  // 为std::pair

VirtualMaze::VirtualMaze(int physicalSize) : size(physicalSize), showRelativePaths(false), playerInteractionEnabled(false), isTimerRunning(false), elapsedTime(0.0f), playersHaveMet(false), 
    playerAHasItem(false), playerBHasItem(false), playerAUsedItem(false), playerBUsedItem(false), 
    playerADelayed(false), playerBDelayed(false), delayDuration(2.0f), 
    showAbsolutePathA(false), showAbsolutePathB(false) {
    virtualSize = 2 * size + 1;
    // 初始化虚拟迷宫，全部置为墙（0）
    maze.resize(virtualSize, std::vector<int>(virtualSize, 0));

    // 创建SFML窗口
    window.create(sf::VideoMode(virtualSize * CELL_SIZE, virtualSize * CELL_SIZE),
        "Virtual Maze", sf::Style::Close);
    window.setFramerateLimit(60);

    // 初始化玩家位置和颜色
    playerPositionA = { 1, 1 };  // 起点位置（虚拟坐标）
    lastPlayerPositionA = { 1, 1 };  // 初始时与当前位置相同
    playerColorA = sf::Color(255, 165, 0);  // 橙色
    playerPositionB = { virtualSize - 2, virtualSize - 2 };  // 终点位置（虚拟坐标）
    lastPlayerPositionB = { virtualSize - 2, virtualSize - 2 };  // 初始时与当前位置相同
    playerColorB = sf::Color(0, 0, 255);  // 蓝色

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
    
    // 初始化计时器文本
    timerText.setFont(font);
    timerText.setString("00:00");
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::Black);
    timerText.setPosition(
        (virtualSize * CELL_SIZE - timerText.getLocalBounds().width) / 2, 
        10.0f  // 位于界面上方10像素处
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
//画的只是迷宫地图，没有玩家
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
    
    int mid = absolutePath.size() / 2;

    // 前一半
    absolutePathA = std::vector<int>(absolutePath.begin(), absolutePath.begin() + mid);
    // 后一半
    absolutePathB = std::vector<int>(absolutePath.begin() + mid, absolutePath.end());

}

// !计算A和B的相对路径，存储到成员变量的三个数组里（A相对，B相对）
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
    int moveB = virtualSize * (virtualSize - 1) - 2;  //B的初始位置(虚拟坐标的序号
    int moveA = virtualSize + 1;
    //std::cout << moveA << std::endl;
    while (A <= B) {  //还没相遇时
        //std::cout << absolutePath[A] << " " << absolutePath[A - 1] << std::endl;
        //std::cout << absolutePath[B] << " " << absolutePath[B + 1] << std::endl;
        int moveRowA = (absolutePath[A] - absolutePath[A - 1]) / virtualSize;  //A绝对移动的行数
        int moveColA = (absolutePath[A] - absolutePath[A - 1]) % virtualSize;  //A绝对移动的列数
        int moveRowB = (absolutePath[B] - absolutePath[B + 1]) / virtualSize;  //B绝对移动的行数
        int moveColB = (absolutePath[B] - absolutePath[B + 1]) % virtualSize;  //B绝对移动的列数

        //物理层相对移动一格，相当于虚拟层相对移动两格,而且移动是相对上一个相对格子，而不是绝对格子
        //以A为参考，B的移动
         //初始的相对移动的格子是相对于起点，后面的都是相对于前一个相对格子
        std::pair<int, int> releB = std::make_pair(moveB / virtualSize + moveRowB - moveRowA,
            moveB % virtualSize + moveColB - moveColA);
        //存的还是物理坐标
        moveB = releB.first * virtualSize + releB.second;
        relativePathB.push_back(moveB);
        

        //以B为参考，A的移动
        std::pair<int, int> releA = std::make_pair(moveA / virtualSize + moveRowA - moveRowB,
            moveA % virtualSize + moveColA - moveColB);
        //存的还是物理坐标
        moveA = releA.first * virtualSize + releA.second;
        relativePathA.push_back(moveA);
        //std::cout << "relative" << A << " " << moveA << std::endl;
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
/*
std::vector<int> VirtualMaze::absolutePathA(){
    int mid = absolutePath.size() / 2;
    // 前一半
    std::vector<int> absolutePathA(absolutePath.begin(), absolutePath.begin() + mid);
    return absolutePathA;
}

std::vector<int> VirtualMaze::absolutePathB(){
    int mid = absolutePath.size() / 2;
    // 后一半
    std::vector<int> absolutePathB(absolutePath.begin() + mid, absolutePath.end());
    return absolutePathB;
}   
*/    
// 画绝对路径和相对路径，已修改
void VirtualMaze::drawRelativePaths() {
    window.clear(sf::Color::White);

    // 清空之前的数字记录
    cellNumbers.clear();
    int counter = absolutePathB.size();  // 数字计数器

    // 绘制B的绝对路径，A玩
    for (int pos : absolutePathB) {
        int row = pos / virtualSize;  //虚拟迷宫格子的横坐标
        int col = pos % virtualSize;  //虚拟迷宫格子的纵坐标
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        cell.setFillColor(sf::Color(200, 200, 200));  // 浅灰色
        window.draw(cell);

        // 记录数字和颜色
        cellNumbers[{row, col}].push_back({ counter--, sf::Color(100, 100, 100) });  // 深灰色数字
    }

    counter = 2;  // 重置计数器
    // 绘制A的相对路径
    for (int pos : relativePathA) {
        int row = pos / virtualSize;
        int col = pos % virtualSize;
        sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        pathCell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        pathCell.setFillColor(sf::Color(100, 100, 255, 128)); // 半透明蓝色
        window.draw(pathCell);
        // 记录数字和颜色
        cellNumbers[{row, col}].push_back({ counter++, sf::Color(100, 100, 100) });  // 深灰色数字
    }
    /*
    counter = 1;  // 重置计数器
    // 绘制B的相对路径
    for (int pos : relativePathB) {
        int row = pos / virtualSize;
        int col = pos % virtualSize;
        sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        pathCell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
        pathCell.setFillColor(sf::Color(255, 165, 0, 128)); // 半透明橙色
        window.draw(pathCell);
    }
    */

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

// 窗口显示，添加点击下一关的按钮和玩家交互
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
            else if (playerInteractionEnabled && event.type == sf::Event::KeyPressed) {
                handleKeyPressA(event.key.code);
                handleKeyPressB(event.key.code);

                // 检查是否到达终点
                if (playerPositionA.first == virtualSize - 2 && playerPositionA.second == virtualSize - 2) {
                    std::cout << "you finish" << std::endl;
                    // 更新玩家格子颜色
                    updatePlayerCellA();
                }
                if (playerPositionB.first == virtualSize - 2 && playerPositionB.second == virtualSize - 2) {
                    std::cout << "You finish" << std::endl;
                    // 更新玩家格子颜色
                    updatePlayerCellB();
                }
                //碰撞检测
                if (playerPositionA == playerPositionB) {
                    std::cout << "We meet" << std::endl;
                    //检查谁跑得快
                    int checkmid = absolutePath.size() / 2;  //找到绝对路径中点的序号
                    int pointer = 0;
                    while (absolutePath[pointer] != playerPositionA.first * virtualSize + playerPositionA.second) {
                        pointer++;
                    }
                    if (pointer < checkmid) {
                        //A跑得快
                        mazefaster = 0;
                    }
                    else {
                        //B跑得快
                        mazefaster = 1;
                    }
                    std::cout << mazefaster << std::endl;
                    // 更新玩家格子颜色
                    updatePlayerCellA();
                    updatePlayerCellB();
                }
            }
        }

        //迷宫是一定要画的
        drawMaze();

        if (playerInteractionEnabled) {            
            drawPlayerA();
            drawPlayerB();
        }
        

        window.display();
    }
}

// 显示单人模式的地图
void VirtualMaze::displaySingle(){
    // 重置计时器
    gameClock.restart();
    isTimerRunning = true;
    elapsedTime = 0.0f;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            else if (playerInteractionEnabled && event.type == sf::Event::KeyPressed) {
                // !要改，不受迷宫路线限制，在窗口内移动就行
                handleKeyPressA_free(event.key.code);

                // !检查是否到达终点，但是这个终点值需要重新计算，根据生成迷宫的最短路径的中点
                if (playerPositionA.first == absolutePathA.back() / virtualSize 
                    && playerPositionA.second == absolutePathA.back() % virtualSize) {
                    std::cout << "Congratulations! You win!" << std::endl;
                    // !停止计时器
                    isTimerRunning = false;
                    // 更新玩家格子颜色
                    updatePlayerCellA();
                }
            }
        }

        // 更新计时器
        if (isTimerRunning) {
            elapsedTime = gameClock.getElapsedTime().asSeconds();
            
            // 格式化时间为 MM:SS
            int minutes = static_cast<int>(elapsedTime) / 60;
            int seconds = static_cast<int>(elapsedTime) % 60;
            
            // 更新计时器文本
            std::string timeString = 
                (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + 
                (seconds < 10 ? "0" : "") + std::to_string(seconds);
            timerText.setString(timeString);
            
            // 更新文本位置（保持居中）
            sf::FloatRect textBounds = timerText.getLocalBounds();
            timerText.setPosition(
                (virtualSize * CELL_SIZE - textBounds.width) / 2,
                10.0f
            );
        }

        //迷宫是一定要画的
        drawSingleMap();

        if (playerInteractionEnabled) {
            // 如果启用了玩家交互，绘制迷宫并在上面绘制玩家
            //drawMaze();
            drawPlayerA();
        }
        
        
        // 绘制计时器
        window.draw(timerText);

        window.display();
    }
}

//画的只是单人模式的地图，没有玩家，
void VirtualMaze::drawSingleMap() {
    window.clear(sf::Color::White);
    showRelativePaths = true;
    //单人模式的本质就是直接只画相对路径，不需要走迷宫的步骤
    drawRelativePaths();    
}


// ----------------- 修改分屏功能代码 -----------------

void VirtualMaze::displayMultiPlayer() {
    // 计算迷宫在像素下的尺寸
    unsigned int mazePixelWidth = virtualSize * CELL_SIZE;
    unsigned int mazePixelHeight = virtualSize * CELL_SIZE;

    // 创建窗口
    window.create(sf::VideoMode(mazePixelWidth, mazePixelHeight),
        "Virtual Maze - Multi Player", sf::Style::Close);
    window.setFramerateLimit(60);

    // 新建两个局部的 SFML 视图对象，用于分屏显示
    sf::View viewA, viewB;
    
    // 创建全局视图（用于参考）
    sf::View defaultView = window.getDefaultView();

    // 设置每个视图的尺寸，保持原有宽高比
    viewA.setSize(mazePixelWidth / 2.f, mazePixelHeight / 2.f);
    viewB.setSize(mazePixelWidth / 2.f, mazePixelHeight / 2.f);

    // 设置视口，保持左右各占一半，并且不会拉伸变形
    viewA.setViewport(sf::FloatRect(0.f, 0.f, 0.5f, 1.f));
    viewB.setViewport(sf::FloatRect(0.5f, 0.f, 0.5f, 1.f));

    // 重置玩家位置到起点和终点
    playerPositionA = { 1, 1 };  // 玩家A位于起点
    playerPositionB = { virtualSize - 2, virtualSize - 2 };  // 玩家B位于终点
    
    // 启用玩家交互
    playerInteractionEnabled = true;
    
    // 重置玩家相遇状态
    playersHaveMet = false;

    // "Next Round"悬浮提示文本
    sf::Text nextRoundText;
    nextRoundText.setFont(font);
    nextRoundText.setString("Next Round");
    nextRoundText.setCharacterSize(40);
    nextRoundText.setFillColor(sf::Color::Red);
    
    // 居中文本
    sf::FloatRect textBounds = nextRoundText.getLocalBounds();
    nextRoundText.setPosition(
        (mazePixelWidth - textBounds.width) / 2,
        (mazePixelHeight - textBounds.height) / 2
    );
    
    // 初始化视图中心为玩家位置
    viewA.setCenter(playerPositionA.second * CELL_SIZE + CELL_SIZE / 2.f, 
                    playerPositionA.first * CELL_SIZE + CELL_SIZE / 2.f);
    viewB.setCenter(playerPositionB.second * CELL_SIZE + CELL_SIZE / 2.f, 
                    playerPositionB.first * CELL_SIZE + CELL_SIZE / 2.f);

    sf::Clock meetingClock; // 用于计时玩家相遇后的等待时间
    bool showingNextRound = false; // 是否正在显示"Next Round"
    float meetingDisplayTime = 0; // 玩家相遇后的显示时间

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed && !showingNextRound) {
                // 调用现有的键盘处理函数
                handleKeyPressA(event.key.code);
                handleKeyPressB(event.key.code);
                
                // 打印玩家位置（调试用）
                std::cout << "Player A: (" << playerPositionA.first << "," << playerPositionA.second << ")" << std::endl;
                std::cout << "Player B: (" << playerPositionB.first << "," << playerPositionB.second << ")" << std::endl;
                
                // 检查玩家是否相遇
                if (playerPositionA.first == playerPositionB.first && 
                    playerPositionA.second == playerPositionB.second && 
                    !playersHaveMet) {
                    std::cout << "Players meet!" << std::endl;

                    //判断谁跑得快
                    int checkmid = absolutePath.size() / 2;  //找到绝对路径中点的序号
                    int pointer = 0;
                    while (absolutePath[pointer] != playerPositionA.first * virtualSize + playerPositionA.second) {
                        pointer++;
                    }
                    if (pointer < checkmid) {
                        //A跑得快
                        mazefaster = 0;
                        playerAHasItem = true;  // A获得道具
                        playerBHasItem = false; // B没有道具
                    }
                    else {
                        //B跑得快
                        mazefaster = 1;
                        playerBHasItem = true;  // B获得道具
                        playerAHasItem = false; // A没有道具
                    }
                    std::cout << mazefaster << std::endl;

                    playersHaveMet = true;
                    showingNextRound = true;
                    meetingClock.restart(); // 开始计时
                }
            }
        }

        // 更新视图中心，跟随玩家移动
        viewA.setCenter(playerPositionA.second * CELL_SIZE + CELL_SIZE / 2.f, 
                        playerPositionA.first * CELL_SIZE + CELL_SIZE / 2.f);
        viewB.setCenter(playerPositionB.second * CELL_SIZE + CELL_SIZE / 2.f, 
                        playerPositionB.first * CELL_SIZE + CELL_SIZE / 2.f);
        
        window.clear(sf::Color::White);
        
        // 绘制左侧视图（玩家A的区域）
        window.setView(viewA);
        // 绘制迷宫内容
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
        
        // 绘制玩家A
        sf::RectangleShape playerA(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerA.setFillColor(playerColorA);
        playerA.setPosition(
            playerPositionA.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionA.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerA);
        
        // 在A的视图中绘制玩家B
        sf::RectangleShape playerBinViewA(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerBinViewA.setFillColor(playerColorB);
        playerBinViewA.setPosition(
            playerPositionB.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionB.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerBinViewA);
        
        // 绘制右侧视图（玩家B的区域）
        window.setView(viewB);
        // 绘制迷宫内容
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
        
        // 绘制玩家B
        sf::RectangleShape playerB(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerB.setFillColor(playerColorB);
        playerB.setPosition(
            playerPositionB.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionB.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerB);
        
        // 在B的视图中绘制玩家A
        sf::RectangleShape playerAinViewB(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerAinViewB.setFillColor(playerColorA);
        playerAinViewB.setPosition(
            playerPositionA.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionA.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerAinViewB);
        
        // 切换回默认视图，绘制分隔线和Next Round文本
        window.setView(defaultView);
        sf::RectangleShape separator(sf::Vector2f(4, mazePixelHeight));
        separator.setFillColor(sf::Color::Black);
        separator.setPosition(mazePixelWidth * 0.5f - 2.f, 0.f);
        window.draw(separator);
        
        // 如果玩家相遇，显示Next Round并准备过渡到第二阶段
        if (showingNextRound) {
            window.draw(nextRoundText);
            meetingDisplayTime = meetingClock.getElapsedTime().asSeconds();
            
            // 如果显示了2秒，进入第二阶段
            if (meetingDisplayTime >= 2.0f) {
                // 进入相对路径阶段
                std::cout << "coming to relative path finding..." << std::endl;
                
                // 转到相对路径阶段
                displayRelativeMaze();
                return;
            }
        }
        
        window.display();
    }
}

// 新增：实现相对路径阶段的游戏逻辑
void VirtualMaze::displayRelativeMaze() {
    // 计算迷宫在像素下的尺寸
    unsigned int mazePixelWidth = virtualSize * CELL_SIZE;
    unsigned int mazePixelHeight = virtualSize * CELL_SIZE;

    // 创建双人相对路径窗口
    window.create(sf::VideoMode(mazePixelWidth, mazePixelHeight),
        "Dual Relative Path Challenge", sf::Style::Close);
    window.setFramerateLimit(60);

    // 创建左右视图
    sf::View leftView, rightView, uiView;
    
    // 设置全局UI视图（用于绘制按钮）
    uiView = window.getDefaultView();
    
    // 设置左右游戏视图
    leftView.setViewport(sf::FloatRect(0.0f, 0.0f, 0.5f, 1.0f));
    rightView.setViewport(sf::FloatRect(0.5f, 0.0f, 0.5f, 1.0f));
    
    // 设置视图大小
    leftView.setSize(static_cast<float>(mazePixelWidth), static_cast<float>(mazePixelHeight));
    rightView.setSize(static_cast<float>(mazePixelWidth), static_cast<float>(mazePixelHeight));
    
    // 设置视图中心
    leftView.setCenter(static_cast<float>(mazePixelWidth)/2, static_cast<float>(mazePixelHeight)/2);
    rightView.setCenter(static_cast<float>(mazePixelWidth)/2, static_cast<float>(mazePixelHeight)/2);
    
    // 记录获胜状态
    bool playerAWon = false;
    bool playerBWon = false;
    
    // 重置玩家位置到起点
    playerPositionA = {virtualSize - 2, virtualSize - 2}; 
    playerPositionB = {1, 1};
    
    // 初始化道具和绝对路径按钮
    initializeItemButtons();
    
    // 重置道具使用状态
    playerAUsedItem = false;
    playerBUsedItem = false;
    playerADelayed = false;
    playerBDelayed = false;
    showAbsolutePathA = false;
    showAbsolutePathB = false;
    
    // 游戏主循环
    while (window.isOpen()) {
        // 处理事件
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            
            // 处理鼠标点击事件
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                // 处理按钮点击 - 简化的按钮点击逻辑
                
                // 获取全局按钮边界
                sf::FloatRect itemButtonABounds = itemButtonA.getGlobalBounds();
                sf::FloatRect absPathButtonABounds = absPathButtonA.getGlobalBounds();
                sf::FloatRect itemButtonBBounds = itemButtonB.getGlobalBounds();
                sf::FloatRect absPathButtonBBounds = absPathButtonB.getGlobalBounds();
                
                // 检查A玩家道具按钮点击
                if (itemButtonABounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    if (playerAHasItem && !playerAUsedItem) {
                        playerAUsedItem = true; 
                        playerBDelayed = true;
                        itemEffectClock.restart();
                        itemButtonA.setFillColor(sf::Color(100, 100, 100)); // 变灰表示已使用
                    }
                }
                
                // 检查A玩家路径显示按钮点击
                if (absPathButtonABounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    showAbsolutePathA = !showAbsolutePathA;
                    
                    if (showAbsolutePathA) {
                        absPathButtonA.setFillColor(sf::Color(50, 50, 200)); // 深蓝色
                        absPathTextA.setString("Hide Path");
                    } else {
                        absPathButtonA.setFillColor(sf::Color(100, 100, 255)); // 浅蓝色
                        absPathTextA.setString("Show Path");
                    }
                    
                    // 更新文本位置以保持居中
                    sf::FloatRect textBounds = absPathTextA.getLocalBounds();
                    absPathTextA.setPosition(
                        absPathButtonA.getPosition().x + (absPathButtonA.getSize().x - textBounds.width) / 2.0f,
                        absPathButtonA.getPosition().y + (absPathButtonA.getSize().y - textBounds.height) / 2.0f - 2.0f
                    );
                }
                
                // 检查B玩家道具按钮点击
                if (itemButtonBBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    if (playerBHasItem && !playerBUsedItem) {
                        playerBUsedItem = true;
                        playerADelayed = true;
                        itemEffectClock.restart();
                        itemButtonB.setFillColor(sf::Color(100, 100, 100)); // 变灰表示已使用
                    }
                }
                
                // 检查B玩家路径显示按钮点击
                if (absPathButtonBBounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    showAbsolutePathB = !showAbsolutePathB;
                    
                    if (showAbsolutePathB) {
                        absPathButtonB.setFillColor(sf::Color(50, 50, 200)); // 深蓝色
                        absPathTextB.setString("Hide Path");
                    } else {
                        absPathButtonB.setFillColor(sf::Color(100, 100, 255)); // 浅蓝色
                        absPathTextB.setString("Show Path");
                    }
                    
                    // 更新文本位置以保持居中
                    sf::FloatRect textBounds = absPathTextB.getLocalBounds();
                    absPathTextB.setPosition(
                        absPathButtonB.getPosition().x + (absPathButtonB.getSize().x - textBounds.width) / 2.0f,
                        absPathButtonB.getPosition().y + (absPathButtonB.getSize().y - textBounds.height) / 2.0f - 2.0f
                    );
                }
            }
            
            // 处理按键 - 玩家移动
            if (event.type == sf::Event::KeyPressed) {
                // 检查是否有玩家被延迟
                if (playerADelayed) {
                    float delayElapsed = itemEffectClock.getElapsedTime().asSeconds();
                    if (delayElapsed >= delayDuration) {
                        playerADelayed = false; // 延迟效果结束
                    }
                }
                
                if (playerBDelayed) {
                    float delayElapsed = itemEffectClock.getElapsedTime().asSeconds();
                    if (delayElapsed >= delayDuration) {
                        playerBDelayed = false; // 延迟效果结束
                    }
                }
                
                // 玩家A移动
                if (!playerADelayed && (event.key.code == sf::Keyboard::Up || 
                    event.key.code == sf::Keyboard::Down || 
                    event.key.code == sf::Keyboard::Left || 
                    event.key.code == sf::Keyboard::Right)) {
                    
                    lastPlayerPositionA = playerPositionA;
                    std::pair<int, int> newPos = playerPositionA;
                    
                    if (event.key.code == sf::Keyboard::Up) newPos.first--;
                    else if (event.key.code == sf::Keyboard::Down) newPos.first++;
                    else if (event.key.code == sf::Keyboard::Left) newPos.second--;
                    else if (event.key.code == sf::Keyboard::Right) newPos.second++;
                    
                    if (canMoveTo(newPos.first, newPos.second)) {
                        playerPositionA = newPos;
                        
                        // 检查是否到达终点
                        if (playerPositionA.first == 1 && playerPositionA.second == 1) {
                            playerAWon = true;
                        }
                    }
                }
                
                // 玩家B移动
                if (!playerBDelayed && (event.key.code == sf::Keyboard::W || 
                    event.key.code == sf::Keyboard::S || 
                    event.key.code == sf::Keyboard::A || 
                    event.key.code == sf::Keyboard::D)) {
                    
                    lastPlayerPositionB = playerPositionB;
                    std::pair<int, int> newPos = playerPositionB;
                    
                    if (event.key.code == sf::Keyboard::W) newPos.first--;
                    else if (event.key.code == sf::Keyboard::S) newPos.first++;
                    else if (event.key.code == sf::Keyboard::A) newPos.second--;
                    else if (event.key.code == sf::Keyboard::D) newPos.second++;
                    
                    if (canMoveTo(newPos.first, newPos.second)) {
                        playerPositionB = newPos;
                        
                        // 检查是否到达终点
                        if (playerPositionB.first == 1 && playerPositionB.second == 1) {
                            playerBWon = true;
                        }
                    }
                }
            }
        }
        
        // 清空窗口
        window.clear(sf::Color::White);
        
        // ===== 绘制左侧视图 =====
        window.setView(leftView);
        
        // 创建延迟状态提示文本
        sf::Text delayStatusTextA;
        if (playerADelayed) {
            delayStatusTextA.setFont(font);
            delayStatusTextA.setCharacterSize(24);
            delayStatusTextA.setPosition(10.0f, 110.0f);
            
            float remainingTime = delayDuration - itemEffectClock.getElapsedTime().asSeconds();
            if (remainingTime < 0) remainingTime = 0;
            
            std::string delayText = "Delayed: " + std::to_string(static_cast<int>(remainingTime + 0.5f)) + "s";
            delayStatusTextA.setString(delayText);
            delayStatusTextA.setFillColor(sf::Color::Red);
        }

        // 绘制B的相对路径（左侧视图）
        cellNumbers.clear();
        int counter = 1;
        for (int pos : relativePathB) {
            int row = pos / virtualSize;
            int col = pos % virtualSize;
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
            cell.setFillColor(sf::Color::Green);
            window.draw(cell);

            // 记录数字
            cellNumbers[{row, col}].push_back({counter++, sf::Color::Green});
        }
        
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
            numberText.setFillColor(sf::Color::Black);

            sf::FloatRect textBounds = numberText.getLocalBounds();
            float x = pos.second * CELL_SIZE + (CELL_SIZE - textBounds.width) / 2;
            float y = pos.first * CELL_SIZE + (CELL_SIZE - textBounds.height) / 2;
            numberText.setPosition(x, y);

            window.draw(numberText);
        }
        
        // 如果玩家A选择显示绝对路径，则绘制
        if (showAbsolutePathA) {
            for (int pos : absolutePath) {
                int row = pos / virtualSize;
                int col = pos % virtualSize;
                sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                pathCell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
                pathCell.setFillColor(sf::Color(150, 150, 150, 100)); // 半透明灰色
                window.draw(pathCell);
            }
        }

        // 绘制起点和终点
        sf::RectangleShape startCellA(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        startCellA.setPosition(1 * CELL_SIZE, 1 * CELL_SIZE); 
        startCellA.setFillColor(sf::Color::Green);
        window.draw(startCellA);
        
        sf::RectangleShape endCellA(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        endCellA.setPosition((virtualSize - 2) * CELL_SIZE, (virtualSize - 2) * CELL_SIZE);
        endCellA.setFillColor(sf::Color::Red);
        window.draw(endCellA);
        
        // 绘制玩家A
        sf::RectangleShape playerAShape(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerAShape.setFillColor(playerColorA);
        playerAShape.setPosition(
            playerPositionA.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionA.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerAShape);
        
        // 显示延迟状态
        if (playerADelayed) {
            window.draw(delayStatusTextA);
        }
        
        // ===== 绘制右侧视图 =====
        window.setView(rightView);
        
        // 创建延迟状态提示文本
        sf::Text delayStatusTextB;
        if (playerBDelayed) {
            delayStatusTextB.setFont(font);
            delayStatusTextB.setCharacterSize(24);
            delayStatusTextB.setPosition(10.0f, 110.0f);
            
            float remainingTime = delayDuration - itemEffectClock.getElapsedTime().asSeconds();
            if (remainingTime < 0) remainingTime = 0;
            
            std::string delayText = "Delayed: " + std::to_string(static_cast<int>(remainingTime + 0.5f)) + "s";
            delayStatusTextB.setString(delayText);
            delayStatusTextB.setFillColor(sf::Color::Red);
        }

        // 绘制A的相对路径（右侧视图）
        cellNumbers.clear();
        counter = 1;
        for (int pos : relativePathA) {
            int row = pos / virtualSize;
            int col = pos % virtualSize;
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
            cell.setFillColor(sf::Color::Blue);
            window.draw(cell);

            // 记录数字
            cellNumbers[{row, col}].push_back({counter++, sf::Color::Blue});
        }
        
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
            numberText.setFillColor(sf::Color::Black);

            sf::FloatRect textBounds = numberText.getLocalBounds();
            float x = pos.second * CELL_SIZE + (CELL_SIZE - textBounds.width) / 2;
            float y = pos.first * CELL_SIZE + (CELL_SIZE - textBounds.height) / 2;
            numberText.setPosition(x, y);

            window.draw(numberText);
        }
        
        // 如果玩家B选择显示绝对路径，则绘制
        if (showAbsolutePathB) {
            for (int pos : absolutePath) {
                int row = pos / virtualSize;
                int col = pos % virtualSize;
                sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                pathCell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
                pathCell.setFillColor(sf::Color(150, 150, 150, 100)); // 半透明灰色
                window.draw(pathCell);
            }
        }
        
        // 绘制起点和终点
        sf::RectangleShape startCellB(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        startCellB.setPosition(1 * CELL_SIZE, 1 * CELL_SIZE);
        startCellB.setFillColor(sf::Color::Green);
        window.draw(startCellB);
        
        sf::RectangleShape endCellB(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        endCellB.setPosition((virtualSize - 2) * CELL_SIZE, (virtualSize - 2) * CELL_SIZE);
        endCellB.setFillColor(sf::Color::Red);
        window.draw(endCellB);
        
        // 绘制玩家B
        sf::RectangleShape playerBShape(sf::Vector2f(CELL_SIZE * 0.8f, CELL_SIZE * 0.8f));
        playerBShape.setFillColor(playerColorB);
        playerBShape.setPosition(
            playerPositionB.second * CELL_SIZE + CELL_SIZE * 0.1f,
            playerPositionB.first * CELL_SIZE + CELL_SIZE * 0.1f
        );
        window.draw(playerBShape);
        
        // 显示延迟状态
        if (playerBDelayed) {
            window.draw(delayStatusTextB);
        }
        
        // ===== 绘制UI元素 =====
        // 使用默认视图绘制UI元素（按钮和分隔线）
        window.setView(uiView);
        
        // 绘制分隔线
        sf::RectangleShape separator(sf::Vector2f(4, mazePixelHeight));
        separator.setFillColor(sf::Color::Black);
        separator.setPosition(mazePixelWidth * 0.5f - 2.f, 0.f);
        window.draw(separator);
        
        // 绘制按钮
        window.draw(itemButtonA);
        window.draw(itemTextA);
        window.draw(absPathButtonA);
        window.draw(absPathTextA);
        window.draw(itemButtonB);
        window.draw(itemTextB);
        window.draw(absPathButtonB);
        window.draw(absPathTextB);
        
        // 显示胜利信息
        if (playerAWon || playerBWon) {
            sf::Text winText;
            winText.setFont(font);
            if (playerAWon && playerBWon) {
                winText.setString("It's a tie!");
            } else if (playerAWon) {
                winText.setString("Player A wins!");
            } else {
                winText.setString("Player B wins!");
            }
            winText.setCharacterSize(40);
            winText.setFillColor(sf::Color::Red);
            
            sf::FloatRect textBounds = winText.getLocalBounds();
            winText.setPosition(
                (mazePixelWidth - textBounds.width) / 2.f,
                (mazePixelHeight - textBounds.height) / 2.f
            );
            
            window.draw(winText);
        }
        
        // 更新显示
        window.display();
    }
}

// 初始化道具按钮
void VirtualMaze::initializeItemButtons() {
    // 按钮尺寸
    const float BUTTON_WIDTH = 120.0f;
    const float BUTTON_HEIGHT = 30.0f;
    const float TOP_MARGIN = 10.0f;
    const float SPACING = 10.0f;
    
    // ------- 左侧视图按钮 -------
    
    // 左侧视图的绘制区域宽度 (半个窗口宽度)
    float leftWidth = static_cast<float>(virtualSize * CELL_SIZE) / 2.0f;
    
    // 玩家A的道具按钮 - 左侧视图左上角
    itemButtonA.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    itemButtonA.setPosition(10.0f, TOP_MARGIN);
    
    // 根据谁跑得快来设置按钮颜色和状态
    if (mazefaster == 0) { // A赢了
        itemButtonA.setFillColor(sf::Color(0, 200, 0)); // 绿色
        playerAHasItem = true;
    } else {
        itemButtonA.setFillColor(sf::Color(150, 150, 150)); // 灰色
        playerAHasItem = false;
    }
    
    // 玩家A的道具文本
    itemTextA.setFont(font);
    itemTextA.setString("Delay Item");
    itemTextA.setCharacterSize(16);
    itemTextA.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect textBoundsA = itemTextA.getLocalBounds();
    itemTextA.setPosition(
        itemButtonA.getPosition().x + (BUTTON_WIDTH - textBoundsA.width) / 2.0f,
        itemButtonA.getPosition().y + (BUTTON_HEIGHT - textBoundsA.height) / 2.0f - 2.0f
    );
    
    // 玩家A的绝对路径按钮 - 右侧紧跟道具按钮
    absPathButtonA.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    absPathButtonA.setPosition(10.0f + BUTTON_WIDTH + SPACING, TOP_MARGIN);
    absPathButtonA.setFillColor(sf::Color(100, 100, 255)); // 蓝色
    
    // 玩家A的绝对路径文本
    absPathTextA.setFont(font);
    absPathTextA.setString("Show Path");
    absPathTextA.setCharacterSize(16);
    absPathTextA.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect absTextBoundsA = absPathTextA.getLocalBounds();
    absPathTextA.setPosition(
        absPathButtonA.getPosition().x + (BUTTON_WIDTH - absTextBoundsA.width) / 2.0f,
        absPathButtonA.getPosition().y + (BUTTON_HEIGHT - absTextBoundsA.height) / 2.0f - 2.0f
    );

    // ------- 右侧视图按钮 -------
    
    // 右侧视图的起始X坐标 (半个窗口宽度)
    float rightStartX = leftWidth;
    
    // 玩家B的道具按钮 - 右侧视图左上角
    itemButtonB.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    itemButtonB.setPosition(rightStartX + 10.0f, TOP_MARGIN);
    
    // 根据谁跑得快来设置按钮颜色和状态
    if (mazefaster == 1) { // B赢了
        itemButtonB.setFillColor(sf::Color(0, 200, 0)); // 绿色
        playerBHasItem = true;
    } else {
        itemButtonB.setFillColor(sf::Color(150, 150, 150)); // 灰色
        playerBHasItem = false;
    }
    
    // 玩家B的道具文本
    itemTextB.setFont(font);
    itemTextB.setString("Delay Item");
    itemTextB.setCharacterSize(16);
    itemTextB.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect textBoundsB = itemTextB.getLocalBounds();
    itemTextB.setPosition(
        itemButtonB.getPosition().x + (BUTTON_WIDTH - textBoundsB.width) / 2.0f,
        itemButtonB.getPosition().y + (BUTTON_HEIGHT - textBoundsB.height) / 2.0f - 2.0f
    );
    
    // 玩家B的绝对路径按钮 - 右侧紧跟道具按钮
    absPathButtonB.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    absPathButtonB.setPosition(rightStartX + 10.0f + BUTTON_WIDTH + SPACING, TOP_MARGIN);
    absPathButtonB.setFillColor(sf::Color(100, 100, 255)); // 蓝色
    
    // 玩家B的绝对路径文本
    absPathTextB.setFont(font);
    absPathTextB.setString("Show Path");
    absPathTextB.setCharacterSize(16);
    absPathTextB.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect absTextBoundsB = absPathTextB.getLocalBounds();
    absPathTextB.setPosition(
        absPathButtonB.getPosition().x + (BUTTON_WIDTH - absTextBoundsB.width) / 2.0f,
        absPathButtonB.getPosition().y + (BUTTON_HEIGHT - absTextBoundsB.height) / 2.0f - 2.0f
    );
}

// 初始化绝对路径按钮
void VirtualMaze::initializeAbsPathButtons() {
    // 按钮尺寸
    const float BUTTON_WIDTH = 120.0f;
    const float BUTTON_HEIGHT = 30.0f;
    const float TOP_MARGIN = 10.0f;
    const float BUTTON_SPACING = 10.0f; // 按钮之间的间距
    
    // 初始化玩家A的绝对路径按钮（左侧屏幕）
    absPathButtonA.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    absPathButtonA.setPosition(10.0f + BUTTON_WIDTH + BUTTON_SPACING, TOP_MARGIN); // 紧跟道具按钮右侧
    absPathButtonA.setFillColor(sf::Color(100, 100, 255)); // 蓝色
    
    // 初始化玩家A的绝对路径文本
    absPathTextA.setFont(font);
    absPathTextA.setString("Show Path");
    absPathTextA.setCharacterSize(16);
    absPathTextA.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect textBoundsA = absPathTextA.getLocalBounds();
    absPathTextA.setPosition(
        absPathButtonA.getPosition().x + (BUTTON_WIDTH - textBoundsA.width) / 2.0f,
        absPathButtonA.getPosition().y + (BUTTON_HEIGHT - textBoundsA.height) / 2.0f - 2.0f
    );
    
    // 初始化玩家B的绝对路径按钮（右侧屏幕）
    absPathButtonB.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    float rightSideX = (virtualSize * CELL_SIZE) / 2.0f + 10.0f; // 右半屏起始位置
    absPathButtonB.setPosition(rightSideX + BUTTON_WIDTH + BUTTON_SPACING, TOP_MARGIN); // 紧跟B的道具按钮右侧
    absPathButtonB.setFillColor(sf::Color(100, 100, 255)); // 蓝色
    
    // 初始化玩家B的绝对路径文本
    absPathTextB.setFont(font);
    absPathTextB.setString("Show Path");
    absPathTextB.setCharacterSize(16);
    absPathTextB.setFillColor(sf::Color::Black);
    
    // 居中文本
    sf::FloatRect textBoundsB = absPathTextB.getLocalBounds();
    absPathTextB.setPosition(
        absPathButtonB.getPosition().x + (BUTTON_WIDTH - textBoundsB.width) / 2.0f,
        absPathButtonB.getPosition().y + (BUTTON_HEIGHT - textBoundsB.height) / 2.0f - 2.0f
    );
}

// 检查A道具按钮是否被点击
bool VirtualMaze::isItemButtonAClicked(sf::Vector2i mousePos) const {
    // 只处理左半屏的点击
    if (mousePos.x < window.getSize().x / 2) {
        sf::FloatRect buttonBounds = itemButtonA.getGlobalBounds();
        return buttonBounds.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
    }
    return false;
}

// 检查B道具按钮是否被点击
bool VirtualMaze::isItemButtonBClicked(sf::Vector2i mousePos) const {
    // 只处理右半屏的点击
    if (mousePos.x >= window.getSize().x / 2) {
        // 右侧视图的按钮坐标是相对于右侧视图左上角的，需要将鼠标坐标进行转换
        float adjustedX = static_cast<float>(mousePos.x - window.getSize().x / 2);
        float adjustedY = static_cast<float>(mousePos.y);
        
        // 使用转换后的坐标检查点击
        sf::Vector2f adjustedPos(adjustedX, adjustedY);
        sf::FloatRect buttonBounds = itemButtonB.getGlobalBounds();
        
        // 调试输出
        if (buttonBounds.contains(adjustedPos)) {
            std::cout << "B道具按钮被点击: 鼠标位置(" << mousePos.x << "," << mousePos.y << "), "
                      << "转换位置(" << adjustedX << "," << adjustedY << "), "
                      << "按钮位置(" << buttonBounds.left << "," << buttonBounds.top << "), "
                      << "按钮尺寸(" << buttonBounds.width << "," << buttonBounds.height << ")" << std::endl;
        }
        
        return buttonBounds.contains(adjustedPos);
    }
    return false;
}

// 检查A绝对路径按钮是否被点击
bool VirtualMaze::isAbsPathButtonAClicked(sf::Vector2i mousePos) const {
    // 只处理左半屏的点击
    if (mousePos.x < window.getSize().x / 2) {
        sf::FloatRect buttonBounds = absPathButtonA.getGlobalBounds();
        return buttonBounds.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
    }
    return false;
}

// 检查B绝对路径按钮是否被点击
bool VirtualMaze::isAbsPathButtonBClicked(sf::Vector2i mousePos) const {
    // 只处理右半屏的点击
    if (mousePos.x >= window.getSize().x / 2) {
        // 右侧视图的按钮坐标是相对于右侧视图左上角的，需要将鼠标坐标进行转换
        float adjustedX = static_cast<float>(mousePos.x - window.getSize().x / 2);
        float adjustedY = static_cast<float>(mousePos.y);
        
        // 使用转换后的坐标检查点击
        sf::Vector2f adjustedPos(adjustedX, adjustedY);
        sf::FloatRect buttonBounds = absPathButtonB.getGlobalBounds();
        
        // 调试输出
        if (buttonBounds.contains(adjustedPos)) {
            std::cout << "B绝对路径按钮被点击: 鼠标位置(" << mousePos.x << "," << mousePos.y << "), "
                      << "转换位置(" << adjustedX << "," << adjustedY << "), "
                      << "按钮位置(" << buttonBounds.left << "," << buttonBounds.top << "), "
                      << "按钮尺寸(" << buttonBounds.width << "," << buttonBounds.height << ")" << std::endl;
        }
        
        return buttonBounds.contains(adjustedPos);
    }
    return false;
}



