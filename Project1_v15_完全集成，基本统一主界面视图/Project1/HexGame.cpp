#include "HexGame.h"

// 构造函数
HexGame::HexGame(bool isServer, const std::string& serverIP, unsigned short port)
    : networkManager(NetworkManager::getInstance()),
      state(),
      clockStarted(false),
      serverIP(serverIP),
      port(port)
{
    state.isServer = isServer;
    gridOrigin = sf::Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
}

// 析构函数
HexGame::~HexGame()
{
    // 关闭网络连接
    networkManager.close();
}

// 初始化游戏
bool HexGame::initialize()
{
    // 初始化网络连接
    if (state.isServer) {
        if (!networkManager.initializeServer(port)) {
            std::cerr << "Failed to initialize server" << std::endl;
            return false;
        }
    } else {
        if (!networkManager.initializeClient(serverIP, port)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }
    }
    
    // 初始化游戏状态
    initializeGameState();
    
    // 设置网络回调
    setupNetworkCallbacks();
    
    // 创建窗口
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                  state.isServer ? "Hex Grid Game - Server" : "Hex Grid Game - Client");
    window.setFramerateLimit(60);
    
    return true;
}

// 初始化游戏状态
void HexGame::initializeGameState()
{
    state.gameStarted = false;
    state.clientReady = false;
    state.gameTimeSynced = false;
    state.gameTime = 0.0f;
    state.gameEnded = false;
    state.checkA = 1;
    state.pathA.clear();
    state.checkB = 1;
    state.pathB.clear();
    state.isSecondPlayerChecked = false;
    
    // 初始化服务器检查点数据字段
    state.serverCheckA = 1;
    state.serverCheckB = 1;
    state.serverPathSize = 0;
    
    // 初始化随机数生成器
    std::random_device rd;
    state.rng = std::mt19937(rd());
    
    if (state.isServer) {
        // 服务器生成迷宫，传递hasClientConnected函数以持续检查客户端连接状态
        int radius = input_grid_size(
            networkManager.hasClientConnected(), // 初始连接状态
            [this]() -> bool { 
                return this->networkManager.hasClientConnected(); 
            } // 检查函数
        );
        std::vector<HexCoord> boundary_cells;
        generate_hex_grid(radius, state.gridData, boundary_cells);
        fix_start_end(radius, state.gridData, state.startHex, state.endHex);
        
        // 初始化路径
        std::unordered_map<HexCoord, HexCoord> came_from;
        std::unordered_set<HexCoord> visited;
        std::vector<HexCoord> path_cells;
        findPath(state.startHex, state.endHex, state.gridData, came_from, visited, state.rng);
        reconstruct_path(state.startHex, state.endHex, state.gridData, came_from, visited, path_cells);
        
        // 创建类似于原始HexGrid中的pathCells_B来标记Path类型单元格
        std::vector<HexCoord> path_cells_B;
        
        // 从末端到起点标记路径（与原始HexGrid中的逻辑类似）
        int left = 0;  // 从末端开始
        int right = path_cells.size() - 1; // 从起点开始
        
        // 分别标记A路径和B路径
        state.gridNumbers.clear();
        
        // 标记起点编号
        state.gridNumbers[state.startHex].push_back(1);
        
        // 从两端标记（与原始HexGrid中的逻辑类似）
        int startIndex = 1;
        int endIndex = 1;
        
        while (left <= right) {
            // 标记B路径（从末端到起点）
            path_cells_B.push_back(path_cells[left]);
            state.gridNumbers[path_cells[left]].push_back(startIndex);
            startIndex++;
            left++;
            
            // 标记A路径（从起点到末端）
            if (right >= 0) {
                state.pathA.push_back(path_cells[right]);
                right--;
            }
        }
        
        // 创建第二个玩家的路径（B路径），从end到start
        for (int i = 0; i < path_cells_B.size(); i++) {
            state.pathB.push_back(path_cells_B[i]);
        }
        
        // 将B路径单元格标记为Path类型
        for (const auto& cell : path_cells_B) {
            if (cell != state.endHex && cell != state.startHex) {
                state.gridData[cell] = CellState::Path;
            }
        }
        
        // 计算相对路径
        std::vector<HexCoord> relative_path_cells;
        calculateHexRelative(state.startHex, state.endHex, path_cells, relative_path_cells, state.gridData);
        
        // 标记相对路径编号
        for (size_t i = 0; i < relative_path_cells.size(); ++i) {
            state.gridNumbers[relative_path_cells[i]].push_back(i + 2);
        }
        
        state.playerPos = state.startHex;
        state.otherPlayerPos = state.startHex;
    } else {
        // 客户端初始化，将玩家位置设为起点
        state.playerPos = HexCoord(0, 0); // 临时位置，会在接收到网格数据时更新
    }
}

