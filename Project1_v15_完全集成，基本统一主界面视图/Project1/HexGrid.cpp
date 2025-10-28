#include "HexGrid.h"
#include <iostream>
#include <chrono>
#include <iomanip> // 用于格式化时间显示
#include <sstream> // 用于字符串流

// 构造函数
HexGrid::HexGrid(int radius) : gridRadius(radius), gameOver(false) {
    // 初始化随机数生成器
    rng = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    // 初始化计时器
    elapsedTime = std::chrono::duration<double>(0);
}

// 初始化网格
void HexGrid::initialize() {
    // 创建SFML窗口
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Hex Grid Pathfinding");
    window.setFramerateLimit(60);
    
    // 设置网格原点（屏幕中心）
    gridOrigin = sf::Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    
    // 生成六边形网格
    generate_hex_grid(gridRadius, gridData, boundaryCells);
    
    // 选择起点和终点
    fix_start_end(gridRadius, gridData, startHex, endHex);
    
    // 设置玩家初始位置为起点
    playerPos = startHex;
    
    // 设置起点为玩家位置
    gridData[startHex] = CellState::Player;
}

// 处理键盘输入
void HexGrid::handleInput(sf::Event event) {
    if (event.type == sf::Event::KeyPressed && !gameOver) {
        Direction moveDir;
        bool validKey = true;
        

        // 根据按键确定移动方向
        switch (event.key.code) {
            case sf::Keyboard::A:
                moveDir = Direction::Left;
                break;
            case sf::Keyboard::D:
                moveDir = Direction::Right;
                break;
            case sf::Keyboard::W:
                moveDir = Direction::UpLeft;
                break;
            case sf::Keyboard::X:
                moveDir = Direction::DownRight;
                break;
            case sf::Keyboard::E:
                moveDir = Direction::UpRight;
                break;
            case sf::Keyboard::Z:
                moveDir = Direction::DownLeft;
                break;
            default:
                validKey = false;
                break;
        }
        
        // 如果是有效的移动键
        if (validKey) {
            // 存储旧位置
            HexCoord oldPos = playerPos;
            
            // 尝试移动玩家
            bool moved = move_player(moveDir, playerPos, gridData);
            
            if (moved) {
                // 更新旧位置的格子状态
                if (oldPos == startHex) {
                    gridData[oldPos] = CellState::Start;
                } else if (oldPos == endHex) {
                    gridData[oldPos] = CellState::End;
                } else {
                    // 恢复格子的原始状态，而不是简单地设为Empty
                    gridData[oldPos] = originalGridState[oldPos];
                }                
                
                // 更新玩家新位置
                gridData[playerPos] = CellState::Player;
                if (playerPos == pathCells_A[check_A]) {
                    std::cout << check_A + 1 << std::endl;
                    check_A++;                   
                    // 如果玩家到达路径A的结尾
                    if (playerPos == pathCells_A[pathCells_A.size() - 1]) {
                        gameOver = true;
                        // 停止计时器
                        if (timerRunning) {
                            timerRunning = false;
                            auto endTime = std::chrono::high_resolution_clock::now();
                            elapsedTime = endTime - startTime;
                            // 输出完成时间
                            int minutes = static_cast<int>(elapsedTime.count()) / 60;
                            int seconds = static_cast<int>(elapsedTime.count()) % 60;
                            std::cout << "Time: " << std::setfill('0') << std::setw(2) << minutes 
                                      << ":" << std::setfill('0') << std::setw(2) << seconds << std::endl;
                        }
                        std::cout << "You win!" << std::endl;
                    }
                }
            }
        }
    }
}

// 更新游戏状态
void HexGrid::update() {
    // 更新计时器
    if (timerRunning && !gameOver) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        elapsedTime = currentTime - startTime;
    }
}

// 运行网格生成和寻路逻辑
void HexGrid::run() {
    // 寻找路径
    std::cout << "Finding path ..." << std::endl;
    bool path_found = findPath(startHex, endHex, gridData, cameFrom, visited, rng);
    
    // 如果找到路径，重建并标记
    if (path_found) {
        // 标记绝对路径
        reconstruct_path(startHex, endHex, gridData, cameFrom, visited, pathCells);
        // 标记相对路径
        calculateHexRelative(startHex, endHex, pathCells, relativePathCells, gridData);
        
        // 保存所有格子的原始状态
        originalGridState = gridData;
        
        // 清空之前的序号数据
        gridNumbers.clear();
        
        // 标记绝对路径序号 - 从起点和终点同时向中间标号
        int startIndex = 1;  
        int endIndex = 1;
        // 绝对路径的左右指针
        int left = 0;  // 从终点开始
        int right = pathCells.size() - 1;  // 从终点开始
        int mid = (pathCells.size() + 1) / 2;  //绝对路径中点，用于划分A和B，A的路径需要隐去
        
        // 将起点的序号设为1
        gridNumbers[startHex].push_back(1);
        //pathCells_A.push_back(startHex);  //将起点加入A的绝对路径
        //gridNumbers[endHex].push_back(1);
        
        pathCells_A.clear();
        pathCells_B.clear();
        // 使用双指针从两端向中间标记序号
        while (left <= right) {            
            // 标记从终点方向的序号            
            gridNumbers[pathCells[left]].push_back(startIndex);
            pathCells_B.push_back(pathCells[left]); 
            startIndex++;
            left++;

            // 标记从起点方向的序号       
            //gridNumbers[pathCells[right]].push_back(endIndex);
            pathCells_A.push_back(pathCells[right]);
            endIndex++;
            right--;
        }
        
        // 标记相对路径序号
        for (size_t i = 0; i < relativePathCells.size(); ++i) {
            gridNumbers[relativePathCells[i]].push_back(i + 2);
        }
    } else {
        std::cout << "Path not found between start and end." << std::endl;
    }
    
    // 重新设置起点为玩家位置
    gridData[startHex] = CellState::Player;

    //标记B的绝对路径（即题面）
    for (const auto& cell : pathCells_B) {
        if (cell != endHex && cell != startHex) {
            gridData[cell] = CellState::Path;
        }
    }
    
    // 开始计时
    startTime = std::chrono::high_resolution_clock::now();
    timerRunning = true;
}

// 绘制计时器
void HexGrid::drawTimer() {
    // 创建用于显示计时器的字体
    static sf::Font font;
    static bool fontLoaded = false;
    
    // 首次加载字体
    if (!fontLoaded) {
        // 尝试从本地加载字体文件
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font for timer" << std::endl;
            return;
        }
        fontLoaded = true;
    }
    
    // 计算时间
    int totalSeconds = static_cast<int>(elapsedTime.count());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    
    // 格式化时间字符串
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" 
       << std::setfill('0') << std::setw(2) << seconds;
    
    // 创建文本对象
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setString(ss.str());
    timerText.setCharacterSize(24); // 设置适当的大小
    timerText.setFillColor(sf::Color::White);
    
    // 将计时器放在窗口顶部中间位置
    timerText.setPosition(WINDOW_WIDTH / 2.0f - timerText.getLocalBounds().width / 2.0f, 10.0f);
    
    // 绘制计时器
    window.draw(timerText);
}

// 显示网格
void HexGrid::display() {
    // 主循环
    while (window.isOpen()) {
        // 处理事件
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            // 处理键盘输入
            handleInput(event);
        }
        
        // 更新游戏状态
        update();
        
        // 渲染带序号的网格
        render_grid_with_numbers(window, gridOrigin, gridData, gridNumbers);
        
        // 绘制计时器
        drawTimer();
        
        // 显示
        window.display();
    }
} 