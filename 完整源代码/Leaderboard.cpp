#include "Leaderboard.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

Leaderboard::Leaderboard(UserManager& manager)
    : userManager(manager), currentSortType(SortType::REGISTRATION_ORDER), scrollOffset(0.0f), maxScrollOffset(0.0f) {
    initWindow();
    initFont();
    initUI();
    initTableHeaders();
    loadAllPlayerData();
    sortPlayerData();
    updateTableRows();
}

void Leaderboard::initWindow() {
    window.create(sf::VideoMode(1200, 700), "Player Statistics", sf::Style::Close);
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
    titleText.setString("Player Statistics - All Data");
    titleText.setCharacterSize(24);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(450, 10);
    
    // 无数据提示
    noDataText.setFont(font);
    noDataText.setString("No player data available");
    noDataText.setCharacterSize(20);
    noDataText.setFillColor(sf::Color::Yellow);
    noDataText.setPosition(450, 300);
    
    // 返回按钮
    backButton.setSize(sf::Vector2f(80, 30));
    backButton.setFillColor(sf::Color(163, 168, 143));
    backButton.setPosition(20, 20);
    
    backButtonText.setFont(font);
    backButtonText.setString("Return");
    backButtonText.setCharacterSize(14);
    backButtonText.setFillColor(sf::Color::White);
    backButtonText.setPosition(35, 27);
}

