#include "UserManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

UserManager::UserManager() 
    : isLoggedIn(false), focusUsername(false), focusPassword(false), userDataFile("users.txt") {
    // 不再在构造函数中初始化窗口，避免启动时显示空白窗口
    // initWindow(); // 移到run()方法中
    initFont();
    initUI();
    
    // 加载用户数据
    loadUserData();
}

UserManager::~UserManager() {
    // 保存用户数据
    saveUserData();
    
    // 关闭窗口
    if (window.isOpen()) {
        window.close();
    }
}

void UserManager::initWindow() {
    window.create(sf::VideoMode(800, 600), "user login/register", sf::Style::Close);
    window.setFramerateLimit(60);
}

void UserManager::initFont() {
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }
}

void UserManager::initUI() {
    // 标题
    titleText.setFont(font);
    titleText.setString("user login/register");
    titleText.setCharacterSize(30);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(300, 50);
    
    // 状态文本
    statusText.setFont(font);
    statusText.setString("");
    statusText.setCharacterSize(18);
    statusText.setFillColor(sf::Color::Red);
    statusText.setPosition(300, 500);
    
    // 用户名输入框
    usernameBox.setSize(sf::Vector2f(300, 40));
    usernameBox.setFillColor(sf::Color(240, 240, 240));
    usernameBox.setOutlineThickness(2);
    usernameBox.setOutlineColor(sf::Color(200, 200, 200));
    usernameBox.setPosition(300, 150);
    
    usernameLabel.setFont(font);
    usernameLabel.setString("username:");
    usernameLabel.setCharacterSize(20);
    usernameLabel.setFillColor(sf::Color::White);
    usernameLabel.setPosition(200, 155);
    
    usernameText.setFont(font);
    usernameText.setCharacterSize(20);
    usernameText.setFillColor(sf::Color::Black);
    usernameText.setPosition(310, 155);
    
    // 密码输入框
    passwordBox.setSize(sf::Vector2f(300, 40));
    passwordBox.setFillColor(sf::Color(240, 240, 240));
    passwordBox.setOutlineThickness(2);
    passwordBox.setOutlineColor(sf::Color(200, 200, 200));
    passwordBox.setPosition(300, 220);
    
    passwordLabel.setFont(font);
    passwordLabel.setString("password:");
    passwordLabel.setCharacterSize(20);
    passwordLabel.setFillColor(sf::Color::White);
    passwordLabel.setPosition(200, 225);
    
    passwordText.setFont(font);
    passwordText.setCharacterSize(20);
    passwordText.setFillColor(sf::Color::Black);
    passwordText.setPosition(310, 225);
    
    // 登录按钮
    loginButton.setSize(sf::Vector2f(150, 50));
    loginButton.setFillColor(sf::Color(70, 70, 170));
    loginButton.setPosition(250, 300);
    
    loginButtonText.setFont(font);
    loginButtonText.setString("login");
    loginButtonText.setCharacterSize(20);
    loginButtonText.setFillColor(sf::Color::White);
    loginButtonText.setPosition(300, 310);
    
    // 注册按钮
    registerButton.setSize(sf::Vector2f(150, 50));
    registerButton.setFillColor(sf::Color(70, 70, 170));
    registerButton.setPosition(450, 300);
    
    registerButtonText.setFont(font);
    registerButtonText.setString("register");
    registerButtonText.setCharacterSize(20);
    registerButtonText.setFillColor(sf::Color::White);
    registerButtonText.setPosition(500, 310);
    
    // 返回按钮
    backButton.setSize(sf::Vector2f(100, 40));
    backButton.setFillColor(sf::Color(70, 70, 170));
    backButton.setPosition(50, 50);
    
    backButtonText.setFont(font);
    backButtonText.setString("return");
    backButtonText.setCharacterSize(18);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(75, 57);
}

