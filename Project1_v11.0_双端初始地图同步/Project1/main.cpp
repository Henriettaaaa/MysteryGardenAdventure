#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <random>
#include <iomanip> // 用于格式化输出
#include <sstream> // 用于字符串流
#include "SingleHex.h"
#include "NetworkManager.h"

// 游戏状态
struct GameState {
    std::unordered_map<HexCoord, CellState> gridData;
    std::unordered_map<HexCoord, std::vector<int>> gridNumbers; // 添加格子标号
    HexCoord playerPos;
    HexCoord otherPlayerPos;
    HexCoord startHex;
    HexCoord endHex;
    bool isServer;
    bool gameStarted;
    float gameTime;
    bool gameEnded;
    std::mt19937 rng; // 添加随机数生成器
    int checkA = 1; // 添加A路径检查点
};

// 处理命令行参数
bool parseArguments(int argc, char* argv[], bool& isServer, std::string& serverIP, unsigned short& port) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [server|client] [serverIP] [port]" << std::endl;
        return false;
    }
    
    std::string mode = argv[1];
    if (mode == "server") {
        isServer = true;
        port = (argc > 2) ? static_cast<unsigned short>(std::stoi(argv[2])) : 54000;
    }
    else if (mode == "client") {
        isServer = false;
        if (argc < 3) {
            std::cout << "Client mode requires server IP address" << std::endl;
            return false;
        }
        serverIP = argv[2];
        port = (argc > 3) ? static_cast<unsigned short>(std::stoi(argv[3])) : 54000;
    }
    else {
        std::cout << "Invalid mode. Use 'server' or 'client'" << std::endl;
        return false;
    }
    
    return true;
}

// 初始化游戏状态
void initializeGame(GameState& state, bool isServer) {
    state.isServer = isServer;
    state.gameStarted = false;
    state.gameTime = 0.0f;
    state.gameEnded = false;
    state.checkA = 1; // 初始化检查点
    
    // 初始化随机数生成器
    std::random_device rd;
    state.rng = std::mt19937(rd());
    
    if (isServer) {
        // 服务器生成迷宫
        int radius = input_grid_size();
        std::vector<HexCoord> boundary_cells;
        generate_hex_grid(radius, state.gridData, boundary_cells);
        select_start_end(radius, boundary_cells, state.gridData, state.startHex, state.endHex, state.rng);
        
        // 初始化路径
        std::unordered_map<HexCoord, HexCoord> came_from;
        std::unordered_set<HexCoord> visited;
        std::vector<HexCoord> path_cells;
        findPath(state.startHex, state.endHex, state.gridData, came_from, visited, state.rng);
        reconstruct_path(state.startHex, state.endHex, state.gridData, came_from, visited, path_cells);
        
        // 创建类似于原HexGrid中的pathCells_B来标记Path类型格子
        std::vector<HexCoord> path_cells_B;
        std::vector<HexCoord> path_cells_A; // A路径用于玩家检查点
        
        // 将路径从终点方向向起点标记（与原HexGrid的逻辑类似）
        int left = 0;  // 从终点开始
        int right = path_cells.size() - 1; // 从起点开始
        
        // 分别标记A路径和B路径
        state.gridNumbers.clear(); // 清空标号
        
        // 标记起点序号
        state.gridNumbers[state.startHex].push_back(1);
        
        // 从两端标记（类似HexGrid中的逻辑）
        int startIndex = 1;
        int endIndex = 1;
        
        while (left <= right) {
            // 标记B路径（从终点到起点）
            path_cells_B.push_back(path_cells[left]);
            state.gridNumbers[path_cells[left]].push_back(startIndex);
            startIndex++;
            left++;
            
            // 标记A路径（从起点到终点）
            if (right >= 0) {
                path_cells_A.push_back(path_cells[right]);
                right--;
            }
        }
        
        // 标记B路径上的格子为Path类型
        for (const auto& cell : path_cells_B) {
            if (cell != state.endHex && cell != state.startHex) {
                state.gridData[cell] = CellState::Path;
            }
        }
        
        // 计算相对路径
        std::vector<HexCoord> relative_path_cells;
        calculateHexRelative(state.startHex, state.endHex, path_cells, relative_path_cells, state.gridData);
        
        // 标记相对路径标号
        for (size_t i = 0; i < relative_path_cells.size(); ++i) {
            state.gridNumbers[relative_path_cells[i]].push_back(i + 2);
        }
        
        state.playerPos = state.startHex;
        state.otherPlayerPos = state.startHex;
    }
}

