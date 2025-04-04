#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <functional>

// 按钮类
class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    std::function<void()> callback;
    bool isHovered;

public:
    // 添加默认构造函数
    Button() : isHovered(false) {}
    
    // 设置按钮属性
    void setup(float x, float y, float width, float height, 
        const sf::Font& font, const std::string& buttonText);
        
    void setColors(const sf::Color& idle, const sf::Color& hover);
    void setCallback(std::function<void()> func);
    void update(const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window);
    bool isMouseOver(const sf::Vector2f& mousePos) const;
    
    // 使friend可以访问私有成员
    friend class MainScene;
};

// 主界面类
class MainScene {
public:
    // 场景状态 - 移到public部分
    enum class SceneState {
        MAIN_MENU,
        TUTORIAL,
        SINGLE_PLAYER,
        MULTI_PLAYER,
        LEADERBOARD,
        EXIT
    };

    MainScene();
    ~MainScene();
    
    // 运行主循环
    void run();
    
    // 获取当前场景状态
    SceneState getState() const;

private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text titleText;
    sf::Music backgroundMusic;
    bool isMusicPlaying;
    
    // 按钮
    Button tutorialButton;      // 教程按钮
    Button singlePlayerButton;  // 单人游戏按钮
    Button multiPlayerButton;   // 多人游戏按钮
    Button loginButton;         // 登录按钮
    Button leaderboardButton;   // 排行榜按钮
    Button musicToggleButton;   // 音乐开关按钮
    
    SceneState currentState;
    
    // 初始化界面元素
    void initWindow();
    void initFont();
    void initButtons();
    void initMusic();
    
    // 事件处理
    void processEvents();
    
    // 更新所有按钮状态（新增方法）
    void updateButtons(const sf::Vector2f& mousePos);
    
    // 处理按钮点击（新增方法）
    void handleButtonClicks(const sf::Vector2f& mousePos);
    
    // 绘制界面
    void render();
};

#endif // MAIN_SCENE_H