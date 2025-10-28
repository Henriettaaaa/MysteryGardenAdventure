#include "NetworkManager.h"
#include <iostream>

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::~NetworkManager() {
    close();
}

bool NetworkManager::initializeServer(unsigned short port) {
    if (listener.listen(port) != sf::Socket::Done) {
        std::cerr << "Failed to listen on port " << port << std::endl;
        return false;
    }
    
    isRunning = true;
    networkThread = std::thread(&NetworkManager::serverThread, this);
    return true;
}

bool NetworkManager::initializeClient(const std::string& serverIP, unsigned short port) {
    if (serverSocket.connect(serverIP, port) != sf::Socket::Done) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }
    
    isRunning = true;
    networkThread = std::thread(&NetworkManager::clientThread, this);
    return true;
}

void NetworkManager::serverThread() {
    while (isRunning) {
        // 接受新的客户端连接
        auto clientSocket = std::make_unique<sf::TcpSocket>();
        if (listener.accept(*clientSocket) == sf::Socket::Done) {
            clients.push_back(std::move(clientSocket));
            std::cout << "New client connected" << std::endl;
            
            // 调用客户端连接回调
            if (clientConnectedCallback) {
                clientConnectedCallback();
            }
        }
        
        // 处理现有客户端的消息
        for (auto it = clients.begin(); it != clients.end();) {
            sf::Packet packet;
            if ((*it)->receive(packet) == sf::Socket::Done) {
                NetworkMessage msg;
                sf::Uint8 typeValue;
                packet >> typeValue;
                msg.type = static_cast<MessageType>(typeValue);
                
                sf::Uint32 dataSize;
                packet >> dataSize;
                msg.data.resize(dataSize);
                for (sf::Uint32 i = 0; i < dataSize; ++i) {
                    sf::Uint8 byte;
                    packet >> byte;
                    msg.data[i] = byte;
                }
                
                // 处理回调
                handleMessageCallback(msg);
                
                std::lock_guard<std::mutex> lock(queueMutex);
                messageQueue.push(msg);
            }
            else if ((*it)->getRemoteAddress() == sf::IpAddress::None) {
                it = clients.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void NetworkManager::clientThread() {
    while (isRunning) {
        sf::Packet packet;
        if (serverSocket.receive(packet) == sf::Socket::Done) {
            NetworkMessage msg;
            sf::Uint8 typeValue;
            packet >> typeValue;
            msg.type = static_cast<MessageType>(typeValue);
            
            sf::Uint32 dataSize;
            packet >> dataSize;
            msg.data.resize(dataSize);
            for (sf::Uint32 i = 0; i < dataSize; ++i) {
                sf::Uint8 byte;
                packet >> byte;
                msg.data[i] = byte;
            }
            
            // 处理回调
            handleMessageCallback(msg);
            
            std::lock_guard<std::mutex> lock(queueMutex);
            messageQueue.push(msg);
        }
    }
}

// 处理消息回调
void NetworkManager::handleMessageCallback(const NetworkMessage& msg) {
    switch (msg.type) {
        case MessageType::GridData:
            if (gridDataCallback) {
                std::unordered_map<HexCoord, CellState> gridData;
                HexCoord startHex, endHex;
                if (deserializeGridData(msg.data, gridData, startHex, endHex)) {
                    gridDataCallback(gridData, startHex, endHex);
                }
            }
            break;
        case MessageType::PlayerPosition:
            if (playerPositionCallback && msg.data.size() >= sizeof(int) * 2) {
                int q, r;
                std::memcpy(&q, msg.data.data(), sizeof(int));
                std::memcpy(&r, msg.data.data() + sizeof(int), sizeof(int));
                playerPositionCallback(HexCoord(q, r));
            }
            break;
        case MessageType::GameState:
            if (gameStateCallback && msg.data.size() >= sizeof(bool) + sizeof(float) + sizeof(bool)) {
                bool gameStarted;
                float gameTime;
                bool gameEnded;
                size_t offset = 0;
                
                std::memcpy(&gameStarted, msg.data.data() + offset, sizeof(bool));
                offset += sizeof(bool);
                
                std::memcpy(&gameTime, msg.data.data() + offset, sizeof(float));
                offset += sizeof(float);
                
                std::memcpy(&gameEnded, msg.data.data() + offset, sizeof(bool));
                
                gameStateCallback(gameStarted, gameTime, gameEnded);
            }
            break;
        case MessageType::GridNumbers:
            if (gridNumbersCallback) {
                std::unordered_map<HexCoord, std::vector<int>> gridNumbers;
                deserializeGridNumbers(msg.data, gridNumbers);
                gridNumbersCallback(gridNumbers);
            }
            break;
    }
}

bool NetworkManager::sendGridData(const std::unordered_map<HexCoord, CellState>& gridData, 
                                const HexCoord& startHex, 
                                const HexCoord& endHex) {
    std::vector<uint8_t> serializedData = serializeGridData(gridData, startHex, endHex);
    
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GridData);
    packet << static_cast<sf::Uint32>(serializedData.size());
    for (auto byte : serializedData) {
        packet << byte;
    }
    
    if (clients.empty()) {
        return serverSocket.send(packet) == sf::Socket::Done;
    }
    
    bool success = true;
    for (auto& client : clients) {
        if (client->send(packet) != sf::Socket::Done) {
            success = false;
        }
    }
    return success;
}

bool NetworkManager::sendPlayerPosition(const HexCoord& pos) {
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::PlayerPosition);
    
    // 序列化位置数据
    std::vector<uint8_t> data;
    data.reserve(sizeof(int) * 2);
    
    int q = pos.q;
    int r = pos.r;
    
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&q),
               reinterpret_cast<uint8_t*>(&q) + sizeof(int));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&r),
               reinterpret_cast<uint8_t*>(&r) + sizeof(int));
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    if (clients.empty()) {
        return serverSocket.send(packet) == sf::Socket::Done;
    }
    
    bool success = true;
    for (auto& client : clients) {
        if (client->send(packet) != sf::Socket::Done) {
            success = false;
        }
    }
    return success;
}

