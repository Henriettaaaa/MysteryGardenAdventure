#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <SFML/Graphics.hpp>
#include "UserManager.h"
#include <vector>
#include <string>

class Leaderboard {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text titleText;
    sf::Text noDataText;
    sf::RectangleShape backButton;
    sf::Text backButtonText;
    
    // 排行榜条目
    struct LeaderboardEntry {
        sf::Text rankText;
        sf::Text usernameText;
        sf::Text timeText;
    };
    
    std::vector<LeaderboardEntry> entries;
    UserManager& userManager;
    int selectedMazeSize;
    
    // 初始化界面
    void initWindow();
    void initFont();
    void initUI();
    
    // 加载排行榜数据
    void loadLeaderboardData();
    
    // 处理鼠标点击
    void handleMouseClick(sf::Vector2i mousePos);
    
public:
    Leaderboard(UserManager& manager, int mazeSize = 5);
    void run();
};

#endif // LEADERBOARD_H 