// 设置网络回调
void HexGame::setupNetworkCallbacks()
{
    // 设置玩家位置回调函数
    networkManager.setPlayerPositionCallback([this](const HexCoord& pos) {
        // 原子操作，更新其他玩家的位置
        std::cout << "[Player Position Callback] Received position: (" << pos.q << "," << pos.r << ")" << std::endl;
        
        // 更新其他玩家的位置
        state.otherPlayerPos = pos;
        std::cout << "Updated other player position to: (" << pos.q << "," << pos.r << ")" << std::endl;
        
        // 如果是服务器端，检查客户端玩家的位置
        if (state.isServer) {
            std::cout << "Server checking client player position: (" << pos.q << "," << pos.r << ")" << std::endl;
            // 服务器端检查第二个玩家（客户端玩家）的位置
            checkOtherPlayerMovement(pos);
        } else {
            // 客户端检查服务器玩家的位置
            std::cout << "Client checking server player position: (" << pos.q << "," << pos.r << ")" << std::endl;
            // 只有在路径已建立且玩家检查已激活时才检查
            if (state.isSecondPlayerChecked && !state.pathA.empty()) {
                bool checkResult = checkPlayerMovement(pos);
                std::cout << "Client checking server player: " << (checkResult ? "valid move" : "invalid move")
                        << ", current checkpoint: " << state.checkA << "/" << state.pathA.size() << std::endl;
            }
        }
    });

    // 设置网格数据回调
    networkManager.setGridDataCallback([this](const std::unordered_map<HexCoord, CellState>& gridData, 
                                             const HexCoord& startHex, 
                                             const HexCoord& endHex) {
        state.gridData = gridData;
        state.startHex = startHex;
        state.endHex = endHex;
        state.playerPos = startHex;
        std::cout << "received grid data, start: (" << startHex.q << "," << startHex.r << "), end: (" 
                 << endHex.q << "," << endHex.r << ")" << std::endl;
                 
        if (!state.isServer) {
            state.otherPlayerPos = startHex;
            state.gameStarted = true;
            
            // 激活第二个玩家的位置检查，并重置检查点
            state.isSecondPlayerChecked = true;
            state.checkA = 1;  // 重置第一个玩家的检查点
            state.checkB = 1;  // 重置第二个玩家的检查点
            
            // 在接收到网格数据后发送客户端就绪消息
            sendClientReady();
        }
    });

    // 设置游戏状态回调
    networkManager.setGameStateCallback([this](bool gameStarted, float gameTime, bool gameEnded) {
        if (state.isServer) {
            // 服务器接收客户端就绪消息
            if (gameStarted && !state.clientReady) {
                state.clientReady = true;
                std::cout << "Client is ready!" << std::endl;
                
                // 发送游戏开始信号
                sendGameStartSignal();
                
                // 同步开始时间
                state.gameTimeSynced = true;
            }
        } else {
            // 客户端接收游戏状态更新
            if (gameStarted && !state.gameTimeSynced) {
                state.gameTimeSynced = true;
                std::cout << "Game time synced!" << std::endl;
            }
            
            if (gameEnded) {
                state.gameTime = gameTime;
                state.gameEnded = true;
                std::cout << "Game ended with time: " << gameTime << "s" << std::endl;
            }
        }
    });

    // 设置网格编号回调
    networkManager.setGridNumbersCallback([this](const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
        if (!state.isServer) {
            state.gridNumbers = gridNumbers;
            std::cout << "Received grid numbers" << std::endl;
            
            // 客户端需要基于接收到的gridNumbers构建路径
            state.pathA.clear();
            state.pathB.clear();
            
            // 找出所有带有序号的格子并按序号排序
            std::vector<std::pair<HexCoord, int>> numberedCells;
            for (const auto& pair : gridNumbers) {
                // 如果格子有序号（向量非空）
                if (!pair.second.empty()) {
                    // 使用第一个序号（大多数格子只有一个序号）
                    numberedCells.push_back(std::make_pair(pair.first, pair.second[0]));
                }
            }
            
            // 按序号排序
            std::sort(numberedCells.begin(), numberedCells.end(), 
                [](const std::pair<HexCoord, int>& a, const std::pair<HexCoord, int>& b) {
                    return a.second < b.second;
                });
            
            // 构建路径B (客户端玩家使用的路径，从起点向中点方向)
            for (const auto& cell : numberedCells) {
                state.pathB.push_back(cell.first);
            }
            
            // 构建pathA (用于检查服务器玩家的路径，从起点到终点)
            // 同样先添加所有格子，但在客户端我们只检查一半路程
            for (int i = numberedCells.size() - 1; i >= 0; i--) {
                state.pathA.push_back(numberedCells[i].first);
            }
            
            std::cout << "Client constructed paths: A path length=" << state.pathA.size() 
                     << ", B path length=" << state.pathB.size() << std::endl;
        }
    });

    // 设置检查点数据回调
    networkManager.setCheckpointDataCallback([this](int checkA, int checkB, int pathSize) {
        if (!state.isServer) {
            // 只有客户端才需要更新检查点数据
            state.serverCheckA = checkA;
            state.serverCheckB = checkB;
            state.serverPathSize = pathSize;
            std::cout << "Received checkpoint data: A=" << checkA << ", B=" << checkB << ", PathSize=" << pathSize << std::endl;
        }
    });
}