void Leaderboard::initTableHeaders() {
    float headerY = 50.0f;
    float startX = 20.0f;
    float colWidth = 90.0f;
    
    // 设置列标题文本和背景框
    usernameHeaderText.setFont(font);
    usernameHeaderText.setString("Player");
    usernameHeaderText.setCharacterSize(12);
    usernameHeaderText.setFillColor(sf::Color::White);
    usernameHeaderText.setPosition(startX, headerY);
    usernameHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    usernameHeaderBox.setPosition(startX, headerY);
    usernameHeaderBox.setFillColor(sf::Color(50, 50, 100));
    
    startX += colWidth;
    
    // 单人正方形时间列
    singleEasyHeaderText.setFont(font);
    singleEasyHeaderText.setString("S-Easy");
    singleEasyHeaderText.setCharacterSize(12);
    singleEasyHeaderText.setFillColor(sf::Color::White);
    singleEasyHeaderText.setPosition(startX, headerY);
    singleEasyHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    singleEasyHeaderBox.setPosition(startX, headerY);
    singleEasyHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    singleMediumHeaderText.setFont(font);
    singleMediumHeaderText.setString("S-Med");
    singleMediumHeaderText.setCharacterSize(12);
    singleMediumHeaderText.setFillColor(sf::Color::White);
    singleMediumHeaderText.setPosition(startX, headerY);
    singleMediumHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    singleMediumHeaderBox.setPosition(startX, headerY);
    singleMediumHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    singleHardHeaderText.setFont(font);
    singleHardHeaderText.setString("S-Hard");
    singleHardHeaderText.setCharacterSize(12);
    singleHardHeaderText.setFillColor(sf::Color::White);
    singleHardHeaderText.setPosition(startX, headerY);
    singleHardHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    singleHardHeaderBox.setPosition(startX, headerY);
    singleHardHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    // 双人正方形胜率列
    multiSquareEasyHeaderText.setFont(font);
    multiSquareEasyHeaderText.setString("MS-Easy");
    multiSquareEasyHeaderText.setCharacterSize(12);
    multiSquareEasyHeaderText.setFillColor(sf::Color::White);
    multiSquareEasyHeaderText.setPosition(startX, headerY);
    multiSquareEasyHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiSquareEasyHeaderBox.setPosition(startX, headerY);
    multiSquareEasyHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiSquareMediumHeaderText.setFont(font);
    multiSquareMediumHeaderText.setString("MS-Med");
    multiSquareMediumHeaderText.setCharacterSize(12);
    multiSquareMediumHeaderText.setFillColor(sf::Color::White);
    multiSquareMediumHeaderText.setPosition(startX, headerY);
    multiSquareMediumHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiSquareMediumHeaderBox.setPosition(startX, headerY);
    multiSquareMediumHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiSquareHardHeaderText.setFont(font);
    multiSquareHardHeaderText.setString("MS-Hard");
    multiSquareHardHeaderText.setCharacterSize(12);
    multiSquareHardHeaderText.setFillColor(sf::Color::White);
    multiSquareHardHeaderText.setPosition(startX, headerY);
    multiSquareHardHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiSquareHardHeaderBox.setPosition(startX, headerY);
    multiSquareHardHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    // 双人六边形时间列
    multiHexTimeEasyHeaderText.setFont(font);
    multiHexTimeEasyHeaderText.setString("HT-Easy");
    multiHexTimeEasyHeaderText.setCharacterSize(12);
    multiHexTimeEasyHeaderText.setFillColor(sf::Color::White);
    multiHexTimeEasyHeaderText.setPosition(startX, headerY);
    multiHexTimeEasyHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexTimeEasyHeaderBox.setPosition(startX, headerY);
    multiHexTimeEasyHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiHexTimeMediumHeaderText.setFont(font);
    multiHexTimeMediumHeaderText.setString("HT-Med");
    multiHexTimeMediumHeaderText.setCharacterSize(12);
    multiHexTimeMediumHeaderText.setFillColor(sf::Color::White);
    multiHexTimeMediumHeaderText.setPosition(startX, headerY);
    multiHexTimeMediumHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexTimeMediumHeaderBox.setPosition(startX, headerY);
    multiHexTimeMediumHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiHexTimeHardHeaderText.setFont(font);
    multiHexTimeHardHeaderText.setString("HT-Hard");
    multiHexTimeHardHeaderText.setCharacterSize(12);
    multiHexTimeHardHeaderText.setFillColor(sf::Color::White);
    multiHexTimeHardHeaderText.setPosition(startX, headerY);
    multiHexTimeHardHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexTimeHardHeaderBox.setPosition(startX, headerY);
    multiHexTimeHardHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    // 双人六边形胜率列
    multiHexStatsEasyHeaderText.setFont(font);
    multiHexStatsEasyHeaderText.setString("HS-Easy");
    multiHexStatsEasyHeaderText.setCharacterSize(12);
    multiHexStatsEasyHeaderText.setFillColor(sf::Color::White);
    multiHexStatsEasyHeaderText.setPosition(startX, headerY);
    multiHexStatsEasyHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexStatsEasyHeaderBox.setPosition(startX, headerY);
    multiHexStatsEasyHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiHexStatsMediumHeaderText.setFont(font);
    multiHexStatsMediumHeaderText.setString("HS-Med");
    multiHexStatsMediumHeaderText.setCharacterSize(12);
    multiHexStatsMediumHeaderText.setFillColor(sf::Color::White);
    multiHexStatsMediumHeaderText.setPosition(startX, headerY);
    multiHexStatsMediumHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexStatsMediumHeaderBox.setPosition(startX, headerY);
    multiHexStatsMediumHeaderBox.setFillColor(sf::Color(50, 50, 100));
    startX += colWidth;
    
    multiHexStatsHardHeaderText.setFont(font);
    multiHexStatsHardHeaderText.setString("HS-Hard");
    multiHexStatsHardHeaderText.setCharacterSize(12);
    multiHexStatsHardHeaderText.setFillColor(sf::Color::White);
    multiHexStatsHardHeaderText.setPosition(startX, headerY);
    multiHexStatsHardHeaderBox.setSize(sf::Vector2f(colWidth, 25));
    multiHexStatsHardHeaderBox.setPosition(startX, headerY);
    multiHexStatsHardHeaderBox.setFillColor(sf::Color(50, 50, 100));
}

