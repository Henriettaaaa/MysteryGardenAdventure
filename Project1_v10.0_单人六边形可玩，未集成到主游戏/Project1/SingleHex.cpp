#include "SingleHex.h"
#include <iostream>
#include <algorithm>

// --- 全局变量定义 ---
// 定义六个方向的轴坐标偏移量
const std::vector<HexCoord> AXIAL_DIRECTIONS = {
    HexCoord(+1, 0), HexCoord(+1, -1), HexCoord(0, -1),
    HexCoord(-1, 0), HexCoord(-1, +1), HexCoord(0, +1)
};

// --- 辅助函数实现 ---
// 获取一个六边形的所有邻居坐标
std::vector<HexCoord> get_neighbors(const HexCoord& h) {
    std::vector<HexCoord> neighbors;
    for (const auto& dir : AXIAL_DIRECTIONS) {
        neighbors.push_back(h + dir);
    }
    return neighbors;
}

// 计算两个六边形之间的距离（格子数）
int hex_distance(const HexCoord& a, const HexCoord& b) {
    // 使用立方体坐标计算距离更方便: x=q, z=r, y=-q-r
    int dx = a.q - b.q;
    int dz = a.r - b.r;
    int dy = (-a.q - a.r) - (-b.q - b.r);
    return (std::abs(dx) + std::abs(dy) + std::abs(dz)) / 2;
}

// 将轴坐标转换为像素坐标（屏幕坐标）
sf::Vector2f axial_to_pixel(const HexCoord& h, float size, const sf::Vector2f& origin) {
    float x = size * (std::sqrt(3.0f) * h.q + std::sqrt(3.0f) / 2.0f * h.r);
    float y = size * (3.0f / 2.0f * h.r);
    return sf::Vector2f(x + origin.x, y + origin.y);
}

// 创建一个六边形的 SFML 可绘制形状
sf::ConvexShape create_hexagon_shape(float size, const sf::Vector2f& center) {
    sf::ConvexShape hex(6);
    for (int i = 0; i < 6; ++i) {
        float angle_deg = 60.0f * i - 30.0f; // 从 x 轴正方向逆时针旋转，第一个顶点在 30 度角
        float angle_rad = angle_deg * acos(-1.0f) / 180.0f;
        hex.setPoint(i, sf::Vector2f(center.x + size * std::cos(angle_rad),
            center.y + size * std::sin(angle_rad)));
    }
    hex.setOutlineThickness(1.0f); // 设置边框厚度
    hex.setOutlineColor(sf::Color::Black); // 设置边框颜色
    return hex;
}

// 获取对应方向的坐标偏移
HexCoord get_direction_offset(Direction dir) {
    switch (dir) {
        case Direction::Right:     return AXIAL_DIRECTIONS[0]; // (+1, 0)
        case Direction::UpRight:   return AXIAL_DIRECTIONS[1]; // (+1, -1)
        case Direction::UpLeft:    return AXIAL_DIRECTIONS[2]; // (0, -1)
        case Direction::Left:      return AXIAL_DIRECTIONS[3]; // (-1, 0)
        case Direction::DownLeft:  return AXIAL_DIRECTIONS[4]; // (-1, +1)
        case Direction::DownRight: return AXIAL_DIRECTIONS[5]; // (0, +1)
        default:                   return HexCoord(0, 0);
    }
}

// 移动玩家位置
bool move_player(Direction dir, HexCoord& player_pos, const std::unordered_map<HexCoord, CellState>& grid_data) {
    // 获取移动方向的坐标偏移
    HexCoord offset = get_direction_offset(dir);
    
    // 计算移动后的新位置
    HexCoord new_pos = player_pos + offset;
    
    // 检查新位置是否有效（在网格内）
    if (grid_data.find(new_pos) != grid_data.end()) {
        // 更新玩家位置
        player_pos = new_pos;
        return true;
    }
    
    // 如果新位置无效（超出边界），则不移动
    return false;
}