void UserManager::loadUserData() {
    std::ifstream file(userDataFile);
    
    // 如果文件不存在，不需要加载
    if (!file) {
        return;
    }
    
    std::string line;
    std::string currentUsername;
    UserInfo currentUser;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            // 如果已经有用户信息，保存它
            if (!currentUsername.empty()) {
                users[currentUsername] = currentUser;
                currentUser = UserInfo();
            }
            
            // 提取新用户名
            currentUsername = line.substr(1, line.length() - 2);
            currentUser.username = currentUsername;
        }
        else if (line.find("password=") == 0) {
            currentUser.password = line.substr(9);
        }
        // 新格式数据加载
        else if (line.find("single_square_time:") == 0) {
            std::istringstream iss(line.substr(19));
            std::string diffStr;
            float time;
            
            if (std::getline(iss, diffStr, ':') && iss >> time) {
                Difficulty diff = stringToDifficulty(diffStr);
                currentUser.gameData.singleSquareBestTimes[diff] = time;
            }
        }
        else if (line.find("multi_square_stats:") == 0) {
            std::istringstream iss(line.substr(19));
            std::string diffStr;
            int wins, total;
            char colon;
            
            if (std::getline(iss, diffStr, ':') && iss >> wins >> colon >> total) {
                Difficulty diff = stringToDifficulty(diffStr);
                currentUser.gameData.multiSquareStats[diff] = std::make_pair(wins, total);
            }
        }
        else if (line.find("multi_hex_time:") == 0) {
            std::istringstream iss(line.substr(15));
            std::string diffStr;
            float time;
            
            if (std::getline(iss, diffStr, ':') && iss >> time) {
                Difficulty diff = stringToDifficulty(diffStr);
                currentUser.gameData.multiHexBestTimes[diff] = time;
            }
        }
        else if (line.find("multi_hex_stats:") == 0) {
            std::istringstream iss(line.substr(16));
            std::string diffStr;
            int wins, total;
            char colon;
            
            if (std::getline(iss, diffStr, ':') && iss >> wins >> colon >> total) {
                Difficulty diff = stringToDifficulty(diffStr);
                currentUser.gameData.multiHexStats[diff] = std::make_pair(wins, total);
            }
        }
        // 兼容旧格式
        else if (line.find("time:") == 0) {
            std::istringstream iss(line.substr(5));
            int mazeSize;
            float time;
            char colon;
            
            if (iss >> mazeSize >> colon >> time) {
                currentUser.bestTimes[mazeSize] = time;
            }
        }
    }
    
    // 保存最后一个用户
    if (!currentUsername.empty()) {
        users[currentUsername] = currentUser;
    }
    
    file.close();
}

void UserManager::saveUserData() {
    std::ofstream file(userDataFile);
    
    if (!file) {
        std::cerr << "cannot save user data" << std::endl;
        return;
    }
    
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        file << "[" << username << "]\n";
        file << "password=" << userInfo.password << "\n";
        
        // 保存新的游戏数据格式
        const auto& gameData = userInfo.gameData;
        
        // 单人正方形最佳时间
        for (const auto& timePair : gameData.singleSquareBestTimes) {
            file << "single_square_time:" << difficultyToString(timePair.first) 
                 << ":" << timePair.second << "\n";
        }
        
        // 双人正方形统计
        for (const auto& statPair : gameData.multiSquareStats) {
            file << "multi_square_stats:" << difficultyToString(statPair.first) 
                 << ":" << statPair.second.first << ":" << statPair.second.second << "\n";
        }
        
        // 双人六边形最佳时间
        for (const auto& timePair : gameData.multiHexBestTimes) {
            file << "multi_hex_time:" << difficultyToString(timePair.first) 
                 << ":" << timePair.second << "\n";
        }
        
        // 双人六边形统计
        for (const auto& statPair : gameData.multiHexStats) {
            file << "multi_hex_stats:" << difficultyToString(statPair.first) 
                 << ":" << statPair.second.first << ":" << statPair.second.second << "\n";
        }
        
        // 保留旧格式以便兼容性
        for (const auto& timePair : userInfo.bestTimes) {
            int mazeSize = timePair.first;
            float time = timePair.second;
            file << "time:" << mazeSize << ":" << time << "\n";
        }
        
        file << "\n";
    }
    
    file.close();
}

void UserManager::handleTextInput(sf::Event& event) {
    if (event.text.unicode < 128) {
        char input = static_cast<char>(event.text.unicode);
        
        if (focusUsername) {
            // 处理删除键
            if (event.text.unicode == 8 && !usernameInput.empty()) {
                usernameInput.pop_back();
            }
            // 忽略回车或制表符
            else if (input != '\r' && input != '\n' && input != '\t' && input != 8) {
                usernameInput += input;
            }
            
            usernameText.setString(usernameInput);
        }
        else if (focusPassword) {
            // 处理删除键
            if (event.text.unicode == 8 && !passwordInput.empty()) {
                passwordInput.pop_back();
            }
            // 忽略回车或制表符
            else if (input != '\r' && input != '\n' && input != '\t' && input != 8) {
                passwordInput += input;
            }
            
            // 显示密码为星号
            std::string displayPassword(passwordInput.length(), '*');
            passwordText.setString(displayPassword);
        }
    }
}