// 向客户端发送客户端就绪消息
void HexGame::sendClientReady()
{
    std::cout << "Sending client ready message..." << std::endl;
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GameState);
    
    // 准备数据
    std::vector<uint8_t> data;
    data.reserve(sizeof(bool) * 3);
    
    bool ready = true;
    bool dummy = false;
    float dummyTime = 0.0f;
    
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&ready),
               reinterpret_cast<uint8_t*>(&ready) + sizeof(bool));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&dummyTime),
               reinterpret_cast<uint8_t*>(&dummyTime) + sizeof(float));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&dummy),
               reinterpret_cast<uint8_t*>(&dummy) + sizeof(bool));
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    networkManager.serverSocket.send(packet);
}

// 服务器发送游戏开始信号
void HexGame::sendGameStartSignal()
{
    std::cout << "Sending game start signal..." << std::endl;
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GameState);
    
    // 准备数据
    std::vector<uint8_t> data;
    data.reserve(sizeof(bool) * 3);
    
    bool gameStarted = true;
    bool gameEnded = false;
    float gameTime = 0.0f;
    
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&gameStarted),
               reinterpret_cast<uint8_t*>(&gameStarted) + sizeof(bool));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gameTime),
               reinterpret_cast<uint8_t*>(&gameTime) + sizeof(float));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gameEnded),
               reinterpret_cast<uint8_t*>(&gameEnded) + sizeof(bool));
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    for (auto& client : networkManager.clients) {
        client->send(packet);
    }
}

// 处理网络消息
void HexGame::handleNetworkMessages()
{
    // 只从队列中检索消息并丢弃，由线程回调处理实际处理
    NetworkMessage msg;
    while (networkManager.receiveMessage(msg)) {
        // 队列中的消息已通过线程回调处理
        // 只是清空队列以防止堆积
    }
}

// 检查玩家移动是否正确（按顺序沿着路径A前进）
bool HexGame::checkPlayerMovement(const HexCoord& newPos)
{
    // 当玩家不在起点时
    if (newPos != state.startHex) {
        // !检查是否走在正确的路线上
        if (state.checkA < state.pathA.size() && newPos == state.pathA[state.checkA]) {
            // 如果走到正确的位置，检查点计数+1
            state.checkA++;
            std::cout << "Player reached correct checkpoint: " << state.checkA 
                     << "/" << state.pathA.size() 
                     << " at position (" << newPos.q << "," << newPos.r << ")" << std::endl;
            return true;
        } else {
            // 如果不在正确路径上但仍在可移动单元格上，允许移动但不增加检查点
            std::cout << "Player moved to non-checkpoint position: (" << newPos.q << "," << newPos.r << ")" << std::endl;
            return true;
        }
    }
    return true; // 在起点移动也是允许的
}