// 检查玩家是否到达终点
bool check_win(const HexCoord& player_pos, const HexCoord& end_pos) {
    return player_pos == end_pos;
}

// --- 核心函数实现 ---
// 获取用户输入的网格大小
int input_grid_size() {
    int GRID_RADIUS;
    std::cout << "input size: ";
    std::cin >> GRID_RADIUS;
    
    // 验证输入合法性
    if (GRID_RADIUS < 3) {
        std::cout << "too small, set to 3" << std::endl;
        GRID_RADIUS = 3;
    } else if (GRID_RADIUS > 20) {
        std::cout << "too large, set to 20" << std::endl;
        GRID_RADIUS = 20;
    }
    
    return GRID_RADIUS;
}

// 生成六边形网格
void generate_hex_grid(int radius, 
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      std::vector<HexCoord>& boundary_cells) {
    std::cout << "Generating hex grid..." << std::endl;
    
    // 清空之前的数据
    grid_data.clear();
    boundary_cells.clear();
    
    // 生成网格
    for (int q = -radius; q <= radius; ++q) {
        for (int r = -radius; r <= radius; ++r) {
            HexCoord current_hex(q, r);
            // 检查是否在大六边形边界内
            if (hex_distance(HexCoord(0, 0), current_hex) <= radius) {
                grid_data[current_hex] = CellState::Empty; // 初始化为空白
                
                // 检查是否是边界单元格
                if (hex_distance(HexCoord(0, 0), current_hex) == radius) {
                    boundary_cells.push_back(current_hex);
                }
            }
        }
    }
    
    std::cout << "Grid generated with " << grid_data.size() << " cells." << std::endl;
    if (boundary_cells.empty()) {
        std::cerr << "Error: No boundary cells found. Grid radius might be too small." << std::endl;
        throw std::runtime_error("No boundary cells found");
    }
    std::cout << "Found " << boundary_cells.size() << " boundary cells." << std::endl;
}

// 选择起点和终点
void select_start_end(int radius,
                      const std::vector<HexCoord>& boundary_cells,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      HexCoord& start_hex,
                      HexCoord& end_hex,
                      std::mt19937& rng) {
    if (boundary_cells.size() >= 2) {
        // 随机选择起点
        std::uniform_int_distribution<int> dist(0, boundary_cells.size() - 1);
        int start_index = dist(rng);
        start_hex = boundary_cells[start_index];

        // 随机选择终点，确保与起点不同且距离较远
        int end_index;
        int attempts = 0;
        const int max_attempts = 100; // 防止无限循环
        
        do {
            end_index = dist(rng);
            end_hex = boundary_cells[end_index];
            attempts++;
        } while (attempts < max_attempts && (end_hex == start_hex || hex_distance(start_hex, end_hex) < radius));

        // 如果尝试多次后仍然没找到合适的终点，随便选一个不同的
        if (end_hex == start_hex) {
            std::cout << "Warning: Could not find a distant end point after " << max_attempts 
                      << " attempts. Choosing a different random one." << std::endl;
            do {
                end_index = dist(rng);
            } while (boundary_cells[end_index] == start_hex);
            end_hex = boundary_cells[end_index];
        }

        // 设置起点和终点状态
        grid_data[start_hex] = CellState::Start;
        grid_data[end_hex] = CellState::End;
        
        std::cout << "Start hex selected at (" << start_hex.q << ", " << start_hex.r << ")" << std::endl;
        std::cout << "End hex selected at (" << end_hex.q << ", " << end_hex.r << ")" << std::endl;
    }
    else {
        std::cerr << "Error: Not enough boundary cells to select start and end points." << std::endl;
        throw std::runtime_error("Not enough boundary cells");
    }
}

//固定位置的起点和终点
void fix_start_end(int radius,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      HexCoord& start_hex,
                      HexCoord& end_hex){
    start_hex = HexCoord(-radius, 0);
    end_hex = HexCoord(radius, 0);
    grid_data[start_hex] = CellState::Start;
    grid_data[end_hex] = CellState::End;
}

