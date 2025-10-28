#include "Leaderboard.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Leaderboard::Leaderboard(UserManager& manager, int mazeSize)
    : userManager(manager), selectedMazeSize(mazeSize) {
    initWindow();
    initFont();
    initUI();
    loadLeaderboardData();
}

void Leaderboard::initWindow() {
    window.create(sf::VideoMode(800, 600), "Ranking List", sf::Style::Close);
    window.setFramerateLimit(60);
}

void Leaderboard::initFont() {
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }
}

void Leaderboard::initUI() {
    // 标题
    titleText.setFont(font);
    std::ostringstream titleStream;
    titleStream << "maze size " << selectedMazeSize << "x" << selectedMazeSize << "ranking list";
    titleText.setString(titleStream.str());
    titleText.setCharacterSize(30);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(250, 50);
    
    // 无数据提示
    noDataText.setFont(font);
    noDataText.setString("no records");
    noDataText.setCharacterSize(24);
    noDataText.setFillColor(sf::Color::Yellow);
    noDataText.setPosition(350, 300);
    
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

void Leaderboard::loadLeaderboardData() {
    // 清空现有条目
    entries.clear();
    
    // 获取排行榜数据
    auto leaderboardData = userManager.getLeaderboard(selectedMazeSize);
    
    // 如果没有数据，不需要创建条目
    if (leaderboardData.empty()) {
        return;
    }
    
    // 为每条记录创建一个条目
    int rank = 1;
    float yPosition = 120.0f;
    
    for (const auto& record : leaderboardData) {
        // 最多显示10条记录
        if (rank > 10) break;
        
        // 获取用户名和时间
        const std::string& username = record.first;
        float time = record.second;
        
        LeaderboardEntry entry;
        
        // 排名
        entry.rankText.setFont(font);
        entry.rankText.setString(std::to_string(rank));
        entry.rankText.setCharacterSize(20);
        entry.rankText.setFillColor(sf::Color::White);
        entry.rankText.setPosition(200, yPosition);
        
        // 用户名
        entry.usernameText.setFont(font);
        entry.usernameText.setString(username);
        entry.usernameText.setCharacterSize(20);
        entry.usernameText.setFillColor(sf::Color::White);
        entry.usernameText.setPosition(250, yPosition);
        
        // 时间
        entry.timeText.setFont(font);
        
        // 格式化时间为 MM:SS.mmm (分:秒.毫秒)
        int minutes = static_cast<int>(time) / 60;
        int seconds = static_cast<int>(time) % 60;
        int milliseconds = static_cast<int>((time - static_cast<int>(time)) * 1000);
        
        std::ostringstream timeStream;
        timeStream << std::setfill('0') << std::setw(2) << minutes << ":" 
                  << std::setfill('0') << std::setw(2) << seconds << "."
                  << std::setfill('0') << std::setw(3) << milliseconds;
        
        entry.timeText.setString(timeStream.str());
        entry.timeText.setCharacterSize(20);
        entry.timeText.setFillColor(sf::Color::White);
        entry.timeText.setPosition(450, yPosition);
        
        entries.push_back(entry);
        
        // 增加排名和位置
        rank++;
        yPosition += 40.0f;
    }
}

void Leaderboard::handleMouseClick(sf::Vector2i mousePos) {
    // 检查返回按钮点击
    if (backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        window.close();
    }
}

void Leaderboard::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(sf::Mouse::getPosition(window));
                }
            }
        }
        
        // 绘制界面
        window.clear(sf::Color(30, 30, 30));
        
        // 绘制标题和返回按钮
        window.draw(titleText);
        window.draw(backButton);
        window.draw(backButtonText);
        
        // 绘制排行榜条目
        if (entries.empty()) {
            window.draw(noDataText);
        } else {
            for (const auto& entry : entries) {
                window.draw(entry.rankText);
                window.draw(entry.usernameText);
                window.draw(entry.timeText);
            }
        }
        
        window.display();
    }
} 