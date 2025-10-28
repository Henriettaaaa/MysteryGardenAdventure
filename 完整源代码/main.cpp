#include "PhysicalMaze.h"
#include "VirtualMaze.h"
#include "MainScene.h"
#include "UserManager.h"
#include "Leaderboard.h"
#include "HexgameRunner.h"
#include <iostream>

// 全局用户管理器
UserManager userManager;

int main() {
    // 创建主界面实例（只创建一次）
    MainScene mainScene;
    bool shouldExit = false;
    
    // 主游戏循环，替代递归调用
    while (!shouldExit) {
        // 显示主界面
        mainScene.run();
        
        // 根据用户在主界面的选择执行不同的操作
        switch (mainScene.getState()) {
        case MainScene::SceneState::MULTI_PLAYER: {
            // 执行原来的迷宫逻辑 - 双人模式
            // 获取迷宫大小
            int size = PhysicalMaze::inputSize();

            // 创建并生成物理迷宫
            PhysicalMaze physicalMaze(size);
            physicalMaze.generateMaze(size);

            // 寻找最短路径
            physicalMaze.findPath(size);

            // 创建虚拟迷宫并从物理迷宫构建
            VirtualMaze virtualMaze(size);
            virtualMaze.buildFromPhysical(physicalMaze);

            // 设置路径（用于相对路径显示）
            virtualMaze.setPath(physicalMaze.getPath());

            // 启用玩家交互功能
            virtualMaze.enablePlayerInteraction();

            // 显示迷宫 - 第一阶段：双人寻找对方
            virtualMaze.displayMultiPlayer();
            
            // 第二阶段会在displayMultiPlayer内部触发，检测到玩家相遇后自动跳转到displayRelativeMaze
            
            // 游戏结束后，重置主界面状态，准备下一次循环
            //更新排行榜
            mainScene.resetState();
            break;
        }


        case MainScene::SceneState::SINGLE_PLAYER: {
            // 单人模式功能
            // 获取迷宫大小
            int size = PhysicalMaze::inputSize();

            // 创建并生成物理迷宫
            PhysicalMaze physicalMaze(size);
            physicalMaze.generateMaze(size);

            // 寻找最短路径
            physicalMaze.findPath(size);

            // 创建虚拟迷宫并从物理迷宫构建
            VirtualMaze virtualMaze(size);
            virtualMaze.buildFromPhysical(physicalMaze);

            // 设置路径（用于相对路径显示）
            virtualMaze.setPath(physicalMaze.getPath());

            // 启用玩家交互
            virtualMaze.enablePlayerInteraction();

            // 记录开始时间
            float startTime = 0.0f;
            
            // 显示迷宫
            virtualMaze.displaySingle();
            
            // 如果用户已登录，更新最佳时间
            if (userManager.isUserLoggedIn()) {
                float elapsedTime = virtualMaze.getElapsedTime();
                // 如果游戏完成（时间不为0），更新记录
                if (elapsedTime > 0.0f) {
                    // 根据迷宫大小确定难度
                    Difficulty diff;
                    if (size == 5) diff = Difficulty::EASY;
                    else if (size == 7) diff = Difficulty::MEDIUM;
                    else if (size == 9) diff = Difficulty::HARD;
                    else diff = Difficulty::MEDIUM; // 默认
                    
                    userManager.updateSingleSquareBestTime(diff, elapsedTime);
                    std::cout << "timing:" << elapsedTime << " seconds" << std::endl;
                }
            }
            
            // 重置状态
            mainScene.resetState();
            break;
        }

        case MainScene::SceneState::SINGLE_PLAYER_HEX: {
            // 双人六边形局域网联网实现
            Hexgame_run(0, nullptr, &userManager);
            
            // 重置状态
            mainScene.resetState();
            break;
        }

        case MainScene::SceneState::TUTORIAL: {
            // 教学模式功能待实现
            std::cout << "teaching" << std::endl;
            
            // 使用系统默认播放器打开视频文件
            std::string videoPath = "teaching video.mp4";
            
            // 构建系统命令来打开视频文件
            std::string command = "start \"\" \"" + videoPath + "\"";
            
            // 执行系统命令打开视频
            int result = system(command.c_str());
            if (result != 0) {
                std::cerr << "Failed to open video file: " << videoPath << std::endl;
                std::cout << "Please make sure the video file exists in the same directory." << std::endl;
            } else {
                std::cout << "Video opened successfully!" << std::endl;
            }
            
            // 重置状态
            mainScene.resetState();
            break;
        }

        case MainScene::SceneState::LEADERBOARD: {
            // 排行榜功能 - 不再需要输入迷宫大小
            Leaderboard leaderboard(userManager);
            leaderboard.run();
            
            // 重置状态
            mainScene.resetState();
            break;
        }
        
        case MainScene::SceneState::LOGIN: {
            // 登录功能
            userManager.run();
            
            // 登录成功后，重新返回主界面
            if (userManager.isUserLoggedIn()) {
                std::cout << "Welcome," << userManager.getCurrentUser() << "!" << std::endl;
            }
            
            // 重置状态
            mainScene.resetState();
            break;
        }

        case MainScene::SceneState::EXIT:
            // 用户关闭了窗口，结束主循环
            shouldExit = true;
            break;

        default:
            // 未知状态，结束主循环
            shouldExit = true;
            break;
        }
    }

    return 0;
}