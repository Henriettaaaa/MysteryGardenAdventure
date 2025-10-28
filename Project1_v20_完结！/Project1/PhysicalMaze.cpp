#include <iostream>
#include <queue>
#include <SFML/Graphics.hpp>

#include "PhysicalMaze.h"

int PhysicalMaze::inputSize() {
    // 创建一个窗口用于选择难度
    sf::RenderWindow window(sf::VideoMode(400, 300), "select difficulty", sf::Style::Close);
    window.setFramerateLimit(60);
    
    sf::Font font;
    // 尝试加载字体
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "fail to load font, use default difficulty(medium)" << std::endl;
        return 7;  // 如果字体加载失败，默认返回中等难度
    }
    
    // 创建标题文本
    sf::Text titleText("Please select difficulty", font, 30);
    titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition((400 - titleBounds.width) / 2, 30);
    
    // 创建三个难度按钮
    sf::RectangleShape easyButton(sf::Vector2f(200, 50));
    easyButton.setPosition(100, 80);
    easyButton.setFillColor(sf::Color(163, 168, 143));
    
    sf::Text easyText("Easy (5)", font, 20);
    easyText.setFillColor(sf::Color::White);
    sf::FloatRect easyBounds = easyText.getLocalBounds();
    easyText.setPosition(
        easyButton.getPosition().x + (easyButton.getSize().x - easyBounds.width) / 2,
        easyButton.getPosition().y + (easyButton.getSize().y - easyBounds.height) / 2 - 5
    );
    
    sf::RectangleShape mediumButton(sf::Vector2f(200, 50));
    mediumButton.setPosition(100, 145);
    mediumButton.setFillColor(sf::Color(163, 168, 143));
    
    sf::Text mediumText("Medium (7)", font, 20);
    mediumText.setFillColor(sf::Color::White);
    sf::FloatRect mediumBounds = mediumText.getLocalBounds();
    mediumText.setPosition(
        mediumButton.getPosition().x + (mediumButton.getSize().x - mediumBounds.width) / 2,
        mediumButton.getPosition().y + (mediumButton.getSize().y - mediumBounds.height) / 2 - 5
    );
    
    sf::RectangleShape hardButton(sf::Vector2f(200, 50));
    hardButton.setPosition(100, 210);
    hardButton.setFillColor(sf::Color(163, 168, 143));
    
    sf::Text hardText("Hard (9)", font, 20);
    hardText.setFillColor(sf::Color::White);
    sf::FloatRect hardBounds = hardText.getLocalBounds();
    hardText.setPosition(
        hardButton.getPosition().x + (hardButton.getSize().x - hardBounds.width) / 2,
        hardButton.getPosition().y + (hardButton.getSize().y - hardBounds.height) / 2 - 5
    );
    
    int selectedSize = 7;  // 默认中等难度
    
    // 主循环
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return selectedSize;  // 如果关闭窗口，返回当前选择的难度
            }
            
            // 处理鼠标移动，改变按钮颜色
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
                
                // 检查鼠标是否悬停在按钮上
                if (easyButton.getGlobalBounds().contains(mousePos)) {
                    easyButton.setFillColor(sf::Color(103, 135, 85));
                } else {
                    easyButton.setFillColor(sf::Color(163, 168, 143));
                }
                
                if (mediumButton.getGlobalBounds().contains(mousePos)) {
                    mediumButton.setFillColor(sf::Color(103, 135, 85));
                } else {
                    mediumButton.setFillColor(sf::Color(163, 168, 143));
                }
                
                if (hardButton.getGlobalBounds().contains(mousePos)) {
                    hardButton.setFillColor(sf::Color(103, 135, 85));
                } else {
                    hardButton.setFillColor(sf::Color(163, 168, 143));
                }
            }
            
            // 处理鼠标点击
            if (event.type == sf::Event::MouseButtonPressed && 
                event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                
                if (easyButton.getGlobalBounds().contains(mousePos)) {
                    selectedSize = 5;  // 简单难度
                    window.close();
                    return selectedSize;
                }
                
                if (mediumButton.getGlobalBounds().contains(mousePos)) {
                    selectedSize = 7;  // 中等难度
                    window.close();
                    return selectedSize;
                }
                
                if (hardButton.getGlobalBounds().contains(mousePos)) {
                    selectedSize = 9;  // 困难难度
                    window.close();
                    return selectedSize;
                }
            }
        }
        
        // 清空窗口并绘制所有元素
        window.clear(sf::Color(169, 189, 126));
        window.draw(titleText);
        window.draw(easyButton);
        window.draw(easyText);
        window.draw(mediumButton);
        window.draw(mediumText);
        window.draw(hardButton);
        window.draw(hardText);
        window.display();
    }
    
    return selectedSize;  // 默认返回选择的难度
}