void UserManager::handleMouseClick(sf::Vector2i mousePos) {
    // 检查用户名框点击
    if (usernameBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        focusUsername = true;
        focusPassword = false;
        
        // 更新输入框外观
        usernameBox.setOutlineColor(sf::Color::Blue);
        passwordBox.setOutlineColor(sf::Color(200, 200, 200));
    }
    // 检查密码框点击
    else if (passwordBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        focusUsername = false;
        focusPassword = true;
        
        // 更新输入框外观
        usernameBox.setOutlineColor(sf::Color(200, 200, 200));
        passwordBox.setOutlineColor(sf::Color::Blue);
    }
    // 检查登录按钮点击
    else if (loginButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        if (usernameInput.empty() || passwordInput.empty()) {
            statusText.setString("input username and password");
        } else {
            if (loginUser(usernameInput, passwordInput)) {
                statusText.setString("login successfully, welcome " + currentUser);
                // 登录成功后延迟关闭窗口
                sf::Clock delayClock;
                while (delayClock.getElapsedTime().asSeconds() < 1.5f) {
                    window.clear(sf::Color(30, 30, 30));
                    window.draw(titleText);
                    window.draw(statusText);
                    window.display();
                }
                window.close();
            } else {
                statusText.setString("username or password error");
            }
        }
    }
    // 检查注册按钮点击
    else if (registerButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        if (usernameInput.empty() || passwordInput.empty()) {
            statusText.setString("input username and password");
        } else {
            if (registerUser(usernameInput, passwordInput)) {
                statusText.setString("register successfully, please login");
            } else {
                statusText.setString("username already exists");
            }
        }
    }
    // 检查返回按钮点击
    else if (backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        window.close();
    }
    else {
        focusUsername = false;
        focusPassword = false;
        
        // 恢复输入框默认外观
        usernameBox.setOutlineColor(sf::Color(200, 200, 200));
        passwordBox.setOutlineColor(sf::Color(200, 200, 200));
    }
}

bool UserManager::registerUser(const std::string& username, const std::string& password) {
    // 检查用户是否已存在
    if (users.find(username) != users.end()) {
        return false;
    }
    
    // 创建新用户
    UserInfo newUser;
    newUser.username = username;
    newUser.password = password;
    
    // 添加到用户列表
    users[username] = newUser;
    
    // 保存到文件
    saveUserData();
    
    return true;
}

bool UserManager::loginUser(const std::string& username, const std::string& password) {
    // 检查用户是否存在
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    // 检查密码是否正确
    if (it->second.password != password) {
        return false;
    }
    
    // 设置当前用户
    currentUser = username;
    isLoggedIn = true;
    
    return true;
}

void UserManager::run() {
    // 只在调用run方法时创建窗口，避免启动时显示空白界面
    if (!window.isOpen()) {
        initWindow();
    }
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::TextEntered) {
                handleTextInput(event);
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(sf::Mouse::getPosition(window));
                }
            }
        }
        
        // 绘制界面
        window.clear(sf::Color(30, 30, 30));
        
        // 绘制界面元素
        window.draw(titleText);
        window.draw(statusText);
        window.draw(usernameBox);
        window.draw(passwordBox);
        window.draw(usernameLabel);
        window.draw(passwordLabel);
        window.draw(usernameText);
        window.draw(passwordText);
        window.draw(loginButton);
        window.draw(registerButton);
        window.draw(loginButtonText);
        window.draw(registerButtonText);
        window.draw(backButton);
        window.draw(backButtonText);
        
        window.display();
    }
}

bool UserManager::isUserLoggedIn() const {
    return isLoggedIn;
}

std::string UserManager::getCurrentUser() const {
    return currentUser;
}

void UserManager::updateBestTime(int mazeSize, float time) {
    if (!isLoggedIn) return;
    
    // 获取当前用户
    auto& user = users[currentUser];
    
    // 如果还没有该迷宫大小的记录，或者新时间更好，更新记录
    if (user.bestTimes.find(mazeSize) == user.bestTimes.end() || time < user.bestTimes[mazeSize]) {
        user.bestTimes[mazeSize] = time;
        saveUserData();
    }
}

