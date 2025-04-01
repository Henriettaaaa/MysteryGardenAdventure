//迷宫的底层物理生成
#pragma once

#include <vector>
#include <algorithm>
#include <random>

class PhysicalMaze{  //类定义的时候没有(),函数定义才需要()
private:
    const int size;  //通过带参构造进行初始化
    std::vector<bool> way;  //用于存储迷宫路径
    std::vector<int> parent;  //用于存储每个格子的根节点,自己就是根节点的，parent值=-1
    std::vector<int> weight;  //用于存储每个格子的权重(用于带权重的合并)
    std::vector<std::pair<int,int>> walls;  //用于存储所有墙

public:
    static int inputSize();  // 静态成员函数用于获取尺寸输入
    explicit PhysicalMaze(int s);  //带参构造函数，并防止隐式转换
    int getSize() const { return size; }   //获取迷宫尺寸

    std::vector<std::pair<int,int>> generateWalls(int size);  // 生成墙
    void generateMaze(int s);  // 生成迷宫
    //void printMaze() const;  // 打印迷宫
    //void solveMaze();  // 解决迷宫
    //void printSolution() const;  // 打印解决方案

    int find(int x);  // 查找根节点
    void unionSet(int x, int y, int i);  // 合并

    std::vector<int> findPath(int size);  // 得到迷宫走通的路径

    // 检查两个相邻格子之间是否有墙
    bool hasWall(int cell1, int cell2) const;
    
    // 友元类声明，允许VirtualMaze访问私有成员
    friend class VirtualMaze;
};