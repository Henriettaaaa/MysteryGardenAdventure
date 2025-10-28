#include "HexGrid.h"
#include <iostream>



int main() {
    int size = input_grid_size();
            
    // 创建并初始化网格
    HexGrid hexGrid(size);
    hexGrid.initialize();
    
    // 运行寻路算法
    hexGrid.run();
    
    // 显示网格
    hexGrid.display();

    return 0;
} 