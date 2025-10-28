#include "NetworkManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstddef>

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::~NetworkManager() {
    close();
}

bool NetworkManager::initializeServer(unsigned short port) {
    // Ensure server socket is in blocking mode
    listener.setBlocking(true);
    
    if (listener.listen(port, sf::IpAddress::Any) != sf::Socket::Done) {
        std::cerr << "Failed to listen on port " << port << std::endl;
        return false;
    }
    
    std::cout << "Server started successfully, listening on port: " << port << std::endl;
    
    isRunning = true;
    _isServer = true;  // Set identifier as server
    networkThread = std::thread(&NetworkManager::serverThread, this);
    return true;
}

bool NetworkManager::initializeClient(const std::string& serverIP, unsigned short port) {
    // Ensure client socket is in blocking mode
    serverSocket.setBlocking(true);
    
    std::cout << "Client attempting to connect to server: " << serverIP << ":" << port << std::endl;
    
    // Use explicit IPv4 address
    sf::IpAddress ipAddress(serverIP);
    if (ipAddress == sf::IpAddress::None) {
        std::cerr << "Invalid server IP address: " << serverIP << std::endl;
        return false;
    }
    
    // Try to connect 3 times
    for (int attempt = 1; attempt <= 3; attempt++) {
        std::cout << "Connection attempt #" << attempt << "..." << std::endl;
        
        auto status = serverSocket.connect(ipAddress, port, sf::seconds(5));
        if (status == sf::Socket::Done) {
            std::cout << "Successfully connected to server!" << std::endl;
            
            isRunning = true;
            _isServer = false;  // Set identifier as client
            networkThread = std::thread(&NetworkManager::clientThread, this);
            return true;
        }
        
        // Connection failed, print error status
        std::string statusStr;
        switch (status) {
            case sf::Socket::NotReady: statusStr = "NotReady"; break;
            case sf::Socket::Partial: statusStr = "Partial"; break;
            case sf::Socket::Disconnected: statusStr = "Disconnected"; break;
            case sf::Socket::Error: statusStr = "Error"; break;
            default: statusStr = "Unknown error"; break;
        }
        std::cerr << "Connection failed, status: " << statusStr << " (" << static_cast<int>(status) << ")" << std::endl;
        
        // Short sleep before retry
        if (attempt < 3) {
            std::cout << "Retrying in 1 second..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    std::cerr << "Failed to connect to server after 3 attempts" << std::endl;
    return false;
}

void NetworkManager::serverThread() {
    // 设置监听器为非阻塞模式，这是解决问题的关键
    listener.setBlocking(false);
    std::cout << "Server thread started, waiting for connections..." << std::endl;
    
    while (isRunning) {
        // 尝试接受新的客户端连接（非阻塞）
        auto clientSocket = std::make_unique<sf::TcpSocket>();
        sf::Socket::Status acceptStatus = listener.accept(*clientSocket);
        
        if (acceptStatus == sf::Socket::Done) {
            // 确保客户端socket是阻塞模式，接收消息时不会立即返回NotReady
            clientSocket->setBlocking(true);
            clients.push_back(std::move(clientSocket));
            std::cout << "New client connected, total clients: " << clients.size() << std::endl;
            
            // 调用客户端连接回调
            if (clientConnectedCallback) {
                clientConnectedCallback();
            }
        } 
        
        // 处理现有客户端的消息 - 独立于新连接接受逻辑
        for (auto it = clients.begin(); it != clients.end();) {
            // 验证客户端socket有效性
            if (!(*it) || (*it)->getRemoteAddress() == sf::IpAddress::None) {
                std::cout << "Invalid client, removing" << std::endl;
                it = clients.erase(it);
                continue;
            }
            
            // 确保socket处于阻塞模式
            (*it)->setBlocking(true);
            
            sf::Packet packet;
            auto status = (*it)->receive(packet);
            
            if (status == sf::Socket::Done) {
                std::cout << "Server received packet, size: " << packet.getDataSize() << " bytes" << std::endl;
                
                // 解析消息类型
                sf::Uint8 typeValue;
                packet >> typeValue;
                MessageType msgType = static_cast<MessageType>(typeValue);
                
                std::cout << "Message type: " << static_cast<int>(msgType) << std::endl;
                
                sf::Uint32 dataSize;
                packet >> dataSize;
                
                std::cout << "Data size: " << dataSize << std::endl;
                
                std::vector<uint8_t> data;
                data.resize(dataSize);
                
                for (sf::Uint32 i = 0; i < dataSize; ++i) {
                    sf::Uint8 byte;
                    packet >> byte;
                    data[i] = byte;
                }
                
                // 处理玩家位置消息
                if (msgType == MessageType::PlayerPosition && dataSize >= sizeof(int) * 2) {
                    int q, r;
                    std::memcpy(&q, data.data(), sizeof(int));
                    std::memcpy(&r, data.data() + sizeof(int), sizeof(int));
                    
                    std::cout << "Server received player position: (" << q << "," << r << ")" << std::endl;
                    
                    // 调用回调函数处理位置更新
                    if (playerPositionCallback) {
                        HexCoord playerPos(q, r);
                        std::cout << "Executing position callback" << std::endl;
                        playerPositionCallback(playerPos);
                    } else {
                        std::cout << "WARNING: Position callback not set!" << std::endl;
                    }
                    
                    // 添加到消息队列
                    NetworkMessage msg;
                    msg.type = msgType;
                    msg.data = std::move(data);
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(msg);
                    
                    // 广播位置给其他客户端
                    sf::Packet broadcastPacket;
                    broadcastPacket << static_cast<sf::Uint8>(MessageType::PlayerPosition);
                    std::vector<uint8_t> posData;
                    posData.resize(sizeof(int) * 2);
                    size_t offset = 0;
                    std::memcpy(posData.data() + offset, &q, sizeof(int));
                    offset += sizeof(int);
                    std::memcpy(posData.data() + offset, &r, sizeof(int));
                    broadcastPacket << static_cast<sf::Uint32>(posData.size());
                    for (auto byte : posData) {
                        broadcastPacket << byte;
                    }
                    
                    // 向所有客户端广播，除了发送者
                    for (auto& otherClient : clients) {
                        if (otherClient.get() != (*it).get()) {
                            std::cout << "Broadcasting position to other clients" << std::endl;
                            otherClient->send(broadcastPacket);
                        }
                    }
                }
                // 其他类型的消息
                else {
                    NetworkMessage msg;
                    msg.type = msgType;
                    msg.data = std::move(data);
                    
                    // 处理回调
                    handleMessageCallback(msg);
                    
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(msg);
                }
            }
            else if (status == sf::Socket::Disconnected) {
                std::cout << "Client disconnected" << std::endl;
                it = clients.erase(it);
                continue;
            }
            else if (status == sf::Socket::NotReady) {
                // 这种状态在非阻塞模式下是正常的，但我们设置了阻塞模式，所以不应该出现
                ++it;
                continue;
            }
            else {
                std::cout << "Receive error, status code: " << static_cast<int>(status) << std::endl;
            }
            ++it;
        }
        
        // 添加短暂的休眠以避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void NetworkManager::clientThread() {
    std::cout << "Client thread started, connecting to server..." << std::endl;
    
    // 确保客户端socket是阻塞模式
    serverSocket.setBlocking(true);
    
    while (isRunning) {
        if (serverSocket.getRemoteAddress() == sf::IpAddress::None) {
            std::cout << "Client connection to server lost" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        sf::Packet packet;
        auto status = serverSocket.receive(packet);
        
        // Print receive status
        std::cout << "Client receive status: ";
        switch(status) {
            case sf::Socket::Done: std::cout << "Done"; break;
            case sf::Socket::NotReady: std::cout << "NotReady"; break;
            case sf::Socket::Partial: std::cout << "Partial"; break;
            case sf::Socket::Disconnected: std::cout << "Disconnected"; break;
            case sf::Socket::Error: std::cout << "Error"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << " (" << static_cast<int>(status) << ")" << std::endl;
        
        if (status == sf::Socket::Done) {
            std::cout << "Client received packet, size: " << packet.getDataSize() << " bytes" << std::endl;
            
            // Print raw packet data
            std::cout << "Raw packet data (HEX): ";
            const void* packetData = packet.getData();
            if (packetData && packet.getDataSize() > 0) {
                const uint8_t* rawData = static_cast<const uint8_t*>(packetData);
                for (size_t i = 0; i < std::min<size_t>(packet.getDataSize(), 32); i++) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                            << static_cast<int>(rawData[i]) << " ";
                }
            } else {
                std::cout << "EMPTY PACKET";
            }
            std::cout << std::dec << std::endl;
            
            try {
                // Parse message type
                sf::Uint8 typeValue = 0;
                if (!(packet >> typeValue)) {
                    std::cout << "ERROR: Failed to extract message type" << std::endl;
                    continue;
                }
                
                MessageType msgType = static_cast<MessageType>(typeValue);
                std::cout << "Message type: " << static_cast<int>(msgType) << std::endl;
                
                sf::Uint32 dataSize = 0;
                if (!(packet >> dataSize)) {
                    std::cout << "ERROR: Failed to extract data size" << std::endl;
                    continue;
                }
                
                std::cout << "Data size: " << dataSize << std::endl;
                
                // 检查数据大小是否合理
                if (dataSize > 1024 * 10) {
                    std::cout << "WARNING: Data size suspiciously large: " << dataSize << ", ignoring packet" << std::endl;
                    continue;
                }
                
                std::vector<uint8_t> data;
                data.resize(dataSize);
                
                bool dataExtracted = true;
                for (sf::Uint32 i = 0; i < dataSize; ++i) {
                    sf::Uint8 byte;
                    if (!(packet >> byte)) {
                        std::cout << "ERROR: Failed to extract data byte at position " << i << std::endl;
                        dataExtracted = false;
                        break;
                    }
                    data[i] = byte;
                }
                
                if (!dataExtracted) {
                    std::cout << "ERROR: Failed to extract complete data - packet corrupted" << std::endl;
                    continue;
                }
                
                // Print packet data content
                std::cout << "Parsed data content (HEX): ";
                for (size_t i = 0; i < std::min<size_t>(dataSize, 16); i++) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                            << static_cast<int>(data[i]) << " ";
                }
                std::cout << std::dec << std::endl;
                
                // Process player position message
                if (msgType == MessageType::PlayerPosition && dataSize >= sizeof(int) * 2) {
                    int q = 0, r = 0;
                    std::memcpy(&q, data.data(), sizeof(int));
                    std::memcpy(&r, data.data() + sizeof(int), sizeof(int));
                    
                    std::cout << "Client received player position: (" << q << "," << r << ")" << std::endl;
                    
                    // Directly call position callback
                    if (playerPositionCallback) {
                        HexCoord playerPos(q, r);
                        std::cout << "Client executing position callback" << std::endl;
                        playerPositionCallback(playerPos);
                    } else {
                        std::cout << "WARNING: Client position callback not set" << std::endl;
                    }
                }
                
                // Create message and trigger callback
                NetworkMessage msg;
                msg.type = msgType;
                msg.data = std::move(data);
                
                // Process callback
                handleMessageCallback(msg);
                
                // Add to message queue
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(msg);
                }
            } catch (const std::exception& e) {
                std::cout << "Exception while processing packet: " << e.what() << std::endl;
            }
        }
        else if (status == sf::Socket::Disconnected) {
            std::cout << "Disconnected from server" << std::endl;
            break;
        }
        else if (status == sf::Socket::NotReady) {
            // 不应该发生，因为socket是阻塞模式
            std::cout << "WARNING: Received NotReady status while in blocking mode" << std::endl;
        }
        else {
            std::cout << "Client receive error, status code: " << static_cast<int>(status) << std::endl;
        }
        
        // Add short sleep to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Process message callback
void NetworkManager::handleMessageCallback(const NetworkMessage& msg) {
    std::cout << "handleMessageCallback - Processing message type: " << static_cast<int>(msg.type) << std::endl;
    
    // Print message content
    std::cout << "Message data content (HEX): ";
    for (size_t i = 0; i < std::min<size_t>(msg.data.size(), 16); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(msg.data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    switch (msg.type) {
        case MessageType::GridData:
            if (gridDataCallback) {
                std::unordered_map<HexCoord, CellState> gridData;
                HexCoord startHex, endHex;
                int radius;
                if (deserializeGridData(msg.data, gridData, startHex, endHex, radius)) {
                    std::cout << "Processing grid data, start: (" << startHex.q << "," << startHex.r 
                              << "), end: (" << endHex.q << "," << endHex.r << "), radius: " << radius << std::endl;
                    gridDataCallback(gridData, startHex, endHex, radius);
                } else {
                    std::cout << "Grid data deserialization failed" << std::endl;
                }
            } else {
                std::cout << "Grid data callback not set" << std::endl;
            }
            break;
        case MessageType::PlayerPosition:
            if (playerPositionCallback && msg.data.size() >= sizeof(int) * 2) {
                int q, r;
                std::memcpy(&q, msg.data.data(), sizeof(int));
                std::memcpy(&r, msg.data.data() + sizeof(int), sizeof(int));
                HexCoord playerPos(q, r);
                
                std::cout << "handleMessageCallback - Processing position message: (" << q << "," << r << ")" << std::endl;
                
                // Always call position callback, whether server or client
                playerPositionCallback(playerPos);
                
                // If server, also forward position to other clients
                if (isServer()) {
                    std::cout << "Server forwarding position to other clients" << std::endl;
                    // Create position data packet
                    sf::Packet posPacket;
                    posPacket << static_cast<sf::Uint8>(MessageType::PlayerPosition);
                    posPacket << static_cast<sf::Uint32>(msg.data.size());
                    for (auto byte : msg.data) {
                        posPacket << byte;
                    }
                    
                    // Broadcast to all clients
                    for (auto& client : clients) {
                        client->send(posPacket);
                    }
                }
            } else {
                std::cout << "handleMessageCallback - Position callback not set or data format error" << std::endl;
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
                
                std::cout << "Processing game state: Game started=" << (gameStarted ? "Yes" : "No") 
                          << ", time=" << gameTime 
                          << ", game ended=" << (gameEnded ? "Yes" : "No") << std::endl;
                
                gameStateCallback(gameStarted, gameTime, gameEnded);
            } else {
                std::cout << "Game state callback not set or data format error" << std::endl;
            }
            break;
        case MessageType::GridNumbers:
            if (gridNumbersCallback) {
                std::unordered_map<HexCoord, std::vector<int>> gridNumbers;
                deserializeGridNumbers(msg.data, gridNumbers);
                std::cout << "Processing grid number data, number count: " << gridNumbers.size() << std::endl;
                gridNumbersCallback(gridNumbers);
            } else {
                std::cout << "Grid number callback not set" << std::endl;
            }
            break;
        case MessageType::CheckpointData:
            if (checkpointDataCallback && msg.data.size() >= sizeof(int) * 3) {
                int checkA, checkB, pathSize;
                std::memcpy(&checkA, msg.data.data(), sizeof(int));
                std::memcpy(&checkB, msg.data.data() + sizeof(int), sizeof(int));
                std::memcpy(&pathSize, msg.data.data() + sizeof(int) * 2, sizeof(int));
                
                checkpointDataCallback(checkA, checkB, pathSize);
            } else {
                std::cout << "Checkpoint data callback not set or data format error" << std::endl;
            }
            break;
        default:
            std::cout << "Unknown message type: " << static_cast<int>(msg.type) << std::endl;
            break;
    }
}

bool NetworkManager::sendGridData(const std::unordered_map<HexCoord, CellState>& gridData, 
                                const HexCoord& startHex, 
                                const HexCoord& endHex,
                                int radius) {
    std::vector<uint8_t> serializedData = serializeGridData(gridData, startHex, endHex, radius);
    
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
    std::cout << "Preparing to send player position: (" << pos.q << "," << pos.r << ")" << std::endl;
    
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::PlayerPosition);
    
    // Serialize position data
    std::vector<uint8_t> data;
    data.resize(sizeof(int) * 2);
    
    int q = pos.q;
    int r = pos.r;
    
    // Use memcpy for safer data serialization
    size_t offset = 0;
    std::memcpy(data.data() + offset, &q, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &r, sizeof(int));
    
    // Print data content
    std::cout << "Position data content (HEX): ";
    for (size_t i = 0; i < data.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    // Print full packet data
    std::cout << "Full packet data (HEX): ";
    const void* packetData = packet.getData();
    if (packetData) {
        const uint8_t* rawData = static_cast<const uint8_t*>(packetData);
        for (size_t i = 0; i < std::min<size_t>(packet.getDataSize(), 32); i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(rawData[i]) << " ";
        }
    }
    std::cout << std::dec << std::endl;
    
    try {
        if (isServer()) {
            bool success = true;
            int successCount = 0;
            
            if (clients.empty()) {
                std::cout << "No clients connected to server, position update not sent" << std::endl;
                return false;
            }
            
            for (auto& client : clients) {
                // Ensure client socket is in blocking mode
                client->setBlocking(true);
                
                if (client->getRemoteAddress() == sf::IpAddress::None) {
                    std::cout << "Client socket is disconnected, skipping" << std::endl;
                    continue;
                }
                
                auto sendStatus = client->send(packet);
                if (sendStatus != sf::Socket::Done) {
                    success = false;
                    std::cout << "Server sending position to client failed, status code: " << static_cast<int>(sendStatus) << std::endl;
                } else {
                    std::cout << "Server successfully sent position to client: (" << pos.q << "," << pos.r << ")" << std::endl;
                    successCount++;
                }
            }
            
            return successCount > 0; // At least one client received the update
        } else {
            // Client sending to server
            std::cout << "Client preparing to send position to server: (" << pos.q << "," << pos.r << ")" << std::endl;
            
            // Check connection status
            if (serverSocket.getRemoteAddress() == sf::IpAddress::None) {
                std::cout << "Error: Client server connection is disconnected" << std::endl;
                return false;
            }
            
            // Ensure server socket is in blocking mode
            serverSocket.setBlocking(true);
            
            // Force send with 3 attempts
            for (int attempt = 0; attempt < 3; attempt++) {
                sf::Socket::Status status = serverSocket.send(packet);
                if (status == sf::Socket::Done) {
                    std::cout << "Client successfully sent position to server, attempt count: " << (attempt + 1) << std::endl;
                    return true;
                } else {
                    std::string statusStr;
                    switch (status) {
                        case sf::Socket::NotReady: statusStr = "NotReady"; break;
                        case sf::Socket::Partial: statusStr = "Partial"; break;
                        case sf::Socket::Disconnected: statusStr = "Disconnected"; break;
                        case sf::Socket::Error: statusStr = "Error"; break;
                        default: statusStr = "Unknown"; break;
                    }
                    std::cout << "Client sending position failed, status: " << statusStr
                            << " (" << static_cast<int>(status) << "), attempt count: " << (attempt + 1) << std::endl;
                    
                    // Increase wait time with each attempt
                    std::this_thread::sleep_for(std::chrono::milliseconds(50 * (attempt + 1)));
                }
            }
            
            std::cout << "Client position sending failed, attempted 3 times" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception while sending position: " << e.what() << std::endl;
        return false;
    }
}

bool NetworkManager::sendGameEndTime(float time) {
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::GameState);
    
    // Serialize game state data
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

// Implementation of sending grid number data
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
    std::cout << "NetworkManager::close() called" << std::endl;
    
    // 停止运行标志
    isRunning = false;
    
    // 强制关闭socket以中断阻塞的网络操作
    try {
        // 先关闭监听器
        listener.close();
        std::cout << "Listener closed" << std::endl;
        
        // 断开服务器socket连接  
        if (serverSocket.getRemoteAddress() != sf::IpAddress::None) {
            std::cout << "Disconnecting server socket..." << std::endl;
            serverSocket.disconnect();
        }
        
        // 清理客户端连接
        for (auto& client : clients) {
            if (client) {
                client->disconnect();
            }
        }
        clients.clear();
        std::cout << "Client connections cleared" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception while closing sockets: " << e.what() << std::endl;
    }
    
    // 等待网络线程结束（在socket关闭后）
    if (networkThread.joinable()) {
        std::cout << "Waiting for network thread to join..." << std::endl;
        try {
            // 设置超时，避免无限等待
            auto future = std::async(std::launch::async, [this]() {
                this->networkThread.join();
            });
            
            if (future.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
                std::cout << "Network thread join timeout, force terminating..." << std::endl;
                // 注意：在生产代码中，强制终止线程是危险的，但这里是为了确保主界面不死机
                // 实际情况下，socket关闭后线程应该能正常结束
            } else {
                std::cout << "Network thread joined successfully" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Exception while joining network thread: " << e.what() << std::endl;
        }
    }
    
    // 重置所有状态，确保单例可以重新使用
    _isServer = false;
    
    // 清理回调函数
    playerPositionCallback = nullptr;
    gridDataCallback = nullptr;
    gameStateCallback = nullptr;
    gridNumbersCallback = nullptr;
    clientConnectedCallback = nullptr;
    checkpointDataCallback = nullptr;
    
    // 清空消息队列
    try {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!messageQueue.empty()) {
            messageQueue.pop();
        }
        std::cout << "Message queue cleared" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception while clearing message queue: " << e.what() << std::endl;
    }
    
    std::cout << "NetworkManager cleanup completed" << std::endl;
}

std::vector<uint8_t> NetworkManager::serializeGridData(const std::unordered_map<HexCoord, CellState>& gridData,
                                                     const HexCoord& startHex,
                                                     const HexCoord& endHex,
                                                     int radius) {
    std::vector<uint8_t> data;
    
    // First serialize start and end hexagons
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
    
    // Serialize radius
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&radius),
               reinterpret_cast<uint8_t*>(&radius) + sizeof(int));
    
    // Serialize cell count
    sf::Uint32 gridSize = static_cast<sf::Uint32>(gridData.size());
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gridSize),
               reinterpret_cast<uint8_t*>(&gridSize) + sizeof(sf::Uint32));
    
    // Serialize each cell data
    for (const auto& pair : gridData) {
        // Serialize coordinates
        int q = pair.first.q;
        int r = pair.first.r;
        data.insert(data.end(), 
                   reinterpret_cast<uint8_t*>(&q),
                   reinterpret_cast<uint8_t*>(&q) + sizeof(int));
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&r),
                   reinterpret_cast<uint8_t*>(&r) + sizeof(int));
        
        // Serialize state
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
                                     HexCoord& endHex,
                                     int& radius) {
    if (data.size() < sizeof(int) * 5 + sizeof(sf::Uint32)) {
        return false;
    }
    
    size_t offset = 0;
    
    // Deserialize start and end hexagons
    int startQ, startR, endQ, endR;
    std::memcpy(&startQ, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&startR, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&endQ, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    std::memcpy(&endR, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    
    // Deserialize radius
    std::memcpy(&radius, data.data() + offset, sizeof(int));
    offset += sizeof(int);
    
    startHex = HexCoord(startQ, startR);
    endHex = HexCoord(endQ, endR);
    
    // Deserialize cell count
    sf::Uint32 gridSize;
    std::memcpy(&gridSize, data.data() + offset, sizeof(sf::Uint32));
    offset += sizeof(sf::Uint32);
    
    // Deserialize each cell data
    gridData.clear();
    for (sf::Uint32 i = 0; i < gridSize; ++i) {
        if (offset + sizeof(int) * 2 + sizeof(CellState) > data.size()) {
            return false;
        }
        
        // Deserialize coordinates
        int q, r;
        std::memcpy(&q, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        std::memcpy(&r, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        
        // Deserialize state
        CellState state;
        std::memcpy(&state, data.data() + offset, sizeof(CellState));
        offset += sizeof(CellState);
        
        gridData[HexCoord(q, r)] = state;
    }
    
    return true;
}

// Implementation of serializing grid number data
std::vector<uint8_t> NetworkManager::serializeGridNumbers(const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
    std::vector<uint8_t> data;
    
    // Serialize cell count
    sf::Uint32 gridSize = static_cast<sf::Uint32>(gridNumbers.size());
    data.insert(data.end(),
               reinterpret_cast<uint8_t*>(&gridSize),
               reinterpret_cast<uint8_t*>(&gridSize) + sizeof(sf::Uint32));
    
    // Serialize each cell data
    for (const auto& pair : gridNumbers) {
        // Serialize coordinates
        int q = pair.first.q;
        int r = pair.first.r;
        data.insert(data.end(), 
                   reinterpret_cast<uint8_t*>(&q),
                   reinterpret_cast<uint8_t*>(&q) + sizeof(int));
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&r),
                   reinterpret_cast<uint8_t*>(&r) + sizeof(int));
        
        // Serialize number count
        sf::Uint32 numbersSize = static_cast<sf::Uint32>(pair.second.size());
        data.insert(data.end(),
                   reinterpret_cast<uint8_t*>(&numbersSize),
                   reinterpret_cast<uint8_t*>(&numbersSize) + sizeof(sf::Uint32));
        
        // Serialize each number
        for (int number : pair.second) {
            data.insert(data.end(),
                       reinterpret_cast<uint8_t*>(&number),
                       reinterpret_cast<uint8_t*>(&number) + sizeof(int));
        }
    }
    
    return data;
}

// Implementation of deserializing grid number data
void NetworkManager::deserializeGridNumbers(const std::vector<uint8_t>& data, 
                                         std::unordered_map<HexCoord, std::vector<int>>& gridNumbers) {
    if (data.size() < sizeof(sf::Uint32)) {
        return;
    }
    
    size_t offset = 0;
    
    // Deserialize cell count
    sf::Uint32 gridSize;
    std::memcpy(&gridSize, data.data() + offset, sizeof(sf::Uint32));
    offset += sizeof(sf::Uint32);
    
    // Deserialize each cell data
    gridNumbers.clear();
    for (sf::Uint32 i = 0; i < gridSize; ++i) {
        if (offset + sizeof(int) * 2 + sizeof(sf::Uint32) > data.size()) {
            break;
        }
        
        // Deserialize coordinates
        int q, r;
        std::memcpy(&q, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        std::memcpy(&r, data.data() + offset, sizeof(int));
        offset += sizeof(int);
        
        // Deserialize number count
        sf::Uint32 numbersSize;
        std::memcpy(&numbersSize, data.data() + offset, sizeof(sf::Uint32));
        offset += sizeof(sf::Uint32);
        
        // Deserialize each number
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

// Implementation of setting callback functions
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

void NetworkManager::setCheckpointDataCallback(CheckpointDataCallback callback) {
    checkpointDataCallback = callback;
}

bool NetworkManager::isServer() const {
    return _isServer;
}

bool NetworkManager::hasClientConnected() const {
    return !clients.empty();
}

bool NetworkManager::sendCheckpointData(int checkA, int checkB, int pathSize) {
    if (!_isServer || clients.empty()) {
        return false;
    }

    sf::Packet packet;
    packet << static_cast<sf::Uint8>(MessageType::CheckpointData);
    
    std::vector<uint8_t> data;
    data.resize(sizeof(int) * 3);
    
    size_t offset = 0;
    std::memcpy(data.data() + offset, &checkA, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &checkB, sizeof(int));
    offset += sizeof(int);
    std::memcpy(data.data() + offset, &pathSize, sizeof(int));
    
    packet << static_cast<sf::Uint32>(data.size());
    for (auto byte : data) {
        packet << byte;
    }
    
    bool allSent = true;
    for (auto& client : clients) {
        if (client->send(packet) != sf::Socket::Done) {
            allSent = false;
        }
    }
    
    return allSent;
} 