// 处理网络消息
void handleNetworkMessages(GameState& state, NetworkManager& networkManager) {
    NetworkMessage msg;
    while (networkManager.receiveMessage(msg)) {
        switch (msg.type) {
            case MessageType::GridData:
                if (!state.isServer) {
                    HexCoord startHex, endHex;
                    networkManager.deserializeGridData(msg.data, state.gridData, startHex, endHex);
                    state.startHex = startHex;
                    state.endHex = endHex;
                    state.gameStarted = true;
                }
                break;
                
            case MessageType::PlayerPosition:
                if (!state.isServer) {
                    int q, r;
                    std::memcpy(&q, msg.data.data(), sizeof(int));
                    std::memcpy(&r, msg.data.data() + sizeof(int), sizeof(int));
                    state.otherPlayerPos = HexCoord(q, r);
                }
                break;
                
            case MessageType::GameState:
                if (!state.isServer) {
                    size_t offset = 0;
                    bool gameStarted;
                    std::memcpy(&gameStarted, msg.data.data() + offset, sizeof(bool));
                    offset += sizeof(bool);
                    
                    float endTime;
                    std::memcpy(&endTime, msg.data.data() + offset, sizeof(float));
                    offset += sizeof(float);
                    
                    bool gameEnded;
                    std::memcpy(&gameEnded, msg.data.data() + offset, sizeof(bool));
                    
                    state.gameTime = endTime;
                    state.gameEnded = gameEnded;
                }
                break;
                
            case MessageType::GridNumbers:
                if (!state.isServer) {
                    networkManager.deserializeGridNumbers(msg.data, state.gridNumbers);
                }
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    bool isServer;
    std::string serverIP;
    unsigned short port;
    
    if (!parseArguments(argc, argv, isServer, serverIP, port)) {
        return 1;
    }
    
    // 初始化网络
    NetworkManager& networkManager = NetworkManager::getInstance();
    if (isServer) {
        if (!networkManager.initializeServer(port)) {
            std::cerr << "Failed to initialize server" << std::endl;
            return 1;
        }
    }
    else {
        if (!networkManager.initializeClient(serverIP, port)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return 1;
        }
    }
    
    // 初始化游戏状态
    GameState state;
    initializeGame(state, isServer);
    
    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Hex Grid Game");
    window.setFramerateLimit(60);
    
    // 计算网格原点位置（居中）
    sf::Vector2f grid_origin(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    
    // 游戏主循环
    sf::Clock gameClock;
    while (window.isOpen()) {
        // 处理事件
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::KeyPressed && !state.gameEnded) {
                Direction dir;
                switch (event.key.code) {
                    case sf::Keyboard::A: dir = Direction::Left; break;
                    case sf::Keyboard::D: dir = Direction::Right; break;
                    case sf::Keyboard::W: dir = Direction::UpLeft; break;
                    case sf::Keyboard::X: dir = Direction::DownRight; break;
                    case sf::Keyboard::E: dir = Direction::UpRight; break;
                    case sf::Keyboard::Z: dir = Direction::DownLeft; break;
                    default: continue;
                }
                
                if (move_player(dir, state.playerPos, state.gridData)) {
                    // 发送位置更新
                    networkManager.sendPlayerPosition(state.playerPos);
                    
                    // 检查是否到达终点
                    if (check_win(state.playerPos, state.endHex)) {
                        state.gameEnded = true;
                        float endTime = gameClock.getElapsedTime().asSeconds();
                        networkManager.sendGameEndTime(endTime);
                        std::cout << "You win! Time: " << endTime << " seconds" << std::endl;
                    }
                }
            }
        }
        
        // 处理网络消息
        handleNetworkMessages(state, networkManager);
        
        // 如果是服务器且游戏已开始，发送网格数据
        if (state.isServer && !state.gameStarted) {
            networkManager.sendGridData(state.gridData, state.startHex, state.endHex);
            networkManager.sendGridNumbers(state.gridNumbers);  // 发送格子标号数据
            state.gameStarted = true;
        }
        
        // 更新游戏时间
        if (!state.gameEnded) {
            state.gameTime = gameClock.getElapsedTime().asSeconds();
        }
        
        // 渲染
        window.clear(sf::Color::White);
        
        // 渲染带标号的网格（替代原来的render_grid调用）
        render_grid_with_numbers(window, grid_origin, state.gridData, state.gridNumbers);
        
        // 渲染玩家位置
        sf::CircleShape playerShape(HEX_SIZE * 0.8f);
        playerShape.setFillColor(sf::Color::Blue);
        sf::Vector2f playerPos = axial_to_pixel(state.playerPos, HEX_SIZE, grid_origin);
        playerShape.setPosition(playerPos - sf::Vector2f(HEX_SIZE * 0.8f, HEX_SIZE * 0.8f));
        window.draw(playerShape);
        
        // 渲染其他玩家位置
        sf::CircleShape otherPlayerShape(HEX_SIZE * 0.8f);
        otherPlayerShape.setFillColor(sf::Color::Green);
        sf::Vector2f otherPlayerPos = axial_to_pixel(state.otherPlayerPos, HEX_SIZE, grid_origin);
        otherPlayerShape.setPosition(otherPlayerPos - sf::Vector2f(HEX_SIZE * 0.8f, HEX_SIZE * 0.8f));
        window.draw(otherPlayerShape);
        
        // 渲染时间
        sf::Font font;
        if (font.loadFromFile("arial.ttf")) {
            sf::Text timeText;
            timeText.setFont(font);
            timeText.setCharacterSize(24);
            timeText.setFillColor(sf::Color::Black);
            timeText.setString("Time: " + std::to_string(static_cast<int>(state.gameTime)) + "s");
            timeText.setPosition(10, 10);
            window.draw(timeText);
        }
        
        window.display();
    }
    
    // 清理网络连接
    networkManager.close();
    
    return 0;
} 