void Leaderboard::loadAllPlayerData() {
    playerData.clear();
    
    // 需要访问UserManager的私有成员，我们需要添加一个getter方法或者friend声明
    // 这里假设我们有一个getAllUsers方法
    auto singleEasyData = userManager.getSingleSquareLeaderboard(Difficulty::EASY);
    auto singleMediumData = userManager.getSingleSquareLeaderboard(Difficulty::MEDIUM);
    auto singleHardData = userManager.getSingleSquareLeaderboard(Difficulty::HARD);
    
    auto multiSquareEasyData = userManager.getMultiSquareLeaderboard(Difficulty::EASY);
    auto multiSquareMediumData = userManager.getMultiSquareLeaderboard(Difficulty::MEDIUM);
    auto multiSquareHardData = userManager.getMultiSquareLeaderboard(Difficulty::HARD);
    
    auto multiHexTimeEasyData = userManager.getMultiHexTimeLeaderboard(Difficulty::EASY);
    auto multiHexTimeMediumData = userManager.getMultiHexTimeLeaderboard(Difficulty::MEDIUM);
    auto multiHexTimeHardData = userManager.getMultiHexTimeLeaderboard(Difficulty::HARD);
    
    auto multiHexStatsEasyData = userManager.getMultiHexStatsLeaderboard(Difficulty::EASY);
    auto multiHexStatsMediumData = userManager.getMultiHexStatsLeaderboard(Difficulty::MEDIUM);
    auto multiHexStatsHardData = userManager.getMultiHexStatsLeaderboard(Difficulty::HARD);
    
    // 收集所有用户名
    std::set<std::string> allUsernames;
    for (const auto& pair : singleEasyData) allUsernames.insert(pair.first);
    for (const auto& pair : singleMediumData) allUsernames.insert(pair.first);
    for (const auto& pair : singleHardData) allUsernames.insert(pair.first);
    for (const auto& pair : multiSquareEasyData) allUsernames.insert(pair.first);
    for (const auto& pair : multiSquareMediumData) allUsernames.insert(pair.first);
    for (const auto& pair : multiSquareHardData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexTimeEasyData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexTimeMediumData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexTimeHardData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexStatsEasyData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexStatsMediumData) allUsernames.insert(pair.first);
    for (const auto& pair : multiHexStatsHardData) allUsernames.insert(pair.first);
    
    int order = 0;
    for (const std::string& username : allUsernames) {
        PlayerRowData rowData;
        rowData.username = username;
        rowData.registrationOrder = order++;
        
        // 初始化所有数据为无效值
        rowData.singleEasy = singleEasyData.count(username) ? singleEasyData[username] : -1.0f;
        rowData.singleMedium = singleMediumData.count(username) ? singleMediumData[username] : -1.0f;
        rowData.singleHard = singleHardData.count(username) ? singleHardData[username] : -1.0f;
        
        // 双人正方形数据
        if (multiSquareEasyData.count(username)) {
            rowData.multiSquareStatsEasy = multiSquareEasyData[username];
            rowData.multiSquareEasy = calculateWinRate(rowData.multiSquareStatsEasy.first, rowData.multiSquareStatsEasy.second);
        } else {
            rowData.multiSquareStatsEasy = {0, 0};
            rowData.multiSquareEasy = -1.0f;
        }
        
        if (multiSquareMediumData.count(username)) {
            rowData.multiSquareStatsMedium = multiSquareMediumData[username];
            rowData.multiSquareMedium = calculateWinRate(rowData.multiSquareStatsMedium.first, rowData.multiSquareStatsMedium.second);
        } else {
            rowData.multiSquareStatsMedium = {0, 0};
            rowData.multiSquareMedium = -1.0f;
        }
        
        if (multiSquareHardData.count(username)) {
            rowData.multiSquareStatsHard = multiSquareHardData[username];
            rowData.multiSquareHard = calculateWinRate(rowData.multiSquareStatsHard.first, rowData.multiSquareStatsHard.second);
        } else {
            rowData.multiSquareStatsHard = {0, 0};
            rowData.multiSquareHard = -1.0f;
        }
        
        // 双人六边形时间数据
        rowData.multiHexTimeEasy = multiHexTimeEasyData.count(username) ? multiHexTimeEasyData[username] : -1.0f;
        rowData.multiHexTimeMedium = multiHexTimeMediumData.count(username) ? multiHexTimeMediumData[username] : -1.0f;
        rowData.multiHexTimeHard = multiHexTimeHardData.count(username) ? multiHexTimeHardData[username] : -1.0f;
        
        // 双人六边形统计数据
        if (multiHexStatsEasyData.count(username)) {
            rowData.multiHexStatsDataEasy = multiHexStatsEasyData[username];
            rowData.multiHexStatsEasy = calculateWinRate(rowData.multiHexStatsDataEasy.first, rowData.multiHexStatsDataEasy.second);
        } else {
            rowData.multiHexStatsDataEasy = {0, 0};
            rowData.multiHexStatsEasy = -1.0f;
        }
        
        if (multiHexStatsMediumData.count(username)) {
            rowData.multiHexStatsDataMedium = multiHexStatsMediumData[username];
            rowData.multiHexStatsMedium = calculateWinRate(rowData.multiHexStatsDataMedium.first, rowData.multiHexStatsDataMedium.second);
        } else {
            rowData.multiHexStatsDataMedium = {0, 0};
            rowData.multiHexStatsMedium = -1.0f;
        }
        
        if (multiHexStatsHardData.count(username)) {
            rowData.multiHexStatsDataHard = multiHexStatsHardData[username];
            rowData.multiHexStatsHard = calculateWinRate(rowData.multiHexStatsDataHard.first, rowData.multiHexStatsDataHard.second);
        } else {
            rowData.multiHexStatsDataHard = {0, 0};
            rowData.multiHexStatsHard = -1.0f;
        }
        
        playerData.push_back(rowData);
    }
}

