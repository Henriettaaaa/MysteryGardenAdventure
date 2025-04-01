#include <iostream>
#include <queue>

#include "PhysicalMaze.h"

int PhysicalMaze::inputSize() {
    int s;  //迷宫尺寸
    std::cout << "please input the size of the maze: ";
    std::cin >> s;
    return s;
}

//*初始化迷宫参数
PhysicalMaze::PhysicalMaze(int s) : size(s) {
    
    way.resize(size * size);
    parent.resize(size * size);
    weight.resize(size * size);

    for(int i = 0; i < size * size; i++) {
            parent[i] = -1;  // 初始化parent数组，每个节点的父节点初始为-1
            weight[i] = 1;   // 初始化权重为1
            way[i] = 0;      // 初始化way数组
        }
}

//*生成迷宫
void PhysicalMaze :: generateMaze(int size){
    std::vector<std::pair<int,int>> walls = generateWalls(size);  //生成随机打乱的墙的数组
    //从最后面往前遍历，这样不影响删边
    for (int i = walls.size() - 1; i >= 0; i--){
        int x = walls[i].first;
        int y = walls[i].second;
        unionSet(x,y,i);
    }
    std::cout << "maze generated!" << std::endl;
}

/*
这种实现使用现代的 C++ 17随机数生成器，产生的随机数质量更好
线程安全
*/
//给所有格子随机打乱顺序
//返回一个随机数组
//*提取并表示每两个相邻格子之间的墙
//*返回一个随机打乱的墙的数组
std::vector<std::pair<int,int>> PhysicalMaze :: generateWalls(int size) {
    //std::vector<std::pair<int,int>> walls;
    
    // 生成所有的墙 
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int current = i * size + j;
            
            // 添加右边的墙
            if (j < size-1) {
                walls.emplace_back(current, current + 1);
            }
            
            // 添加下边的墙
            if (i < size-1) {
                walls.emplace_back(current, current + size);
            }
        }
    }
    
    // 随机打乱
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(walls.begin(), walls.end(), gen);
    
    return walls;
}

// !路径压缩找根节点
int PhysicalMaze:: find(int x) {
    if (parent[x] == -1) {
        return x;        // 返回根节点的索引
    }
    parent[x] = find(parent[x]);  // 递归找到根节点，并压缩路径
    return parent[x];   // 返回根节点的索引
}

//*带权重的合并
//结束条件是所有格子都属于同一个等价类
void PhysicalMaze:: unionSet(int x, int y, int i) {
    int root_x = find(x);    // 获取x的根节点
    int root_y = find(y);    // 获取y的根节点
    
    if (root_x == root_y) {  // 如果已经在同一集合中，不删边
        return;
    }
    
    // 按权重合并
    if (weight[root_x] < weight[root_y]) {
        parent[root_x] = root_y;
        weight[root_y] += weight[root_x];
    } else {
        parent[root_y] = root_x;
        weight[root_x] += weight[root_y];
    }
    //删边
    walls.erase(walls.begin() + i);
}

std::vector<int> PhysicalMaze::findPath(int size) {
    std::vector<int> path;  // 存储最终路径
    std::vector<bool> visited(size * size, false);  // 访问标记数组
    std::vector<int> prev(size * size, -1);  // 前驱节点数组，用于重建路径
    std::queue<int> q;  // BFS队列
    
    // 起点为左上角(0)
    int start = 0;
    // 终点为右下角(size*size-1)
    int end = size * size - 1;
    
    // 将起点加入队列
    q.push(start);
    visited[start] = true;
    
    // BFS主循环
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        // 如果到达终点，退出循环
        if (current == end) {
            break;
        }
        
        // 获取当前节点的行列坐标
        int row = current / size;
        int col = current % size;
        
        // 检查四个方向的相邻节点
        // 右、下、左、上的偏移量
        int dr[] = {0, 1, 0, -1};
        int dc[] = {1, 0, -1, 0};
        
        for (int i = 0; i < 4; i++) {
            int newRow = row + dr[i];
            int newCol = col + dc[i];
            
            // 检查是否在迷宫范围内
            if (newRow >= 0 && newRow < size && newCol >= 0 && newCol < size) {
                int next = newRow * size + newCol;
                
                // 如果没有访问过
                if (!visited[next]) {
                    // 检查是否有墙
                    bool hasWall = false;
                    for (const auto& wall : walls) {
                        if ((wall.first == current && wall.second == next) ||
                            (wall.first == next && wall.second == current)) {
                            hasWall = true;
                            break;
                        }
                    }
                    
                    if (!hasWall) {
                        visited[next] = true;
                        prev[next] = current;
                        q.push(next);
                    }
                }
            }
        }
    }
    
    // 从终点回溯到起点，重建路径
    if (visited[end]) {
        for (int at = end; at != -1; at = prev[at]) {
            path.push_back(at);
        }
        // 反转路径，使其从起点开始
        std::reverse(path.begin(), path.end());
    }
    
    return path;
}

bool PhysicalMaze::hasWall(int cell1, int cell2) const {
    // 检查是否存在这面墙
    for (const auto& wall : walls) {
        if ((wall.first == cell1 && wall.second == cell2) ||
            (wall.first == cell2 && wall.second == cell1)) {
            return true;  // 有墙
        }
    }
    return false;  // 没有墙
}
