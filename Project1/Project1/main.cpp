#include "PhysicalMaze.h"
#include "VirtualMaze.h"
#include "MainScene.h"
#include <iostream>

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

            // 显示迷宫
            virtualMaze.display();
            break;
        }
        
        case MainScene::SceneState::SINGLE_PLAYER: {
            // 单人模式功能待实现
            std::cout << "single" << std::endl;
            break;
        }
        
        case MainScene::SceneState::TUTORIAL: {
            // 教学模式功能待实现
            std::cout << "teaching" << std::endl;
            break;
        }
        
        case MainScene::SceneState::LEADERBOARD: {
            // 排行榜功能待实现
            std::cout << "ranking" << std::endl;
            break;
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