//*初始化迷宫参数
PhysicalMaze::PhysicalMaze(int s) : size(s) {

    //way.resize(size * size);
    parent.resize(size * size);
    weight.resize(size * size);

    for (int i = 0; i < size * size; i++) {
        parent[i] = -1;  // 初始化parent数组，每个节点的父节点初始为-1
        weight[i] = 1;   // 初始化权重为1
        //way[i] = 0;      // 初始化way数组
    }
}

//*生成迷宫
void PhysicalMaze::generateMaze(int size) {
    std::vector<std::pair<int, int>> wallsToUse;
    
    // 检查是否有存储的迷宫记录
    bool hasStored = hasStoredMazes(size);
    
    // 随机选择是使用新生成还是从记录中读取
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    bool useNewGeneration = true;
    if (hasStored) {
        // 如果有存储记录，50%概率选择新生成，50%概率选择旧记录
        useNewGeneration = dis(gen) < 0.5;
    }
    
    if (useNewGeneration || !hasStored) {
        // 新生成迷宫
        std::cout << "Generating new maze..." << std::endl;
        wallsToUse = generateWalls(size);
        // 保存到文件
        saveWallsToFile(wallsToUse, size);
        std::cout << "New maze saved to file." << std::endl;
    } else {
        // 从文件中读取随机一个
        std::cout << "Loading maze from stored records..." << std::endl;
        wallsToUse = loadRandomWallsFromFile(size);
        std::cout << "Maze loaded from file." << std::endl;
    }
    
    // 将选择的墙数据赋值给成员变量
    walls = wallsToUse;
    
    //下面的代码都不需要改变。因为只是墙的数组的来源不同，后续处理方式都相同
    //从最后面往前遍历，这样不影响删边
    for (int i = walls.size() - 1; i >= 0; i--) {
        int x = walls[i].first;
        int y = walls[i].second;
        unionSet(x, y, i);
    }
    std::cout << "maze generated!" << std::endl;
}

// 检查是否有指定尺寸的存储迷宫
bool PhysicalMaze::hasStoredMazes(int size) const {
    std::string filename = "maze_walls_" + std::to_string(size) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool hasData = false;
    while (std::getline(file, line)) {
        if (!line.empty() && line != "---") {
            hasData = true;
            break;
        }
    }
    file.close();
    return hasData;
}

// 保存墙数据到文件
void PhysicalMaze::saveWallsToFile(const std::vector<std::pair<int, int>>& walls, int size) const {
    std::string filename = "maze_walls_" + std::to_string(size) + ".txt";
    std::ofstream file(filename, std::ios::app); // 追加模式
    
    if (!file.is_open()) {
        std::cout << "Error: Cannot open file " << filename << " for writing." << std::endl;
        return;
    }
    
    // 写入墙数据，每对墙用空格分隔，每条记录用换行分隔
    for (const auto& wall : walls) {
        file << wall.first << " " << wall.second << " ";
    }
    file << std::endl;
    file << "---" << std::endl; // 分隔符，用于区分不同的迷宫记录
    
    file.close();
}

// 从文件中随机读取一个墙数据
std::vector<std::pair<int, int>> PhysicalMaze::loadRandomWallsFromFile(int size) {
    std::string filename = "maze_walls_" + std::to_string(size) + ".txt";
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Error: Cannot open file " << filename << " for reading." << std::endl;
        return generateWalls(size); // 如果读取失败，返回新生成的
    }
    
    // 读取所有迷宫记录
    std::vector<std::vector<std::pair<int, int>>> allMazes;
    std::string line;
    std::vector<std::pair<int, int>> currentMaze;
    
    while (std::getline(file, line)) {
        if (line == "---") {
            // 遇到分隔符，保存当前迷宫
            if (!currentMaze.empty()) {
                allMazes.push_back(currentMaze);
                currentMaze.clear();
            }
        } else if (!line.empty()) {
            // 解析墙数据
            std::istringstream iss(line);
            int first, second;
            while (iss >> first >> second) {
                currentMaze.emplace_back(first, second);
            }
        }
    }
    
    // 处理最后一个迷宫（如果文件末尾没有分隔符）
    if (!currentMaze.empty()) {
        allMazes.push_back(currentMaze);
    }
    
    file.close();
    
    // 如果没有找到有效的迷宫记录，生成新的
    if (allMazes.empty()) {
        std::cout << "No valid maze records found, generating new maze." << std::endl;
        return generateWalls(size);
    }
    
    // 随机选择一个迷宫
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, allMazes.size() - 1);
    int selectedIndex = dis(gen);
    
    std::cout << "Selected maze record " << (selectedIndex + 1) << " out of " << allMazes.size() << " available records." << std::endl;
    
    return allMazes[selectedIndex];
}

