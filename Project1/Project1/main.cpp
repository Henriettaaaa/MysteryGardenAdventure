#include "PhysicalMaze.h"
#include "VirtualMaze.h"
#include <iostream>

int main() {
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

    return 0;
}