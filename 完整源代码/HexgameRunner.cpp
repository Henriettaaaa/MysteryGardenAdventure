#include "HexgameRunner.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "HexGame.h"

// =============== selectGameMode 的实现 ===============
ModeSelection selectGameMode() {
    ModeSelection result;

    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(600, 400), "Hexagonal Game - Select Mode");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "can't load font" << std::endl;
        return result;
    }
    
    // 创建标题文本
    sf::Text titleText("Hexagonal Grid Game", font, 30);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(200, 50);
    
    // 创建按钮
    sf::RectangleShape serverButton(sf::Vector2f(200, 60));
    serverButton.setPosition(200, 150);
    serverButton.setFillColor(sf::Color(103, 135, 85));
    
    sf::Text serverText("Server Mode", font, 20);
    serverText.setFillColor(sf::Color::White);
    serverText.setPosition(235, 170);
    
    sf::RectangleShape clientButton(sf::Vector2f(200, 60));
    clientButton.setPosition(200, 250);
    clientButton.setFillColor(sf::Color(100, 200, 100));
    
    sf::Text clientText("Client Mode", font, 20);
    clientText.setFillColor(sf::Color::White);
    clientText.setPosition(235, 270);
    
    // 客户端输入字段
    bool showClientInput = false;
    std::string ipInput = "127.0.0.1";
    std::string portInput = "54000";
    
    sf::RectangleShape ipInputBox(sf::Vector2f(300, 40));
    ipInputBox.setPosition(150, 150);
    ipInputBox.setFillColor(sf::Color(50, 50, 50));
    ipInputBox.setOutlineThickness(2);
    ipInputBox.setOutlineColor(sf::Color(200, 200, 200));
    
    sf::Text ipInputText(ipInput, font, 20);
    ipInputText.setFillColor(sf::Color::White);
    ipInputText.setPosition(160, 155);
    
    sf::Text ipLabelText("Server IP:", font, 20);
    ipLabelText.setFillColor(sf::Color::White);
    ipLabelText.setPosition(150, 120);
    
    sf::RectangleShape portInputBox(sf::Vector2f(100, 40));
    portInputBox.setPosition(250, 220);
    portInputBox.setFillColor(sf::Color(50, 50, 50));
    portInputBox.setOutlineThickness(2);
    portInputBox.setOutlineColor(sf::Color(200, 200, 200));
    
    sf::Text portInputText(portInput, font, 20);
    portInputText.setFillColor(sf::Color::White);
    portInputText.setPosition(260, 225);
    
    sf::Text portLabelText("Port:", font, 20);
    portLabelText.setFillColor(sf::Color::White);
    portLabelText.setPosition(150, 220);
    
    sf::RectangleShape connectButton(sf::Vector2f(200, 50));
    connectButton.setPosition(200, 300);
    connectButton.setFillColor(sf::Color(150, 150, 255));
    
    sf::Text connectText("Connect", font, 20);
    connectText.setFillColor(sf::Color::White);
    connectText.setPosition(270, 315);
    
    sf::RectangleShape backButton(sf::Vector2f(100, 40));
    backButton.setPosition(50, 300);
    backButton.setFillColor(sf::Color(255, 100, 100));
    
    sf::Text backText("Back", font, 20);
    backText.setFillColor(sf::Color::White);
    backText.setPosition(75, 310);
    
    // 主循环
    bool inputActive = false;
    bool ipInputSelected = false;
    bool portInputSelected = false;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return result;
            }
            
            // 鼠标点击事件
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                if (!showClientInput) {
                    // 处理主界面按钮
                    if (serverButton.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        result.valid = true;
                        result.isServer = true;
                        window.close();
                        return result;
                    }
                    
                    if (clientButton.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        showClientInput = true;
                    }
                } else {
                    // 处理客户端输入界面
                    if (ipInputBox.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        ipInputSelected = true;
                        portInputSelected = false;
                    } else if (portInputBox.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        ipInputSelected = false;
                        portInputSelected = true;
                    } else {
                        ipInputSelected = false;
                        portInputSelected = false;
                    }
                    
                    if (connectButton.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        result.valid = true;
                        result.isServer = false;
                        result.serverIP = ipInput;
                        try {
                            result.port = (unsigned short)std::stoi(portInput);
                            window.close();
                            return result;
                        } catch (...) {
                            // 端口转换错误，使用默认端口
                            result.port = 54000;
                            window.close();
                            return result;
                        }
                    }
                    
                    if (backButton.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y)) {
                        showClientInput = false;
                    }
                }
            }
            
            // 处理键盘输入
            if (event.type == sf::Event::TextEntered) {
                if (ipInputSelected) {
                    if (event.text.unicode == 8 && ipInput.size() > 0) { // Backspace
                        ipInput.pop_back();
                    } else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                        ipInput += static_cast<char>(event.text.unicode);
                    }
                    ipInputText.setString(ipInput);
                } else if (portInputSelected) {
                    if (event.text.unicode == 8 && portInput.size() > 0) { // Backspace
                        portInput.pop_back();
                    } else if (event.text.unicode >= 48 && event.text.unicode <= 57) { // 仅接受数字
                        portInput += static_cast<char>(event.text.unicode);
                    }
                    portInputText.setString(portInput);
                }
            }
        }
        
        window.clear(sf::Color(169, 189, 126));
        
        if (!showClientInput) {
            // 显示主界面
            window.draw(titleText);
            window.draw(serverButton);
            window.draw(serverText);
            window.draw(clientButton);
            window.draw(clientText);
        } else {
            // 显示客户端输入界面
            window.draw(titleText);
            
            window.draw(ipLabelText);
            window.draw(ipInputBox);
            window.draw(ipInputText);
            
            window.draw(portLabelText);
            window.draw(portInputBox);
            window.draw(portInputText);
            
            window.draw(connectButton);
            window.draw(connectText);
            window.draw(backButton);
            window.draw(backText);
            
            // 更新选中状态显示
            if (ipInputSelected) {
                ipInputBox.setOutlineColor(sf::Color(100, 200, 255));
            } else {
                ipInputBox.setOutlineColor(sf::Color(200, 200, 200));
            }
            
            if (portInputSelected) {
                portInputBox.setOutlineColor(sf::Color(100, 200, 255));
            } else {
                portInputBox.setOutlineColor(sf::Color(200, 200, 200));
            }
        }
        
        window.display();
    }
    
    return result;
}

// =============== Hexgame_run 的实现 ===============
int Hexgame_run(int argc, char* argv[], UserManager* userManager)
{
// 使用图形界面选择模式
ModeSelection mode = selectGameMode();
if (!mode.valid) {
std::cerr << "Can't select valid game mode, program exit." << std::endl;
return 1;
}

// 创建并初始化游戏
HexGame game(mode.isServer, mode.serverIP, mode.port);

// 设置用户管理器
if (userManager) {
    game.setUserManager(userManager);
}

if (!game.initialize()) {
return 1;
}

// 运行游戏
game.run();

return 0;
}