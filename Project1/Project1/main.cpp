#include "PhysicalMaze.h"
#include "VirtualMaze.h"
#include "MainScene.h"
#include "UserManager.h"
#include "Leaderboard.h"
#include <iostream>

// 全局用户管理器
UserManager userManager;

int main() {
    // 创建并显示主界面
    MainScene mainScene;
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

        //绘制最短路径的虚拟路径
        //virtualMaze.convertToVirtualPath(physicalMaze.getPath());

        // 设置路径（用于相对路径显示）
        virtualMaze.setPath(physicalMaze.getPath());

        // 启用玩家交互功能
        virtualMaze.enablePlayerInteraction();

        // 显示迷宫
        virtualMaze.display();
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
                userManager.updateBestTime(size, elapsedTime);
                std::cout << "timing:" << elapsedTime << " seconds" << std::endl;
            }
        }
        
        break;
    }

    case MainScene::SceneState::TUTORIAL: {
        // 教学模式功能待实现
        std::cout << "teaching" << std::endl;
        break;
    }

    case MainScene::SceneState::LEADERBOARD: {
        // 排行榜功能
        // 要求用户输入迷宫大小
        std::cout << "input maze size: ";
        int mazeSize;
        std::cin >> mazeSize;
        
        // 显示排行榜
        Leaderboard leaderboard(userManager, mazeSize);
        leaderboard.run();
        break;
    }
    
    case MainScene::SceneState::LOGIN: {
        // 登录功能
        userManager.run();
        
        // 登录成功后，重新返回主界面
        if (userManager.isUserLoggedIn()) {
            std::cout << "Welcome," << userManager.getCurrentUser() << "!" << std::endl;
        }
        
        // 重新显示主界面
        mainScene.run();
        
        // 递归调用main，处理主界面的下一个选择
        // 注意：这种方式在生产环境可能不是最佳做法，但这里为了简单演示使用
        return main();
    }

    case MainScene::SceneState::EXIT:
        // 用户关闭了窗口
        break;

    default:
        // 未知状态
        break;
    }

    return 0;
}