// 检查第二个玩家（客户端玩家）的移动是否正确（沿着路径B前进）
bool HexGame::checkOtherPlayerMovement(const HexCoord& newPos)
{
    // 当玩家不在起点时
    if (newPos != state.startHex) {
        // 检查是否在正确的路径上前进
        if (state.checkB < state.pathA.size() && newPos == state.pathA[state.checkB]) {
            // 如果在正确位置，增加检查点
            state.checkB++;
            std::cout << "Second player reached correct checkpoint: " << state.checkB 
                     << "/" << state.pathA.size() 
                     << " at position (" << newPos.q << "," << newPos.r << ")" << std::endl;
            return true;
        } else {
            // 如果不在正确路径上但仍在可移动的单元格内，允许移动但不增加检查点
            std::cout << "Second player moved to non-checkpoint position: (" << newPos.q << "," << newPos.r << ")" << std::endl;
            return true;
        }
    }
    return true; // 在起点移动也是允许的
}

// 处理键盘输入
void HexGame::handleKeyPress(sf::Keyboard::Key key)
{
    if (!state.gameStarted || !state.gameTimeSynced || state.gameEnded) {
        return;
    }
    
    Direction dir;
    switch (key) {
        case sf::Keyboard::A: dir = Direction::Left; break;
        case sf::Keyboard::D: dir = Direction::Right; break;
        case sf::Keyboard::W: dir = Direction::UpLeft; break;
        case sf::Keyboard::X: dir = Direction::DownRight; break;
        case sf::Keyboard::E: dir = Direction::UpRight; break;
        case sf::Keyboard::Z: dir = Direction::DownLeft; break;
        default: return;
    }
    
    HexCoord newPos = state.playerPos + get_direction_offset(dir);
    if (move_player(dir, state.playerPos, state.gridData)) {
        // 检查移动是否正确
        if (state.isServer) {
            checkPlayerMovement(state.playerPos);
        } else if (state.isSecondPlayerChecked) {
            // 客户端玩家移动时也进行检查，注意客户端应该使用pathB路径来检查自己的移动
            bool checkResult = checkOtherPlayerMovement(state.playerPos);
            std::cout << "Client check result: " << (checkResult ? "valid move" : "invalid move") 
                     << ", current checkpoint: " << state.checkB << "/" << state.pathB.size() << std::endl;
        }
        
        // 调试输出
        std::cout << "Player moved to: (" << state.playerPos.q << "," << state.playerPos.r << "), preparing to send position update" << std::endl;
        
        // 尝试发送位置3次以确保成功更新
        bool sent = false;
        for (int attempt = 1; attempt <= 3 && !sent; attempt++) {
            std::cout << "Attempting to send position, attempt #" << attempt << std::endl;
            sent = networkManager.sendPlayerPosition(state.playerPos);
            if (sent) {
                std::cout << "Position update sent successfully!" << std::endl;
                break;
            } else {
                std::cout << "Position send failed! Retrying..." << std::endl;
                // 重试前短暂延迟，延迟时间逐渐增加
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * attempt));
            }
        }
        
        if (!sent) {
            std::cout << "All position update attempts failed" << std::endl;
        }
        
        // 检查是否到达终点
        if (check_win(state.playerPos, state.endHex)) {
            state.gameEnded = true;
            float endTime = gameClock.getElapsedTime().asSeconds();
            networkManager.sendGameEndTime(endTime);
            std::cout << "You win! Time: " << endTime << " seconds" << std::endl;
        }
    }
}

// 处理游戏事件
void HexGame::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        }
    }
}

