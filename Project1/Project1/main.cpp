#include "PhysicalMaze.h"

int main(){
    int size = PhysicalMaze::inputSize();
    PhysicalMaze maze(size);
    maze.generateMaze(size);
    return 0;
}