// 寻路函数，使用随机DFS且禁止回头
bool findPath(const HexCoord& start, const HexCoord& end, 
              std::unordered_map<HexCoord, CellState>& grid_data,
              std::unordered_map<HexCoord, HexCoord>& came_from,
              std::unordered_set<HexCoord>& visited,
              std::mt19937& rng) {
    
    // 使用栈来模拟DFS过程
    std::vector<HexCoord> stack;
    stack.push_back(start);
    visited.insert(start);
    
    // 记录每个节点的前一个节点，用于避免回头
    std::unordered_map<HexCoord, HexCoord> previous;
    previous[start] = start; // 起点的前一个节点设为自己
    
    while (!stack.empty()) {
        HexCoord current = stack.back();
        stack.pop_back();
        
        // 如果找到终点，返回成功
        if (current == end) {
            return true;
        }
        
        // 获取所有邻居并随机排序
        std::vector<HexCoord> neighbors = get_neighbors(current);
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        
        // 遍历所有邻居
        for (const auto& next : neighbors) {
            // 检查邻居是否在网格内且未被访问
            if (grid_data.count(next) && visited.find(next) == visited.end()) {
                // 记录来源以便重建路径
                came_from[next] = current;
                visited.insert(next);
                
                // 记录此邻居的前一个节点，用于后续防止回头
                previous[next] = current;
                
                // 将邻居加入栈中
                stack.push_back(next);
            }
        }
    }
    
    // 如果搜索完所有可能的路径但没有找到终点，返回失败
    return false;
}

// 重建并标记路径
void reconstruct_path(const HexCoord& start_hex, 
                      const HexCoord& end_hex,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      std::unordered_map<HexCoord, HexCoord>& came_from,
                      std::unordered_set<HexCoord>& visited,
                      std::vector<HexCoord>& path_cells) {
    std::cout << "Reconstructing path..." << std::endl;
    
    // 清空之前的路径
    path_cells.clear();
    
    HexCoord current = end_hex;
    
    // 先收集路径上的所有格子
    while (current != start_hex) {
        if (came_from.count(current)) {
            path_cells.push_back(current);
            current = came_from[current];
        }
        else {
            std::cerr << "Error: Path reconstruction failed. Missing link." << std::endl;
            path_cells.clear();
            return;
        }
    }
    path_cells.push_back(start_hex);  //现在绝对路径里面包含起点和终点，起点在最后面，终点在最前面
    // 确保所有路径格子恢复到Empty状态
    for (const auto& cell : visited) {
        if (cell != start_hex && cell != end_hex && grid_data.count(cell)) {
            grid_data[cell] = CellState::Empty;
        }
    }
    
    // 只标记最终路径上的格子
    /*
    for (const auto& cell : path_cells) {
        if (cell != end_hex && cell != start_hex) {
            grid_data[cell] = CellState::Path;
        }
    }
    */
    
    // 恢复起点和终点的状态
    grid_data[start_hex] = CellState::Start;
    grid_data[end_hex] = CellState::End;
    
    std::cout << "Path reconstruction complete. Path length: " << path_cells.size() + 1 << std::endl;
}

// 计算相对路径
void calculateHexRelative(const HexCoord& start_hex,
                             const HexCoord& end_hex,
                             const std::vector<HexCoord>& path_cells,
                             std::vector<HexCoord>& relative_path_cells,
                             std::unordered_map<HexCoord, CellState>& grid_data){
    int B = 1, A = path_cells.size() - 2;  //左右两个指针，分别从两边走绝对路径数组
    HexCoord moveA = start_hex;  //相对路径的初始位置
    // 清空之前的相对路径
    relative_path_cells.clear();
    //std::cout << moveA << std::endl;
    while (B <= A) {  //还没相遇时
        
        int moveR_A = path_cells[A].r - path_cells[A + 1].r;  //A绝对移动的r轴
        int moveQ_A = path_cells[A].q - path_cells[A + 1].q;  //A绝对移动的q轴
        int moveR_B = path_cells[B].r - path_cells[B - 1].r;  //B绝对移动的r轴
        int moveQ_B = path_cells[B].q - path_cells[B - 1].q;  //B绝对移动的q轴

        //初始的相对移动的格子是相对于起点，后面的都是相对于前一个相对格子        
        //以B为参考，A的移动
        moveA.r = moveA.r + moveR_A - moveR_B;
        moveA.q = moveA.q + moveQ_A - moveQ_B;
        relative_path_cells.push_back(moveA);
        //std::cout << "relative" << A << " " << moveA << std::endl;
        //更新A和B，A往前走，B往后走
        B++;
        A--;
    }

    //标记相对路径上的格子
    for (const auto& cell : relative_path_cells) {
        if (cell != end_hex && cell != start_hex) {
            grid_data[cell] = CellState::Relative;
        }
    }
}