// 更新游戏状态
void HexGame::update()
{
    // 处理网络消息
    handleNetworkMessages();
    
    // 如果是服务器并且游戏已开始，发送网格数据
    if (state.isServer && !state.gameStarted) {
        networkManager.sendGridData(state.gridData, state.startHex, state.endHex);
        networkManager.sendGridNumbers(state.gridNumbers);  // 发送网格编号数据
        state.gameStarted = true;
    }
    
    // 服务器端定期发送检查点数据更新
    if (state.isServer && state.gameStarted && state.clientReady) {
        if (checkpointClock.getElapsedTime().asSeconds() >= 0.5f) { // 每0.5秒发送一次
            networkManager.sendCheckpointData(state.checkA, state.checkB, state.pathA.size());
            checkpointClock.restart();
        }
    }
    
    // 启动游戏时钟（当所有客户端都准备就绪时）
    if ((state.isServer && state.clientReady && !clockStarted) || 
        (!state.isServer && state.gameTimeSynced && !clockStarted)) {
        gameClock.restart();
        clockStarted = true;
        std::cout << "Game clock started!" << std::endl;
    }
    
    // 更新游戏时间
    if (clockStarted && !state.gameEnded) {
        state.gameTime = gameClock.getElapsedTime().asSeconds();
    }
}

// 渲染游戏
void HexGame::render()
{
    window.clear(sf::Color::White);
    
    // 渲染带有编号的网格
    render_grid_with_numbers(window, gridOrigin, state.gridData, state.gridNumbers);
    
    // 渲染玩家位置
    sf::CircleShape playerShape(HEX_SIZE * 0.8f);
    playerShape.setFillColor(sf::Color::Blue);
    sf::Vector2f playerPos = axial_to_pixel(state.playerPos, HEX_SIZE, gridOrigin);
    playerShape.setPosition(playerPos - sf::Vector2f(HEX_SIZE * 0.8f, HEX_SIZE * 0.8f));
    window.draw(playerShape);
    
    // 渲染其他玩家位置
    sf::CircleShape otherPlayerShape(HEX_SIZE * 0.8f);
    otherPlayerShape.setFillColor(sf::Color::Green);
    sf::Vector2f otherPlayerPos = axial_to_pixel(state.otherPlayerPos, HEX_SIZE, gridOrigin);
    otherPlayerShape.setPosition(otherPlayerPos - sf::Vector2f(HEX_SIZE * 0.8f, HEX_SIZE * 0.8f));
    window.draw(otherPlayerShape);
    
    // 渲染时间和状态信息
    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        // 游戏时间文本
        sf::Text timeText;
        timeText.setFont(font);
        timeText.setCharacterSize(24);
        timeText.setFillColor(sf::Color::Black);
        
        // 显示游戏状态信息
        std::string statusInfo;
        if (!state.gameStarted) {
            statusInfo = "waiting for game start...";
        } else if (!state.gameTimeSynced) {
            statusInfo = "waiting for sync...";
        } else if (state.gameEnded) {
            statusInfo = "game over";
        } else {
            statusInfo = "game running";
        }
        
        timeText.setString("Time: " + std::to_string(static_cast<int>(state.gameTime)) + "s | " + statusInfo);
        timeText.setPosition(10, 10);
        window.draw(timeText);
        
        // 服务器端显示检查点信息
        if (state.isServer) {
            sf::Text checkText;
            checkText.setFont(font);
            checkText.setCharacterSize(18);
            checkText.setFillColor(sf::Color::Black);
            // !服务器端显示的服务器玩家的点位检查，逻辑正确
            // 服务器端现实的客户端玩家的点位检查，现在也正确
            checkText.setString("Player 1 Check Point: " + std::to_string(state.checkA) + "/" + 
                               std::to_string(state.pathA.size()) + 
                               " | Player 2 Check Point: " + std::to_string(state.checkB) + "/" + 
                               std::to_string(state.pathA.size()));
            checkText.setPosition(10, 40);
            window.draw(checkText);
        } else {
            // 客户端使用从服务器接收的检查点数据
            sf::Text checkText;
            checkText.setFont(font);
            checkText.setCharacterSize(18);
            checkText.setFillColor(sf::Color::Black);
            checkText.setString("Player 1 : " + std::to_string(state.serverCheckA) + "/" + 
                               std::to_string(state.serverPathSize) + 
                               " | Player 2 : " + std::to_string(state.serverCheckB) + "/" + 
                               std::to_string(state.serverPathSize));
            checkText.setPosition(10, 40);
            window.draw(checkText);
        }
    }
    
    window.display();
}

// 运行游戏
void HexGame::run()
{
    while (isRunning()) {
        processEvents();
        update();
        render();
    }
}

// 游戏是否运行中
bool HexGame::isRunning() const
{
    return window.isOpen();
} 