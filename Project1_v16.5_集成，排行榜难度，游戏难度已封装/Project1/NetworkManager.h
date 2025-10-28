#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <functional>
#include "SingleHex.h"

// 消息类型
enum class MessageType : sf::Uint8 {
    GridData,
    PlayerPosition,
    GameState,
    GridNumbers,  // 添加格子标号消息类型
    CheckpointData  // 添加检查点数据消息类型
};

// 网络消息结构
struct NetworkMessage {
    MessageType type;
    std::vector<uint8_t> data;
};

class NetworkManager {
public:
    static NetworkManager& getInstance();
    
    // 初始化服务器
    bool initializeServer(unsigned short port);
    
    // 初始化客户端
    bool initializeClient(const std::string& serverIP, unsigned short port);
    
    // 发送网格数据
    bool sendGridData(const std::unordered_map<HexCoord, CellState>& gridData, 
                     const HexCoord& startHex, 
                     const HexCoord& endHex);
    
    // 发送玩家位置
    bool sendPlayerPosition(const HexCoord& pos);
    
    // 发送游戏结束时间
    bool sendGameEndTime(float time);
    
    // 发送检查点数据
    bool sendCheckpointData(int checkA, int checkB, int pathSize);
    
    // 接收消息
    bool receiveMessage(NetworkMessage& msg);
    
    // 关闭连接
    void close();

    // 新增：发送格子标号数据
    void sendGridNumbers(const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers);

    // 反序列化网格数据
    bool deserializeGridData(const std::vector<uint8_t>& data, 
                            std::unordered_map<HexCoord, CellState>& gridData, 
                            HexCoord& startHex, 
                            HexCoord& endHex);

    // 新增：反序列化格子标号数据
    void deserializeGridNumbers(const std::vector<uint8_t>& data,
        std::unordered_map<HexCoord, std::vector<int>>& gridNumbers);

    // 新增：接收数据的回调函数
    using GridDataCallback = std::function<void(const std::unordered_map<HexCoord, CellState>&, 
                                             const HexCoord&, 
                                             const HexCoord&)>;
    using PlayerPositionCallback = std::function<void(const HexCoord&)>;
    using GameStateCallback = std::function<void(bool, float, bool)>;
    using GridNumbersCallback = std::function<void(const std::unordered_map<HexCoord, std::vector<int>>&)>; // 添加格子标号回调
    using CheckpointDataCallback = std::function<void(int, int, int)>; // 添加检查点数据回调(checkA, checkB, pathSize)

    void setGridDataCallback(GridDataCallback callback);
    void setPlayerPositionCallback(PlayerPositionCallback callback);
    void setGameStateCallback(GameStateCallback callback);
    void setGridNumbersCallback(GridNumbersCallback callback); // 设置格子标号回调
    void setCheckpointDataCallback(CheckpointDataCallback callback); // 设置检查点数据回调
    void setClientConnectedCallback(std::function<void()> callback);

    bool isServer() const;
    bool hasClientConnected() const;

    // 客户端相关
    sf::TcpSocket serverSocket;

    //服务器相关
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

private:
    NetworkManager() : _isServer(false), isRunning(false) {} // 初始化成员变量
    ~NetworkManager();
    
    // 禁止拷贝和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    // 标识是服务器还是客户端
    bool _isServer;
    
    // 服务器相关
    sf::TcpListener listener;
    
    // 消息队列
    std::queue<NetworkMessage> messageQueue;
    std::mutex queueMutex;
    
    // 网络线程
    std::thread networkThread;
    bool isRunning;
    
    // 序列化和反序列化函数
    std::vector<uint8_t> serializeGridData(const std::unordered_map<HexCoord, CellState>& gridData, 
                                            const HexCoord& startHex, 
                                            const HexCoord& endHex);
    
    // 新增：序列化格子标号数据
    std::vector<uint8_t> serializeGridNumbers(const std::unordered_map<HexCoord, std::vector<int>>& gridNumbers);
    
    // 处理消息回调
    void handleMessageCallback(const NetworkMessage& msg);
    
    // 网络线程函数
    void serverThread();
    void clientThread();

    // 新增：回调函数
    GridDataCallback gridDataCallback;
    PlayerPositionCallback playerPositionCallback;
    GameStateCallback gameStateCallback;
    GridNumbersCallback gridNumbersCallback; // 添加格子标号回调
    CheckpointDataCallback checkpointDataCallback; // 添加检查点数据回调
    std::function<void()> clientConnectedCallback;
}; 