bool NetworkManager::sendGameEndTime(float time) {
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GameState);
    
    // 序列化游戏状态数据
    std::vector<uint8_t> data;
    data.reserve(sizeof(bool) + sizeof(float) + sizeof(bool));
    
    bool gameStarted = true;
    bool gameEnded = true;
    
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&gameStarted),
               reinterpret_cast<uint8_t*>(&gameStarted) + sizeof(bool));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&time),
               reinterpret_cast<uint8_t*>(&time) + sizeof(float));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gameEnded),
               reinterpret_cast<uint8_t*>(&gameEnded) + sizeof(bool));
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    if (clients.empty()) {
        return serverSocket.send(packet) == sf::Socket::Done;
    }
    
    bool success = true;
    for (auto& client : clients) {
        if (client->send(packet) != sf::Socket::Done) {
            success = false;
        }
    }
    return success;
}

// 实现发送格子标号数据的方法
void NetworkManager::sendGridNumbers(const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
    std::vector<uint8_t> serializedData = serializeGridNumbers(gridNumbers);
    
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GridNumbers);
    packet << static_cast<sf::Uint32>(serializedData.size());
    for (auto byte : serializedData) {
        packet << byte;
    }
    
    if (clients.empty()) {
        serverSocket.send(packet);
    } else {
        for (auto& client : clients) {
            client->send(packet);
        }
    }
}

bool NetworkManager::receiveMessage(NetworkMessage& msg) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (messageQueue.empty()) {
        return false;
    }
    
    msg = messageQueue.front();
    messageQueue.pop();
    return true;
}

void NetworkManager::close() {
    isRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }
    
    for (auto& client : clients) {
        client->disconnect();
    }
    clients.clear();
    
    if (serverSocket.getRemoteAddress() != sf::IpAddress::None) {
        serverSocket.disconnect();
    }
    
    listener.close();
}

std::vector<uint8_t> NetworkManager::serializeGridData(const std::unordered_map<HexCoord, CellState>& gridData,
                                                     const HexCoord& startHex,
                                                     const HexCoord& endHex) {
    std::vector<uint8_t> data;
    
    // 首先序列化起始和结束六边形
    int startQ = startHex.q;
    int startR = startHex.r;
    int endQ = endHex.q;
    int endR = endHex.r;
    
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&startQ),
               reinterpret_cast<uint8_t*>(&startQ) + sizeof(int));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&startR),
               reinterpret_cast<uint8_t*>(&startR) + sizeof(int));
    data.insert(data.end(), 
               reinterpret_cast<uint8_t*>(&endQ),
               reinterpret_cast<uint8_t*>(&endQ) + sizeof(int));
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&endR),
               reinterpret_cast<uint8_t*>(&endR) + sizeof(int));
    
    // 序列化格子数量
    sf::Uint32 gridSize = static_cast<sf::Uint32>(gridData.size());
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gridSize),
               reinterpret_cast<uint8_t*>(&gridSize) + sizeof(sf::Uint32));
    
    // 序列化每个格子数据
    for (const auto& pair : gridData) {
        // 序列化坐标
        int q = pair.first.q;
        int r = pair.first.r;
        data.insert(data.end(), 
                   reinterpret_cast<uint8_t*>(&q),
                   reinterpret_cast<uint8_t*>(&q) + sizeof(int));
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&r),
                   reinterpret_cast<uint8_t*>(&r) + sizeof(int));
        
        // 序列化状态
        CellState state = pair.second;
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&state),
                   reinterpret_cast<uint8_t*>(&state) + sizeof(CellState));
    }
    
    return data;
}

