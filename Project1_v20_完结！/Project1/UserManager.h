#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <SFML/Graphics.hpp>

// 难度枚举
enum class Difficulty {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2
};

// 游戏模式枚举
enum class GameMode {
    SINGLE_SQUARE = 0,      // 单人正方形
    MULTI_SQUARE = 1,       // 双人正方形
    MULTI_HEX = 2          // 双人六边形
};

// 游戏数据结构
struct GameData {
    // 单人正方形：最佳时间（秒）
    std::map<Difficulty, float> singleSquareBestTimes;
    
    // 双人正方形：胜利次数和总游戏次数
    std::map<Difficulty, std::pair<int, int>> multiSquareStats;  // 胜利次数/总次数
    
    // 双人六边形：最佳时间和胜负统计
    std::map<Difficulty, float> multiHexBestTimes;               // 最佳时间（秒）
    std::map<Difficulty, std::pair<int, int>> multiHexStats;     // 胜利次数/总次数
};

// 用户信息结构体
struct UserInfo {
    std::string username;
    std::string password;
    GameData gameData;      // 新的游戏数据结构
    
    // 兼容性：保留旧的数据结构以便数据迁移
    std::map<int, float> bestTimes; // 迷宫大小 -> 最佳通关时间(秒)
};

// 用户管理类
class UserManager {
private:
    // 用户数据
    std::map<std::string, UserInfo> users;
    std::string currentUser; // 当前登录用户
    bool isLoggedIn; // 是否已登录
    
    // 用户数据文件路径
    std::string userDataFile;
    
    // SFML界面相关
    sf::RenderWindow window;
    sf::Font font;
    sf::Text titleText;
    sf::Text statusText;
    
    // 输入框
    sf::RectangleShape usernameBox;
    sf::RectangleShape passwordBox;
    sf::Text usernameLabel;
    sf::Text passwordLabel;
    sf::Text usernameText;
    sf::Text passwordText;
    std::string usernameInput;
    std::string passwordInput;
    bool focusUsername;
    bool focusPassword;
    
    // 按钮
    sf::RectangleShape loginButton;
    sf::RectangleShape registerButton;
    sf::RectangleShape backButton;
    sf::Text loginButtonText;
    sf::Text registerButtonText;
    sf::Text backButtonText;
    
    // 工具函数：难度和字符串转换
    std::string difficultyToString(Difficulty diff) const;
    Difficulty stringToDifficulty(const std::string& str) const;
    std::string gameModeToString(GameMode mode) const;
    GameMode stringToGameMode(const std::string& str) const;
    
    // 初始化界面
    void initWindow();
    void initFont();
    void initUI();
    
    // 加载和保存用户数据
    void loadUserData();
    void saveUserData();
    
    // 处理输入
    void handleTextInput(sf::Event& event);
    void handleMouseClick(sf::Vector2i mousePos);
    
    // 用户操作
    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);

public:
    UserManager();
    ~UserManager();
    
    // 运行登录界面
    void run();
    
    // 获取当前登录状态和用户
    bool isUserLoggedIn() const;
    std::string getCurrentUser() const;
    
    // 旧接口兼容性
    void updateBestTime(int mazeSize, float time);
    std::vector<std::pair<std::string, float>> getLeaderboard(int mazeSize);
    
    // 新的数据更新接口
    void updateSingleSquareBestTime(Difficulty diff, float time);
    void updateMultiSquareGameResult(Difficulty diff, bool won);
    void updateMultiHexBestTime(Difficulty diff, float time);
    void updateMultiHexGameResult(Difficulty diff, bool won);
    
    // 新的排行榜数据接口
    std::map<std::string, float> getSingleSquareLeaderboard(Difficulty diff);
    std::map<std::string, std::pair<int, int>> getMultiSquareLeaderboard(Difficulty diff);
    std::map<std::string, float> getMultiHexTimeLeaderboard(Difficulty diff);
    std::map<std::string, std::pair<int, int>> getMultiHexStatsLeaderboard(Difficulty diff);
    
    // 退出登录
    void logout();
    
    // 强制保存用户数据（用于确保数据完整性）
    void forceSaveData();
};

#endif // USER_MANAGER_H
