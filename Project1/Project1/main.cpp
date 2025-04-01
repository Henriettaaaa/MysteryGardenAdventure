#include "PhysicalMaze.h"
#include "VirtualMaze.h"
#include <iostream>

int main() {
    // 获取迷宫大小
    int size = PhysicalMaze::inputSize();
    
    // 创建并生成物理迷宫
    PhysicalMaze physicalMaze(size);
    physicalMaze.generateMaze(size);
    
    // 创建虚拟迷宫并从物理迷宫构建
    VirtualMaze virtualMaze(size);
    virtualMaze.buildFromPhysical(physicalMaze);
    
    // 寻找最短路径
    std::vector<int> path = physicalMaze.findPath(size);
    
    // 显示迷宫和路径
    virtualMaze.highlightPath(path);
    virtualMaze.display();
    
    return 0;
}