// 渲染网格
void render_grid(sf::RenderWindow& window,
                const sf::Vector2f& grid_origin,
                const std::unordered_map<HexCoord, CellState>& grid_data) {
    // 清屏
    window.clear(sf::Color(50, 50, 50)); // 深灰色背景
    
    // 绘制所有六边形
    for (const auto& pair : grid_data) {
        const HexCoord& coord = pair.first;
        const CellState& state = pair.second;
        
        sf::Vector2f center = axial_to_pixel(coord, HEX_SIZE, grid_origin);
        sf::ConvexShape hex_shape = create_hexagon_shape(HEX_SIZE * 0.95f, center); // 缩小一点点以显示缝隙
        
        // 根据状态设置颜色
        switch (state) {
            case CellState::Empty:
                hex_shape.setFillColor(sf::Color(200, 200, 200)); // 浅灰色
                break;
            case CellState::Start:
                hex_shape.setFillColor(sf::Color::Green); // 绿色起点
                break;
            case CellState::End:
                hex_shape.setFillColor(sf::Color::Red);   // 红色终点
                break;
            case CellState::Path:
                hex_shape.setFillColor(sf::Color::Yellow); // 黄色路径
                break;
            case CellState::Player:
                hex_shape.setFillColor(sf::Color::Blue);  // 蓝色玩家
                break;
            case CellState::Relative:
                hex_shape.setFillColor(sf::Color::Magenta); // 紫色相对路径
                break;
        }
        
        window.draw(hex_shape);
    }
}

// 渲染带序号的网格
void render_grid_with_numbers(sf::RenderWindow& window,
                const sf::Vector2f& grid_origin,
                const std::unordered_map<HexCoord, CellState>& grid_data,
                const std::unordered_map<HexCoord, std::vector<int>>& grid_numbers) {
    // 先渲染基础网格
    render_grid(window, grid_origin, grid_data);
    
    // 创建用于显示序号的字体
    static sf::Font font;
    static bool fontLoaded = false;
    
    // 首次加载字体
    if (!fontLoaded) {
        // 尝试加载系统默认字体
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cerr << "fail" << std::endl;
            return;
        }
        fontLoaded = true;
    }
    
    // 为每个有序号的格子添加序号文本
    for (const auto& pair : grid_numbers) {
        const HexCoord& coord = pair.first;
        const std::vector<int>& numbers = pair.second;
        
        // 跳过没有序号的格子
        if (numbers.empty()) {
            continue;
        }
        
        // 将序号转换为字符串
        std::string numberText;
        for (size_t i = 0; i < numbers.size(); ++i) {
            numberText += std::to_string(numbers[i]);
            if (i < numbers.size() - 1) {
                numberText += ",";
            }
        }
        
        // 创建文本对象
        sf::Text text;
        text.setFont(font);
        text.setString(numberText);
        text.setCharacterSize(12); // 设置适当的大小
        text.setFillColor(sf::Color::Black);
        
        // 计算格子中心位置
        sf::Vector2f center = axial_to_pixel(coord, HEX_SIZE, grid_origin);
        
        // 计算文本大小以居中显示
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(center);
        
        // 绘制文本
        window.draw(text);
    }
} 