std::vector<std::pair<std::string, float>> UserManager::getLeaderboard(int mazeSize) {
    std::vector<std::pair<std::string, float>> leaderboard;
    
    // 遍历所有用户，查找有该迷宫大小记录的用户
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        auto it = userInfo.bestTimes.find(mazeSize);
        if (it != userInfo.bestTimes.end()) {
            leaderboard.push_back(std::make_pair(username, it->second));
        }
    }
    
    // 按时间从小到大排序
    std::sort(leaderboard.begin(), leaderboard.end(), 
            [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return leaderboard;
}

void UserManager::logout() {
    isLoggedIn = false;
    currentUser = "";
}

std::string UserManager::difficultyToString(Difficulty diff) const {
    switch (diff) {
        case Difficulty::EASY: return "easy";
        case Difficulty::MEDIUM: return "medium";
        case Difficulty::HARD: return "hard";
        default: return "medium";
    }
}

Difficulty UserManager::stringToDifficulty(const std::string& str) const {
    if (str == "easy") return Difficulty::EASY;
    if (str == "hard") return Difficulty::HARD;
    return Difficulty::MEDIUM; // 默认中等
}

std::string UserManager::gameModeToString(GameMode mode) const {
    switch (mode) {
        case GameMode::SINGLE_SQUARE: return "single_square";
        case GameMode::MULTI_SQUARE: return "multi_square";
        case GameMode::MULTI_HEX: return "multi_hex";
        default: return "single_square";
    }
}

GameMode UserManager::stringToGameMode(const std::string& str) const {
    if (str == "multi_square") return GameMode::MULTI_SQUARE;
    if (str == "multi_hex") return GameMode::MULTI_HEX;
    return GameMode::SINGLE_SQUARE; // 默认单人正方形
}

void UserManager::updateSingleSquareBestTime(Difficulty diff, float time) {
    if (!isLoggedIn) return;
    
    auto& user = users[currentUser];
    auto& bestTimes = user.gameData.singleSquareBestTimes;
    
    // 如果还没有该难度的记录，或者新时间更好，更新记录
    if (bestTimes.find(diff) == bestTimes.end() || time < bestTimes[diff]) {
        bestTimes[diff] = time;
        saveUserData();
    }
}

void UserManager::updateMultiSquareGameResult(Difficulty diff, bool won) {
    if (!isLoggedIn) return;
    
    auto& user = users[currentUser];
    auto& stats = user.gameData.multiSquareStats;
    
    // 如果还没有该难度的记录，初始化为(0, 0)
    if (stats.find(diff) == stats.end()) {
        stats[diff] = std::make_pair(0, 0);
    }
    
    // 更新统计数据
    if (won) {
        stats[diff].first++;  // 胜利次数+1
    }
    stats[diff].second++;     // 总次数+1
    
    saveUserData();
}

void UserManager::updateMultiHexBestTime(Difficulty diff, float time) {
    if (!isLoggedIn) return;
    
    auto& user = users[currentUser];
    auto& bestTimes = user.gameData.multiHexBestTimes;
    
    // 如果还没有该难度的记录，或者新时间更好，更新记录
    if (bestTimes.find(diff) == bestTimes.end() || time < bestTimes[diff]) {
        bestTimes[diff] = time;
        saveUserData();
    }
}

void UserManager::updateMultiHexGameResult(Difficulty diff, bool won) {
    if (!isLoggedIn) return;
    
    auto& user = users[currentUser];
    auto& stats = user.gameData.multiHexStats;
    
    // 如果还没有该难度的记录，初始化为(0, 0)
    if (stats.find(diff) == stats.end()) {
        stats[diff] = std::make_pair(0, 0);
    }
    
    // 更新统计数据
    if (won) {
        stats[diff].first++;  // 胜利次数+1
    }
    stats[diff].second++;     // 总次数+1
    
    saveUserData();
}

std::map<std::string, float> UserManager::getSingleSquareLeaderboard(Difficulty diff) {
    std::map<std::string, float> leaderboard;
    
    // 遍历所有用户，查找有该难度记录的用户
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        auto it = userInfo.gameData.singleSquareBestTimes.find(diff);
        if (it != userInfo.gameData.singleSquareBestTimes.end()) {
            leaderboard[username] = it->second;
        }
    }
    
    return leaderboard;
}

std::map<std::string, std::pair<int, int>> UserManager::getMultiSquareLeaderboard(Difficulty diff) {
    std::map<std::string, std::pair<int, int>> leaderboard;
    
    // 遍历所有用户，查找有该难度记录的用户
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        auto it = userInfo.gameData.multiSquareStats.find(diff);
        if (it != userInfo.gameData.multiSquareStats.end()) {
            leaderboard[username] = it->second;
        }
    }
    
    return leaderboard;
}

std::map<std::string, float> UserManager::getMultiHexTimeLeaderboard(Difficulty diff) {
    std::map<std::string, float> leaderboard;
    
    // 遍历所有用户，查找有该难度记录的用户
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        auto it = userInfo.gameData.multiHexBestTimes.find(diff);
        if (it != userInfo.gameData.multiHexBestTimes.end()) {
            leaderboard[username] = it->second;
        }
    }
    
    return leaderboard;
}

std::map<std::string, std::pair<int, int>> UserManager::getMultiHexStatsLeaderboard(Difficulty diff) {
    std::map<std::string, std::pair<int, int>> leaderboard;
    
    // 遍历所有用户，查找有该难度记录的用户
    for (const auto& userPair : users) {
        const std::string& username = userPair.first;
        const UserInfo& userInfo = userPair.second;
        
        auto it = userInfo.gameData.multiHexStats.find(diff);
        if (it != userInfo.gameData.multiHexStats.end()) {
            leaderboard[username] = it->second;
        }
    }
    
    return leaderboard;
}

void UserManager::forceSaveData()
{
    std::cout << "forceSaveData() called - ensuring user data is saved" << std::endl;
    try {
        saveUserData();
        std::cout << "User data force saved successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error during force save: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error during force save" << std::endl;
    }
} 