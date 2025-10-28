#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <cmath>
#include <functional>

// --- 常量定义 ---
const int WINDOW_WIDTH = 1000; // 窗口宽度
const int WINDOW_HEIGHT = 800; // 窗口高度
const float HEX_SIZE = 20.0f;   // 小六边形的尺寸（中心到顶点的距离）

// --- 六边形轴坐标结构体 ---
struct HexCoord {
    int q, r; // 轴坐标 (q, r)

    // 构造函数
    HexCoord(int q_ = 0, int r_ = 0) : q(q_), r(r_) {}

    // 等于运算符重载
    bool operator==(const HexCoord& other) const {
        return q == other.q && r == other.r;
    }

    // 不等于运算符重载
    bool operator!=(const HexCoord& other) const {
        return !(*this == other);
    }

    // 加法运算符重载
    HexCoord operator+(const HexCoord& other) const {
        return HexCoord(q + other.q, r + other.r);
    }

    // 减法运算符重载
    HexCoord operator-(const HexCoord& other) const {
        return HexCoord(q - other.q, r - other.r);
    }
};

// --- 为 HexCoord 提供哈希函数，以便用于 unordered_map/unordered_set ---
namespace std {
    template <> struct hash<HexCoord> {
        size_t operator()(const HexCoord& h) const {
            // 使用一种简单的组合哈希方法
            size_t hq = hash<int>{}(h.q);
            size_t hr = hash<int>{}(h.r);
            // 结合 Boost::hash_combine 的思想
            return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
        }
    };
}

// --- 单元格状态枚举 ---
enum class CellState {
    Empty,  // 空白
    Start,  // 路径起点
    End,    // 路径终点
    Path,   // 路径部分
    Player, // 玩家位置
    Relative // 相对路径
};

// --- 方向枚举 ---
enum class Direction {
    Right,      // D键 - 右
    Left,       // A键 - 左
    UpRight,    // E键 - 右上
    UpLeft,     // W键 - 左上
    DownRight,  // X键 - 右下
    DownLeft    // Z键 - 左下
};

// --- 辅助函数声明 ---
// 定义六个方向的轴坐标偏移量
extern const std::vector<HexCoord> AXIAL_DIRECTIONS;

// 获取一个六边形的所有邻居坐标
std::vector<HexCoord> get_neighbors(const HexCoord& h);

// 计算两个六边形之间的距离（格子数）
int hex_distance(const HexCoord& a, const HexCoord& b);

// 将轴坐标转换为像素坐标（屏幕坐标）
sf::Vector2f axial_to_pixel(const HexCoord& h, float size, const sf::Vector2f& origin);

// 创建一个六边形的 SFML 可绘制形状
sf::ConvexShape create_hexagon_shape(float size, const sf::Vector2f& center);

// 获取对应方向的坐标偏移
HexCoord get_direction_offset(Direction dir);

// 移动玩家位置
bool move_player(Direction dir, HexCoord& player_pos, const std::unordered_map<HexCoord, CellState>& grid_data);

// 检查玩家是否到达终点
bool check_win(const HexCoord& player_pos, const HexCoord& end_pos);

// --- 核心函数声明 ---
// 获取用户输入的网格大小
int input_grid_size(bool initialClientConnected = false, std::function<bool()> hasClientConnectedFunc = nullptr);

// 寻路函数，使用随机DFS且禁止回头
bool findPath(const HexCoord& start, const HexCoord& end, 
              std::unordered_map<HexCoord, CellState>& grid_data,
              std::unordered_map<HexCoord, HexCoord>& came_from,
              std::unordered_set<HexCoord>& visited,
              std::mt19937& rng);

// 生成六边形网格，并返回网格数据和边界单元格
void generate_hex_grid(int radius, 
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      std::vector<HexCoord>& boundary_cells);

// 选择起点和终点
void select_start_end(int radius,
                      const std::vector<HexCoord>& boundary_cells,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      HexCoord& start_hex,
                      HexCoord& end_hex,
                      std::mt19937& rng);

// 固定位置的起点和终点
void fix_start_end(int radius,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      HexCoord& start_hex,
                      HexCoord& end_hex);

// 重建并标记路径
void reconstruct_path(const HexCoord& start_hex, 
                      const HexCoord& end_hex,
                      std::unordered_map<HexCoord, CellState>& grid_data,
                      std::unordered_map<HexCoord, HexCoord>& came_from,
                      std::unordered_set<HexCoord>& visited,
                      std::vector<HexCoord>& path_cells);

// 计算相对路径
void calculateHexRelative(const HexCoord& start_hex,
                             const HexCoord& end_hex,
                             const std::vector<HexCoord>& path_cells,
                             std::vector<HexCoord>& relative_path_cells,
                             std::unordered_map<HexCoord, CellState>& grid_data);

// 渲染网格
void render_grid(sf::RenderWindow& window,
                const sf::Vector2f& grid_origin,
                const std::unordered_map<HexCoord, CellState>& grid_data);

// 渲染带序号的网格
void render_grid_with_numbers(sf::RenderWindow& window,
                const sf::Vector2f& grid_origin,
                const std::unordered_map<HexCoord, CellState>& grid_data,
                const std::unordered_map<HexCoord, std::vector<int>>& grid_numbers); 