/*
这种实现使用现代的 C++ 17随机数生成器，产生的随机数质量更好
线程安全
*/
//给所有格子随机打乱顺序
//返回一个随机数组
//*提取并表示每两个相邻格子之间的墙
//*返回一个随机打乱的墙的数组
std::vector<std::pair<int, int>> PhysicalMaze::generateWalls(int size) {
    //std::vector<std::pair<int,int>> walls;

    // 生成所有的墙 
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int current = i * size + j;

            // 添加右边的墙
            if (j < size - 1) {
                walls.emplace_back(current, current + 1);
            }

            // 添加下边的墙
            if (i < size - 1) {
                walls.emplace_back(current, current + size);
            }
        }
    }

    // 随机打乱
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(walls.begin(), walls.end(), gen);

    return walls;
}

// !路径压缩找根节点
int PhysicalMaze::find(int x) {
    if (parent[x] == -1) {
        return x;        // 返回根节点的索引
    }
    parent[x] = find(parent[x]);  // 递归找到根节点，并压缩路径
    return parent[x];   // 返回根节点的索引
}

//*带权重的合并
//结束条件是所有格子都属于同一个等价类
void PhysicalMaze::unionSet(int x, int y, int i) {
    int root_x = find(x);    // 获取x的根节点
    int root_y = find(y);    // 获取y的根节点

    if (root_x == root_y) {  // 如果已经在同一集合中，不删边
        return;
    }

    // 按权重合并
    if (weight[root_x] < weight[root_y]) {
        parent[root_x] = root_y;
        weight[root_y] += weight[root_x];
    }
    else {
        parent[root_y] = root_x;
        weight[root_x] += weight[root_y];
    }
    //删边
    walls.erase(walls.begin() + i);
}

void PhysicalMaze::findPath(int size) {
    //std::vector<int> path;  // 存储最终路径
    std::vector<bool> visited(size * size, false);  // 访问标记数组
    std::vector<int> prev(size * size, -1);  // 前驱节点数组，用于重建路径
    std::queue<int> q;  // BFS队列

    // 起点为左上角(0)
    int start = 0;
    // 终点为右下角(size*size-1)
    int end = size * size - 1;

    // 将起点加入队列
    q.push(start);
    visited[start] = true;

    // BFS主循环
    while (!q.empty()) {
        int current = q.front();
        q.pop();

        // 如果到达终点，退出循环
        if (current == end) {
            break;
        }

        // 获取当前节点的行列坐标
        int row = current / size;
        int col = current % size;

        // 检查四个方向的相邻节点
        // 右、下、左、上的偏移量
        int dr[] = { 0, 1, 0, -1 };
        int dc[] = { 1, 0, -1, 0 };

        for (int i = 0; i < 4; i++) {
            int newRow = row + dr[i];
            int newCol = col + dc[i];

            // 检查是否在迷宫范围内
            if (newRow >= 0 && newRow < size && newCol >= 0 && newCol < size) {
                int next = newRow * size + newCol;

                // 如果没有访问过
                if (!visited[next]) {
                    // 检查是否有墙
                    bool hasWall = false;
                    for (const auto& wall : walls) {
                        if ((wall.first == current && wall.second == next) ||
                            (wall.first == next && wall.second == current)) {
                            hasWall = true;
                            break;
                        }
                    }

                    if (!hasWall) {
                        visited[next] = true;
                        prev[next] = current;
                        q.push(next);
                    }
                }
            }
        }
    }

    // 从终点回溯到起点，重建路径
    if (visited[end]) {
        for (int at = end; at != -1; at = prev[at]) {
            path.push_back(at);
        }
        // 反转路径，使其从起点开始
        std::reverse(path.begin(), path.end());
    }
}

bool PhysicalMaze::hasWall(int cell1, int cell2) const {
    // 检查是否存在这面墙
    for (const auto& wall : walls) {
        if ((wall.first == cell1 && wall.second == cell2) ||
            (wall.first == cell2 && wall.second == cell1)) {
            return true;  // 有墙
        }
    }
    return false;  // 没有墙
}