void Leaderboard::sortPlayerData() {
    switch (currentSortType) {
        case SortType::REGISTRATION_ORDER:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         return a.registrationOrder < b.registrationOrder; });
            break;
        case SortType::SINGLE_EASY:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.singleEasy < 0 && b.singleEasy < 0) return false;
                         if (a.singleEasy < 0) return false;
                         if (b.singleEasy < 0) return true;
                         return a.singleEasy < b.singleEasy; });
            break;
        case SortType::SINGLE_MEDIUM:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.singleMedium < 0 && b.singleMedium < 0) return false;
                         if (a.singleMedium < 0) return false;
                         if (b.singleMedium < 0) return true;
                         return a.singleMedium < b.singleMedium; });
            break;
        case SortType::SINGLE_HARD:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.singleHard < 0 && b.singleHard < 0) return false;
                         if (a.singleHard < 0) return false;
                         if (b.singleHard < 0) return true;
                         return a.singleHard < b.singleHard; });
            break;
        case SortType::MULTI_SQUARE_EASY:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiSquareEasy < 0 && b.multiSquareEasy < 0) return false;
                         if (a.multiSquareEasy < 0) return false;
                         if (b.multiSquareEasy < 0) return true;
                         return a.multiSquareEasy > b.multiSquareEasy; });
            break;
        case SortType::MULTI_SQUARE_MEDIUM:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiSquareMedium < 0 && b.multiSquareMedium < 0) return false;
                         if (a.multiSquareMedium < 0) return false;
                         if (b.multiSquareMedium < 0) return true;
                         return a.multiSquareMedium > b.multiSquareMedium; });
            break;
        case SortType::MULTI_SQUARE_HARD:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiSquareHard < 0 && b.multiSquareHard < 0) return false;
                         if (a.multiSquareHard < 0) return false;
                         if (b.multiSquareHard < 0) return true;
                         return a.multiSquareHard > b.multiSquareHard; });
            break;
        case SortType::MULTI_HEX_TIME_EASY:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexTimeEasy < 0 && b.multiHexTimeEasy < 0) return false;
                         if (a.multiHexTimeEasy < 0) return false;
                         if (b.multiHexTimeEasy < 0) return true;
                         return a.multiHexTimeEasy < b.multiHexTimeEasy; });
            break;
        case SortType::MULTI_HEX_TIME_MEDIUM:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexTimeMedium < 0 && b.multiHexTimeMedium < 0) return false;
                         if (a.multiHexTimeMedium < 0) return false;
                         if (b.multiHexTimeMedium < 0) return true;
                         return a.multiHexTimeMedium < b.multiHexTimeMedium; });
            break;
        case SortType::MULTI_HEX_TIME_HARD:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexTimeHard < 0 && b.multiHexTimeHard < 0) return false;
                         if (a.multiHexTimeHard < 0) return false;
                         if (b.multiHexTimeHard < 0) return true;
                         return a.multiHexTimeHard < b.multiHexTimeHard; });
            break;
        case SortType::MULTI_HEX_STATS_EASY:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexStatsEasy < 0 && b.multiHexStatsEasy < 0) return false;
                         if (a.multiHexStatsEasy < 0) return false;
                         if (b.multiHexStatsEasy < 0) return true;
                         return a.multiHexStatsEasy > b.multiHexStatsEasy; });
            break;
        case SortType::MULTI_HEX_STATS_MEDIUM:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexStatsMedium < 0 && b.multiHexStatsMedium < 0) return false;
                         if (a.multiHexStatsMedium < 0) return false;
                         if (b.multiHexStatsMedium < 0) return true;
                         return a.multiHexStatsMedium > b.multiHexStatsMedium; });
            break;
        case SortType::MULTI_HEX_STATS_HARD:
            std::sort(playerData.begin(), playerData.end(), 
                     [](const PlayerRowData& a, const PlayerRowData& b) { 
                         if (a.multiHexStatsHard < 0 && b.multiHexStatsHard < 0) return false;
                         if (a.multiHexStatsHard < 0) return false;
                         if (b.multiHexStatsHard < 0) return true;
                         return a.multiHexStatsHard > b.multiHexStatsHard; });
            break;
    }
}

