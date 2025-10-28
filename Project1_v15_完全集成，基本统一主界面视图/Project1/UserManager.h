#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <SFML/Graphics.hpp>

// 用户信息结构体
struct UserInfo {
    std::string username;
    std::string password;
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
    
    // 更新用户最佳时间
    void updateBestTime(int mazeSize, float time);
    
    // 获取排行榜数据
    std::vector<std::pair<std::string, float>> getLeaderboard(int mazeSize);
    
    // 退出登录
    void logout();
};

#endif // USER_MANAGER_H
