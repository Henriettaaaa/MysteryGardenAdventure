#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <SFML/Graphics.hpp>
#include "UserManager.h"
#include <vector>
#include <string>

// 排序类型枚举
enum class SortType {
    REGISTRATION_ORDER = 0,     // 注册顺序（默认）
    SINGLE_EASY = 1,           // 单人Easy时间
    SINGLE_MEDIUM = 2,         // 单人Medium时间
    SINGLE_HARD = 3,           // 单人Hard时间
    MULTI_SQUARE_EASY = 4,     // 双人正方形Easy胜率
    MULTI_SQUARE_MEDIUM = 5,   // 双人正方形Medium胜率
    MULTI_SQUARE_HARD = 6,     // 双人正方形Hard胜率
    MULTI_HEX_TIME_EASY = 7,   // 双人六边形Easy时间
    MULTI_HEX_TIME_MEDIUM = 8, // 双人六边形Medium时间
    MULTI_HEX_TIME_HARD = 9,   // 双人六边形Hard时间
    MULTI_HEX_STATS_EASY = 10, // 双人六边形Easy胜率
    MULTI_HEX_STATS_MEDIUM = 11, // 双人六边形Medium胜率
    MULTI_HEX_STATS_HARD = 12  // 双人六边形Hard胜率
};

// 玩家完整数据行
struct PlayerRowData {
    std::string username;
    int registrationOrder;  // 注册顺序
    
    // 单人正方形时间
    float singleEasy;
    float singleMedium;
    float singleHard;
    
    // 双人正方形胜率
    float multiSquareEasy;
    float multiSquareMedium;
    float multiSquareHard;
    
    // 双人六边形时间
    float multiHexTimeEasy;
    float multiHexTimeMedium;
    float multiHexTimeHard;
    
    // 双人六边形胜率
    float multiHexStatsEasy;
    float multiHexStatsMedium;
    float multiHexStatsHard;
    
    // 原始统计数据（用于显示详细信息）
    std::pair<int, int> multiSquareStatsEasy;
    std::pair<int, int> multiSquareStatsMedium;
    std::pair<int, int> multiSquareStatsHard;
    std::pair<int, int> multiHexStatsDataEasy;
    std::pair<int, int> multiHexStatsDataMedium;
    std::pair<int, int> multiHexStatsDataHard;
};

// 表格UI元素
struct TableRow {
    sf::Text usernameText;
    sf::Text singleEasyText;
    sf::Text singleMediumText;
    sf::Text singleHardText;
    sf::Text multiSquareEasyText;
    sf::Text multiSquareMediumText;
    sf::Text multiSquareHardText;
    sf::Text multiHexTimeEasyText;
    sf::Text multiHexTimeMediumText;
    sf::Text multiHexTimeHardText;
    sf::Text multiHexStatsEasyText;
    sf::Text multiHexStatsMediumText;
    sf::Text multiHexStatsHardText;
};

class Leaderboard {
private:
    UserManager& userManager;
    sf::RenderWindow window;
    sf::Font font;
    
    // 当前排序方式
    SortType currentSortType;
    
    // UI元素
    sf::Text titleText;
    sf::Text noDataText;
    sf::RectangleShape backButton;
    sf::Text backButtonText;
    
    // 表格列标题
    sf::Text usernameHeaderText;
    sf::Text singleEasyHeaderText;
    sf::Text singleMediumHeaderText;
    sf::Text singleHardHeaderText;
    sf::Text multiSquareEasyHeaderText;
    sf::Text multiSquareMediumHeaderText;
    sf::Text multiSquareHardHeaderText;
    sf::Text multiHexTimeEasyHeaderText;
    sf::Text multiHexTimeMediumHeaderText;
    sf::Text multiHexTimeHardHeaderText;
    sf::Text multiHexStatsEasyHeaderText;
    sf::Text multiHexStatsMediumHeaderText;
    sf::Text multiHexStatsHardHeaderText;
    
    // 列标题背景框（用于点击检测）
    sf::RectangleShape usernameHeaderBox;
    sf::RectangleShape singleEasyHeaderBox;
    sf::RectangleShape singleMediumHeaderBox;
    sf::RectangleShape singleHardHeaderBox;
    sf::RectangleShape multiSquareEasyHeaderBox;
    sf::RectangleShape multiSquareMediumHeaderBox;
    sf::RectangleShape multiSquareHardHeaderBox;
    sf::RectangleShape multiHexTimeEasyHeaderBox;
    sf::RectangleShape multiHexTimeMediumHeaderBox;
    sf::RectangleShape multiHexTimeHardHeaderBox;
    sf::RectangleShape multiHexStatsEasyHeaderBox;
    sf::RectangleShape multiHexStatsMediumHeaderBox;
    sf::RectangleShape multiHexStatsHardHeaderBox;
    
    // 玩家数据和表格行
    std::vector<PlayerRowData> playerData;
    std::vector<TableRow> tableRows;
    
    // 滚动相关
    float scrollOffset;
    float maxScrollOffset;
    
    // 初始化函数
    void initWindow();
    void initFont();
    void initUI();
    void initTableHeaders();
    
    // 数据处理
    void loadAllPlayerData();
    void sortPlayerData();
    void updateTableRows();
    
    // 事件处理
    void handleMouseClick(sf::Vector2i mousePos);
    void handleScroll(sf::Event& event);
    
    // 渲染
    void drawTable();
    void drawHeaders();
    void drawPlayerRows();
    
    // 工具函数
    std::string formatTime(float time);
    std::string formatWinRate(int wins, int total);
    std::string formatWinRateFloat(float rate);
    float calculateWinRate(int wins, int total);
    SortType getHeaderSortType(sf::Vector2i mousePos);

public:
    Leaderboard(UserManager& manager);
    void run();
};

#endif // LEADERBOARD_H 