void Leaderboard::updateTableRows() {
    tableRows.clear();
    
    float startY = 80.0f;
    float rowHeight = 25.0f;
    float startX = 20.0f;
    float colWidth = 90.0f;
    
    for (size_t i = 0; i < playerData.size(); i++) {
        const PlayerRowData& data = playerData[i];
        TableRow row;
        
        float currentY = startY + i * rowHeight - scrollOffset;
        float currentX = startX;
        
        // 只显示可见区域的行
        if (currentY < 80.0f || currentY > 680.0f) {
            continue;
        }
        
        // 用户名
        row.usernameText.setFont(font);
        row.usernameText.setString(data.username);
        row.usernameText.setCharacterSize(12);
        row.usernameText.setFillColor(sf::Color::White);
        row.usernameText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        // 单人时间数据
        row.singleEasyText.setFont(font);
        row.singleEasyText.setString(data.singleEasy >= 0 ? formatTime(data.singleEasy) : "-");
        row.singleEasyText.setCharacterSize(12);
        row.singleEasyText.setFillColor(sf::Color::White);
        row.singleEasyText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.singleMediumText.setFont(font);
        row.singleMediumText.setString(data.singleMedium >= 0 ? formatTime(data.singleMedium) : "-");
        row.singleMediumText.setCharacterSize(12);
        row.singleMediumText.setFillColor(sf::Color::White);
        row.singleMediumText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.singleHardText.setFont(font);
        row.singleHardText.setString(data.singleHard >= 0 ? formatTime(data.singleHard) : "-");
        row.singleHardText.setCharacterSize(12);
        row.singleHardText.setFillColor(sf::Color::White);
        row.singleHardText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        // 双人正方形胜率数据
        row.multiSquareEasyText.setFont(font);
        row.multiSquareEasyText.setString(data.multiSquareEasy >= 0 ? 
            formatWinRate(data.multiSquareStatsEasy.first, data.multiSquareStatsEasy.second) : "-");
        row.multiSquareEasyText.setCharacterSize(12);
        row.multiSquareEasyText.setFillColor(sf::Color::White);
        row.multiSquareEasyText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiSquareMediumText.setFont(font);
        row.multiSquareMediumText.setString(data.multiSquareMedium >= 0 ? 
            formatWinRate(data.multiSquareStatsMedium.first, data.multiSquareStatsMedium.second) : "-");
        row.multiSquareMediumText.setCharacterSize(12);
        row.multiSquareMediumText.setFillColor(sf::Color::White);
        row.multiSquareMediumText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiSquareHardText.setFont(font);
        row.multiSquareHardText.setString(data.multiSquareHard >= 0 ? 
            formatWinRate(data.multiSquareStatsHard.first, data.multiSquareStatsHard.second) : "-");
        row.multiSquareHardText.setCharacterSize(12);
        row.multiSquareHardText.setFillColor(sf::Color::White);
        row.multiSquareHardText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        // 双人六边形时间数据
        row.multiHexTimeEasyText.setFont(font);
        row.multiHexTimeEasyText.setString(data.multiHexTimeEasy >= 0 ? formatTime(data.multiHexTimeEasy) : "-");
        row.multiHexTimeEasyText.setCharacterSize(12);
        row.multiHexTimeEasyText.setFillColor(sf::Color::White);
        row.multiHexTimeEasyText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiHexTimeMediumText.setFont(font);
        row.multiHexTimeMediumText.setString(data.multiHexTimeMedium >= 0 ? formatTime(data.multiHexTimeMedium) : "-");
        row.multiHexTimeMediumText.setCharacterSize(12);
        row.multiHexTimeMediumText.setFillColor(sf::Color::White);
        row.multiHexTimeMediumText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiHexTimeHardText.setFont(font);
        row.multiHexTimeHardText.setString(data.multiHexTimeHard >= 0 ? formatTime(data.multiHexTimeHard) : "-");
        row.multiHexTimeHardText.setCharacterSize(12);
        row.multiHexTimeHardText.setFillColor(sf::Color::White);
        row.multiHexTimeHardText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        // 双人六边形胜率数据
        row.multiHexStatsEasyText.setFont(font);
        row.multiHexStatsEasyText.setString(data.multiHexStatsEasy >= 0 ? 
            formatWinRate(data.multiHexStatsDataEasy.first, data.multiHexStatsDataEasy.second) : "-");
        row.multiHexStatsEasyText.setCharacterSize(12);
        row.multiHexStatsEasyText.setFillColor(sf::Color::White);
        row.multiHexStatsEasyText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiHexStatsMediumText.setFont(font);
        row.multiHexStatsMediumText.setString(data.multiHexStatsMedium >= 0 ? 
            formatWinRate(data.multiHexStatsDataMedium.first, data.multiHexStatsDataMedium.second) : "-");
        row.multiHexStatsMediumText.setCharacterSize(12);
        row.multiHexStatsMediumText.setFillColor(sf::Color::White);
        row.multiHexStatsMediumText.setPosition(currentX, currentY);
        currentX += colWidth;
        
        row.multiHexStatsHardText.setFont(font);
        row.multiHexStatsHardText.setString(data.multiHexStatsHard >= 0 ? 
            formatWinRate(data.multiHexStatsDataHard.first, data.multiHexStatsDataHard.second) : "-");
        row.multiHexStatsHardText.setCharacterSize(12);
        row.multiHexStatsHardText.setFillColor(sf::Color::White);
        row.multiHexStatsHardText.setPosition(currentX, currentY);
        
        tableRows.push_back(row);
    }
    
    // 计算最大滚动偏移
    maxScrollOffset = std::max(0.0f, (float)playerData.size() * rowHeight - 500.0f);
}