bool NetworkManager::deserializeGridData(const std::vector<uint8_t>& data, 
                                     std::unordered_map<HexCoord, CellState>& gridData,
                                     HexCoord& startHex,
                                     HexCoord& endHex) {
    if (data.size() < sizeof(int) * 4 + sizeof(sf::Uint32)) {
        return false;
    }
    
    size_t offset = 0;
    
    // 反序列化起始和结束六边形
    int startQ, startR, endQ, endR;
    std::memcpy(&startQ, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&startR, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&endQ, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&endR, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    
    startHex = HexCoord(startQ, startR);
    endHex = HexCoord(endQ, endR);
    
    // 反序列化格子数量
    sf::Uint32 gridSize;
    std::memcpy(&gridSize, data.data() + offset, sizeof(sf::Uint32));
    offset += sizeof(sf::Uint32);
    
    // 反序列化每个格子数据
    gridData.clear();
    for (sf::Uint32 i = 0; i < gridSize; ++i) {
        if (offset + sizeof(int) * 2 + sizeof(CellState) > data.size()) {
            return false;
        }
        
        // 反序列化坐标
        int q, r;
        std::memcpy(&q, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        std::memcpy(&r, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        
        // 反序列化状态
        CellState state;
        std::memcpy(&state, data.data() + offset, sizeof(CellState));
        offset += sizeof(CellState);
        
        gridData[HexCoord(q, r)] = state;
    }
    
    return true;
}

// 实现序列化格子标号数据
std::vector<uint8_t> NetworkManager::serializeGridNumbers(const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
    std::vector<uint8_t> data;
    
    // 序列化格子数量
    sf::Uint32 gridSize = static_cast<sf::Uint32>(gridNumbers.size());
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gridSize),
               reinterpret_cast<uint8_t*>(&gridSize) + sizeof(sf::Uint32));
    
    // 序列化每个格子数据
    for (const auto& pair : gridNumbers) {
        // 序列化坐标
        int q = pair.first.q;
        int r = pair.first.r;
        data.insert(data.end(), 
                   reinterpret_cast<uint8_t*>(&q),
                   reinterpret_cast<uint8_t*>(&q) + sizeof(int));
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&r),
                   reinterpret_cast<uint8_t*>(&r) + sizeof(int));
        
        // 序列化标号数量
        sf::Uint32 numbersSize = static_cast<sf::Uint32>(pair.second.size());
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&numbersSize),
                   reinterpret_cast<uint8_t*>(&numbersSize) + sizeof(sf::Uint32));
        
        // 序列化每个标号
        for (int number : pair.second) {
            data.insert(data.end(),
                       reinterpret_cast<uint8_t*>(&number),
                       reinterpret_cast<uint8_t*>(&number) + sizeof(int));
        }
    }
    
    return data;
}

// 实现反序列化格子标号数据
void NetworkManager::deserializeGridNumbers(const std::vector<uint8_t>& data, 
                                         std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
    if (data.size() < sizeof(sf::Uint32)) {
        return;
    }
    
    size_t offset = 0;
    
    // 反序列化格子数量
    sf::Uint32 gridSize;
    std::memcpy(&gridSize, data.data() + offset, sizeof(sf::Uint32));
    offset += sizeof(sf::Uint32);
    
    // 反序列化每个格子数据
    gridNumbers.clear();
    for (sf::Uint32 i = 0; i < gridSize; ++i) {
        if (offset + sizeof(int) * 2 + sizeof(sf::Uint32) > data.size()) {
            break;
        }
        
        // 反序列化坐标
        int q, r;
        std::memcpy(&q, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        std::memcpy(&r, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        
        // 反序列化标号数量
        sf::Uint32 numbersSize;
        std::memcpy(&numbersSize, data.data() + offset, sizeof(sf::Uint32));
        offset += sizeof(sf::Uint32);
        
        // 反序列化每个标号
        std::vector<int> numbers;
        for (sf::Uint32 j = 0; j < numbersSize; ++j) {
            if (offset + sizeof(int) > data.size()) {
                break;
            }
            
            int number;
            std::memcpy(&number, data.data() + offset, sizeof(int));
            offset += sizeof(int);
            
            numbers.push_back(number);
        }
        
        gridNumbers[HexCoord(q, r)] = numbers;
    }
}

// 实现设置回调函数
void NetworkManager::setGridDataCallback(GridDataCallback callback) {
    gridDataCallback = callback;
}

void NetworkManager::setPlayerPositionCallback(PlayerPositionCallback callback) {
    playerPositionCallback = callback;
}

void NetworkManager::setGameStateCallback(GameStateCallback callback) {
    gameStateCallback = callback;
}

void NetworkManager::setGridNumbersCallback(GridNumbersCallback callback) {
    gridNumbersCallback = callback;
}

void NetworkManager::setClientConnectedCallback(std::function<void()> callback) {
    clientConnectedCallback = callback;
}

bool NetworkManager::isServer() const {
    return !clients.empty();
}

bool NetworkManager::hasClientConnected() const {
    return !clients.empty();
} 