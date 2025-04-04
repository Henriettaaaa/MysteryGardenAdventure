#include "MainScene.h"
#include <iostream>

// Button 类实现
void Button::setup(float x, float y, float width, float height, 
                  const sf::Font& font, const std::string& buttonText) {
    shape.setPosition(sf::Vector2f(x, y));
    shape.setSize(sf::Vector2f(width, height));
    
    text.setFont(font);
    text.setString(buttonText);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(18);
    
    // 居中文本
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(sf::Vector2f(x + width / 2.0f, y + height / 2.0f));
}

void Button::setColors(const sf::Color& idle, const sf::Color& hover) {
    shape.setFillColor(idle);
}

void Button::setCallback(std::function<void()> func) {
    callback = func;
}

void Button::update(const sf::Vector2f& mousePos) {
    isHovered = shape.getGlobalBounds().contains(mousePos);
    
    if (isHovered) {
        shape.setFillColor(sf::Color(100, 100, 200)); // 悬停颜色
    } else {
        shape.setFillColor(sf::Color(70, 70, 170)); // 默认颜色
    }
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(text);
}

bool Button::isMouseOver(const sf::Vector2f& mousePos) const {
    return shape.getGlobalBounds().contains(mousePos);
}

// MainScene 类实现
MainScene::MainScene() 
    : currentState(SceneState::MAIN_MENU),
      isMusicPlaying(true) {
    
    initWindow();
    initFont();
    initButtons();
    initMusic();
}

MainScene::~MainScene() {
    if (backgroundMusic.getStatus() == sf::Music::Playing) {
        backgroundMusic.stop();
    }
}

void MainScene::initWindow() {
    window.create(sf::VideoMode(1024, 768), "game", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);
}

void MainScene::initFont() {
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Error loading font" << std::endl;
    }
    
    titleText.setFont(font);
    titleText.setString("game");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::White);
    
    // 居中标题
    sf::FloatRect textRect = titleText.getLocalBounds();
    titleText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    titleText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, 100.0f));
}

void MainScene::initButtons() {
    // 初始化按钮
    sf::Color idleColor(70, 70, 170);
    sf::Color hoverColor(100, 100, 200);
    
    tutorialButton.setup(400, 200, 200, 50, font, "teaching");
    tutorialButton.setColors(idleColor, hoverColor);
    
    singlePlayerButton.setup(400, 270, 200, 50, font, "single");
    singlePlayerButton.setColors(idleColor, hoverColor);
    
    multiPlayerButton.setup(400, 340, 200, 50, font, "double");
    multiPlayerButton.setColors(idleColor, hoverColor);
    
    loginButton.setup(50, 50, 120, 40, font, "login");
    loginButton.setColors(idleColor, hoverColor);
    
    leaderboardButton.setup(50, 100, 120, 40, font, "ranking");
    leaderboardButton.setColors(idleColor, hoverColor);
    
    musicToggleButton.setup(850, 50, 120, 40, font, "music:on");
    musicToggleButton.setColors(idleColor, hoverColor);
    
    // 设置按钮回调函数
    tutorialButton.setCallback([this]() {
        currentState = SceneState::TUTORIAL;
    });
    
    singlePlayerButton.setCallback([this]() {
        currentState = SceneState::SINGLE_PLAYER;
    });
    
    multiPlayerButton.setCallback([this]() {
        currentState = SceneState::MULTI_PLAYER;
    });
    
    loginButton.setCallback([this]() {
        // 登录功能待实现
        std::cout << "login button is clicked" << std::endl;
    });
    
    leaderboardButton.setCallback([this]() {
        currentState = SceneState::LEADERBOARD;
    });
    
    musicToggleButton.setCallback([this]() {
        isMusicPlaying = !isMusicPlaying;
        if (isMusicPlaying) {
            backgroundMusic.play();
            musicToggleButton.text.setString("music:on");
        } else {
            backgroundMusic.pause();
            musicToggleButton.text.setString("music:off");
        }
    });
}

void MainScene::initMusic() {
    if (!backgroundMusic.openFromFile("Reflections in Silence.mp3")) {
        std::cout << "背景音乐加载失败，请确保音乐文件存在" << std::endl;
        isMusicPlaying = false;
    } else {
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(50);
        if (isMusicPlaying) {
            backgroundMusic.play();
        }
    }
}

// 更新所有按钮状态的方法
void MainScene::updateButtons(const sf::Vector2f& mousePos) {
    tutorialButton.update(mousePos);
    singlePlayerButton.update(mousePos);
    multiPlayerButton.update(mousePos);
    loginButton.update(mousePos);
    leaderboardButton.update(mousePos);
    musicToggleButton.update(mousePos);
}

// 处理按钮点击的方法
void MainScene::handleButtonClicks(const sf::Vector2f& mousePos) {
    if (tutorialButton.isMouseOver(mousePos)) {
        tutorialButton.callback();
    } else if (singlePlayerButton.isMouseOver(mousePos)) {
        singlePlayerButton.callback();
    } else if (multiPlayerButton.isMouseOver(mousePos)) {
        multiPlayerButton.callback();
    } else if (loginButton.isMouseOver(mousePos)) {
        loginButton.callback();
    } else if (leaderboardButton.isMouseOver(mousePos)) {
        leaderboardButton.callback();
    } else if (musicToggleButton.isMouseOver(mousePos)) {
        musicToggleButton.callback();
    }
}

void MainScene::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            currentState = SceneState::EXIT;
        }
        
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            updateButtons(mousePos); // 使用新方法更新按钮状态
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                handleButtonClicks(mousePos); // 使用新方法处理按钮点击
            }
        }
    }
}

void MainScene::render() {
    window.clear(sf::Color(30, 30, 30));
    
    window.draw(titleText);
    
    tutorialButton.draw(window);
    singlePlayerButton.draw(window);
    multiPlayerButton.draw(window);
    loginButton.draw(window);
    leaderboardButton.draw(window);
    musicToggleButton.draw(window);
    
    window.display();
}

void MainScene::run() {
    while (window.isOpen() && currentState == SceneState::MAIN_MENU) {
        processEvents();
        render();
    }
}

MainScene::SceneState MainScene::getState() const {
    return currentState;
}