void Leaderboard::handleMouseClick(sf::Vector2i mousePos) {
    // 检查返回按钮
    if (backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
        window.close();
        return;
    }
    
    // 检查列标题点击
    SortType newSortType = getHeaderSortType(mousePos);
    if (newSortType != currentSortType) {
        currentSortType = newSortType;
        sortPlayerData();
        updateTableRows();
    }
}

void Leaderboard::handleScroll(sf::Event& event) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        scrollOffset -= event.mouseWheelScroll.delta * 30.0f;
        scrollOffset = std::max(0.0f, std::min(scrollOffset, maxScrollOffset));
        updateTableRows();
    }
}

SortType Leaderboard::getHeaderSortType(sf::Vector2i mousePos) {
    float mx = static_cast<float>(mousePos.x);
    float my = static_cast<float>(mousePos.y);
    
    if (singleEasyHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::SINGLE_EASY;
    if (singleMediumHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::SINGLE_MEDIUM;
    if (singleHardHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::SINGLE_HARD;
    if (multiSquareEasyHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_SQUARE_EASY;
    if (multiSquareMediumHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_SQUARE_MEDIUM;
    if (multiSquareHardHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_SQUARE_HARD;
    if (multiHexTimeEasyHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_TIME_EASY;
    if (multiHexTimeMediumHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_TIME_MEDIUM;
    if (multiHexTimeHardHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_TIME_HARD;
    if (multiHexStatsEasyHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_STATS_EASY;
    if (multiHexStatsMediumHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_STATS_MEDIUM;
    if (multiHexStatsHardHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::MULTI_HEX_STATS_HARD;
    if (usernameHeaderBox.getGlobalBounds().contains(mx, my)) return SortType::REGISTRATION_ORDER;
    
    return currentSortType;
}

void Leaderboard::drawHeaders() {
    // 绘制所有列标题背景框
    window.draw(usernameHeaderBox);
    window.draw(singleEasyHeaderBox);
    window.draw(singleMediumHeaderBox);
    window.draw(singleHardHeaderBox);
    window.draw(multiSquareEasyHeaderBox);
    window.draw(multiSquareMediumHeaderBox);
    window.draw(multiSquareHardHeaderBox);
    window.draw(multiHexTimeEasyHeaderBox);
    window.draw(multiHexTimeMediumHeaderBox);
    window.draw(multiHexTimeHardHeaderBox);
    window.draw(multiHexStatsEasyHeaderBox);
    window.draw(multiHexStatsMediumHeaderBox);
    window.draw(multiHexStatsHardHeaderBox);
    
    // 绘制所有列标题文本
    window.draw(usernameHeaderText);
    window.draw(singleEasyHeaderText);
    window.draw(singleMediumHeaderText);
    window.draw(singleHardHeaderText);
    window.draw(multiSquareEasyHeaderText);
    window.draw(multiSquareMediumHeaderText);
    window.draw(multiSquareHardHeaderText);
    window.draw(multiHexTimeEasyHeaderText);
    window.draw(multiHexTimeMediumHeaderText);
    window.draw(multiHexTimeHardHeaderText);
    window.draw(multiHexStatsEasyHeaderText);
    window.draw(multiHexStatsMediumHeaderText);
    window.draw(multiHexStatsHardHeaderText);
}

void Leaderboard::drawPlayerRows() {
    for (const auto& row : tableRows) {
        window.draw(row.usernameText);
        window.draw(row.singleEasyText);
        window.draw(row.singleMediumText);
        window.draw(row.singleHardText);
        window.draw(row.multiSquareEasyText);
        window.draw(row.multiSquareMediumText);
        window.draw(row.multiSquareHardText);
        window.draw(row.multiHexTimeEasyText);
        window.draw(row.multiHexTimeMediumText);
        window.draw(row.multiHexTimeHardText);
        window.draw(row.multiHexStatsEasyText);
        window.draw(row.multiHexStatsMediumText);
        window.draw(row.multiHexStatsHardText);
    }
}

void Leaderboard::drawTable() {
    window.draw(titleText);
    window.draw(backButton);
    window.draw(backButtonText);
    
    if (playerData.empty()) {
        window.draw(noDataText);
    } else {
        drawHeaders();
        drawPlayerRows();
    }
}

std::string Leaderboard::formatTime(float time) {
    if (time < 0) return "-";
    
    int minutes = static_cast<int>(time) / 60;
    int seconds = static_cast<int>(time) % 60;
    int milliseconds = static_cast<int>((time - static_cast<int>(time)) * 1000);
    
    std::ostringstream timeStream;
    if (minutes > 0) {
        timeStream << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    } else {
        timeStream << seconds << "." << std::setfill('0') << std::setw(3) << milliseconds;
    }
    
    return timeStream.str();
}

std::string Leaderboard::formatWinRate(int wins, int total) {
    if (total == 0) return "-";
    
    float winRate = static_cast<float>(wins) / total * 100.0f;
    std::ostringstream rateStream;
    rateStream << wins << "/" << total << " " << std::fixed << std::setprecision(0) << winRate << "%";
    
    return rateStream.str();
}

std::string Leaderboard::formatWinRateFloat(float rate) {
    if (rate < 0) return "-";
    
    std::ostringstream rateStream;
    rateStream << std::fixed << std::setprecision(1) << rate << "%";
    
    return rateStream.str();
}

float Leaderboard::calculateWinRate(int wins, int total) {
    if (total == 0) return -1.0f;
    return static_cast<float>(wins) / total * 100.0f;
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
            else {
                handleScroll(event);
            }
        }
        
        // 绘制界面
        window.clear(sf::Color(169, 189, 126));
        drawTable();
        window.display();
    }
} 