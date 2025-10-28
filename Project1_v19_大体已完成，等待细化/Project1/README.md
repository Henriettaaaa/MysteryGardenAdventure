# 多模块迷宫游戏系统 v2.4

## 项目概述

这是一个集成了用户系统、多种游戏模式和统一排行榜的多模块迷宫游戏项目。支持单人/双人模式，不同难度设置，以及完整的数据统计功能。

## 核心功能模块

### 1. 用户管理系统 (UserManager)
- **用户注册/登录**: 基于用户名密码的身份验证系统
- **数据持久化**: 用户信息和游戏数据保存在`users.txt`文件中
- **数据结构重构**: 支持新的难度系统和多种游戏模式数据记录

#### 新增难度枚举
```cpp
enum class Difficulty {
    EASY = 0,     // 简单 (5x5迷宫/半径4六边形)
    MEDIUM = 1,   // 中等 (7x7迷宫/半径5六边形)  
    HARD = 2      // 困难 (9x9迷宫/半径7六边形)
};
```

#### 新增游戏模式枚举
```cpp
enum class GameMode {
    SINGLE_SQUARE = 0,  // 单人正方形迷宫
    MULTI_SQUARE = 1,   // 双人正方形迷宫
    MULTI_HEX = 2       // 双人六边形迷宫
};
```

### 2. 游戏数据结构 (GameData)
每个用户现在可以记录以下类型的数据：

#### 单人正方形迷宫 (singleSquareBestTimes)
- 记录每个难度的最佳通关时间（秒）
- 自动根据迷宫大小映射难度：5x5→Easy, 7x7→Medium, 9x9→Hard

#### 双人正方形迷宫 (multiSquareStats)  
- 记录每个难度的胜利次数和总游戏次数
- 用于计算胜率统计
- 只统计登录玩家（玩家A，左侧窗口）的胜负结果
- 游戏流程：第一阶段寻找对方 → 第二阶段相对路径竞赛
- 胜利条件：玩家A到达起点(1,1)获胜，玩家B到达终点获胜
- 平局按败北处理

#### 双人六边形迷宫 (multiHexBestTimes & multiHexStats)
- **时间记录**: 记录每个难度的最佳通关时间（秒）
- **胜负统计**: 记录每个难度的胜利次数/总游戏次数和胜率
- **联网模式**: 支持局域网双人对战，两个独立窗口各自登录计时
- **胜利判定**: 玩家计数器先达到路径总长度者获胜，非到达终点判定
- **计数器机制**: 窗口左上角显示Player 1/Player 2的进度计数器
- **数据记录**: 双方玩家都记录胜负统计，获胜方额外记录最佳时间
- **自动关闭**: 产生赢家后3秒自动关闭游戏窗口，返回主界面
- **本地权限**: 客户端有权限直接更新本地登录玩家数据
- **难度映射**: 半径4→Easy, 半径5→Medium, 半径7→Hard

### 3. 六边形游戏结束逻辑详解

#### 胜利检测机制
```cpp
// 服务器端获胜检测 (Player 1)
if (state.checkA >= state.pathA.size() && !state.gameEnded && !winDataRecorded) {
    // 立即停止游戏，记录时间
    state.gameEnded = true;
    float endTime = gameClock.getElapsedTime().asSeconds();
    
    // 发送胜利信号给客户端
    networkManager.sendGameEndTime(endTime);
    
    // 记录获胜方数据：时间 + 胜率
    userManager->updateMultiHexBestTime(diff, endTime);
    userManager->updateMultiHexGameResult(diff, true);
}

// 客户端获胜检测 (Player 2)  
if (state.checkB >= state.pathB.size() && !state.gameEnded && !winDataRecorded) {
    // 同样的逻辑...
}
```

#### 败北数据记录机制
```cpp
// 网络回调中接收到败北信号时
networkManager.setGameStateCallback([this](bool gameStarted, float gameTime, bool gameEnded) {
    if (gameEnded && !winDataRecorded) {
        // 败北方只记录胜负统计，不记录时间
        userManager->updateMultiHexGameResult(diff, false);
        winDataRecorded = true;
    }
});
```

#### 游戏结束流程
1. **胜利检测**: 检查计数器是否达到路径长度
2. **立即停止**: 设置 `gameEnded = true`，停止游戏时钟
3. **网络通信**: 获胜方通过 `sendGameEndTime()` 发送胜利信号
4. **数据记录**: 
   - 获胜方：记录最佳时间 + 胜率统计
   - 败北方：仅记录胜率统计
5. **防重复**: `winDataRecorded` 标志确保每局只记录一次
6. **倒计时**: 启动3秒胜利倒计时器
7. **自动关闭**: 倒计时结束后自动关闭窗口

#### 窗口关闭与返回主界面
```cpp
// 在 HexGame::update() 中
if (victoryTimerStarted && victoryTimer.getElapsedTime().asSeconds() >= 3.0f) {
    std::cout << "Victory timer expired, closing game window..." << std::endl;
    window.close();
}

// 在 HexgameRunner.cpp 中，游戏结束后自动返回
int Hexgame_run(...) {
    HexGame game(mode.isServer, mode.serverIP, mode.port);
    game.setUserManager(userManager);
    game.initialize();
    game.run(); // 游戏循环，直到窗口关闭
    return 0;   // 返回后主程序继续，回到主界面
}
```

#### 网络通信结束逻辑
```cpp
// HexGame析构函数中的清理
HexGame::~HexGame() {
    // 确保窗口关闭
    if (window.isOpen()) {
        window.close();
    }
    
    // 延迟清理避免竞态条件
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 清理网络资源
    networkManager.close();
}
```

#### 数据记录权限管理
- **本地数据**: 客户端和服务器各自拥有写入本地用户数据的完整权限
- **无需网络传输**: 玩家数据不需要在网络间传输，各自保存到本地 `users.txt`
- **登录验证**: 只有已登录用户的数据才会被记录
- **文件锁定**: 数据保存使用文件锁机制确保数据一致性

### 4. 统一表格式排行榜系统 (Leaderboard)

#### 设计理念
- **一页显示全部数据**: 所有玩家的所有游戏数据在同一页面展示
- **横纵坐标固定**: 纵轴为玩家名，横轴为各种数据项目
- **列标题排序**: 点击任意列标题可按该项目排序
- **整行数据联动**: 排序时玩家的所有数据同时移动

#### 表格列结构
| 列名 | 说明 | 排序规则 |
|------|------|----------|
| Player | 玩家用户名 | 按注册顺序排序 |
| S-Easy | 单人简单难度最佳时间 | 时间从短到长 |
| S-Med | 单人中等难度最佳时间 | 时间从短到长 |
| S-Hard | 单人困难难度最佳时间 | 时间从短到长 |
| MS-Easy | 双人正方形简单难度胜率 | 胜率从高到低 |
| MS-Med | 双人正方形中等难度胜率 | 胜率从高到低 |
| MS-Hard | 双人正方形困难难度胜率 | 胜率从高到低 |
| HT-Easy | 双人六边形简单难度最佳时间 | 时间从短到长 |
| HT-Med | 双人六边形中等难度最佳时间 | 时间从短到长 |
| HT-Hard | 双人六边形困难难度最佳时间 | 时间从短到长 |
| HS-Easy | 双人六边形简单难度胜率 | 胜率从高到低 |
| HS-Med | 双人六边形中等难度胜率 | 胜率从高到低 |
| HS-Hard | 双人六边形困难难度胜率 | 胜率从高到低 |

#### 交互功能
- **列标题点击排序**: 点击任意列标题按该项目重新排序
- **鼠标滚轮滚动**: 支持上下滚动查看更多玩家数据
- **数据格式化**: 
  - 时间显示为 `MM:SS` 或 `SS.mmm` 格式
  - 胜率显示为 `胜利次数/总次数 百分比%` 格式
  - 无数据显示为 `-`

### 5. 数据存储格式

#### 新数据格式
```
[username]
password=password
single_square_time:easy:23.456
single_square_time:medium:45.123
multi_square_stats:easy:5:10
multi_hex_time:hard:67.890
multi_hex_stats:medium:3:8
```

#### 向后兼容
系统保持对旧数据格式的兼容：
```
time:5:23.456  # 旧格式：迷宫大小:时间
```

### 6. 游戏流程集成

#### 单人模式数据记录
1. 用户选择迷宫大小 (5/7/9)
2. 系统自动映射到难度 (Easy/Medium/Hard)
3. 游戏结束后调用 `updateSingleSquareBestTime(diff, time)`
4. 只有更好的成绩才会更新记录

#### 双人模式数据记录
- **双人正方形**: 调用 `updateMultiSquareGameResult(diff, won)`
  - 只统计登录玩家（玩家A）的胜负结果
  - 游戏分两阶段：走迷宫寻找对方 → 相对路径竞赛
  - 胜负判定在相对路径阶段完成
  - 登录玩家A到达起点(1,1)为胜利，玩家B到达终点为A败北
  - 平局情况按败北处理
- **双人六边形**: 
  - **双人六边形联网**: 调用 `updateMultiHexBestTime(diff, time)` 和 `updateMultiHexGameResult(diff, won)`
    - **数据记录**: 双方玩家都记录胜负统计，获胜方额外记录最佳时间
    - **本地权限**: 客户端/服务器各自直接更新本地登录玩家数据，无需网络传输
    - **胜利处理流程**:
      - 检测到胜利 → 立即停止计时器 → 记录数据 → 启动3秒倒计时 → 自动关闭窗口
      - 获胜方：调用 `updateMultiHexBestTime()` + `updateMultiHexGameResult(diff, true)`
      - 败北方：仅调用 `updateMultiHexGameResult(diff, false)`
    - **胜利判定机制**: 基于计数器满值判定，非终点到达判定
      - 服务器玩家：`checkA >= pathA.size()` 时获胜
      - 客户端玩家：`checkB >= pathB.size()` 时获胜
      - 计数器通过网络实时同步，确保双方状态一致
    - 根据迷宫半径映射难度：radius=4→Easy, radius=5→Medium, radius=7→Hard
    - 防重复记录：使用`winDataRecorded`标志确保每局游戏只记录一次

### 7. 技术特性

#### 界面优化
- **1200x700 窗口**: 适应表格式排行榜显示
- **12号字体**: 保证13列数据的可读性
- **蓝色列标题**: 区分表头和数据区域
- **滚动支持**: 处理大量玩家数据

#### 性能优化
- **可见区域渲染**: 只渲染屏幕可见的表格行
- **智能排序**: 无效数据(-1)自动排到末尾
- **内存管理**: 动态更新表格UI元素

#### 网络稳定性
- **超时重试**: 网络发送失败时自动重试3次
- **状态同步**: 定期同步检查点数据确保一致性
- **优雅退出**: 游戏结束时正确清理网络资源

### 8. 使用指南

#### 查看排行榜
1. 从主菜单选择"Leaderboard"
2. 查看所有玩家的完整数据表格
3. 点击任意列标题进行排序
4. 使用鼠标滚轮查看更多数据
5. 点击"Return"返回主菜单

#### 六边形联网游戏
1. 两台设备分别启动游戏并登录
2. 一方选择"Server Mode"，另一方选择"Client Mode"
3. 客户端输入服务器IP地址连接
4. 游戏开始后通过WASDEXZ移动
5. 先达到路径终点的玩家获胜
6. 游戏结束3秒后自动返回主界面

#### 数据说明
- **时间数据**: 显示最佳通关时间，越短越好
- **胜率数据**: 显示为"胜/总 百分比%"，如"5/10 50%"
- **空数据**: 显示为"-"，表示该玩家在此项目无记录

## 文件结构

```
Project1/
├── UserManager.h/.cpp     # 用户管理和数据存储
├── Leaderboard.h/.cpp     # 统一表格式排行榜
├── HexGame.h/.cpp         # 双人六边形游戏逻辑
├── NetworkManager.h/.cpp  # 网络通信管理
├── HexgameRunner.h/.cpp   # 六边形游戏启动器
├── main.cpp               # 主程序入口和流程控制
├── users.txt              # 用户数据文件
└── README.md              # 本文档
```

## 开发说明

### 数据流程
1. **登录** → UserManager验证 → 设置当前用户
2. **游戏** → 记录成绩 → 调用相应update方法
3. **排行榜** → 获取所有数据 → 统一表格显示

### 网络架构
1. **服务器端**: 生成迷宫，管理游戏状态，处理客户端连接
2. **客户端**: 接收迷宫数据，发送玩家位置，处理游戏逻辑
3. **双向通信**: 位置更新、游戏状态同步、胜利信号传输

### 扩展性
- 新增游戏模式只需在枚举和数据结构中添加
- 新增难度级别只需修改Difficulty枚举
- 新增数据类型只需在GameData结构中添加对应字段

## 更新历史
- **v2.4**: 完善六边形游戏结束逻辑文档，确保网络通信和窗口关闭的正确性
- **v2.3**: 完善双人六边形联网模式胜负统计功能，添加胜率记录和3秒自动关闭机制
- **v2.2**: 完成双人六边形联网模式胜负统计功能，支持局域网对战和获胜时间记录
- **v2.1**: 完成双人正方形迷宫胜负统计功能，支持三种难度的胜率记录和排行榜显示
- **v2.0**: 重构为统一表格式排行榜，支持所有数据类型同页显示和列排序
- **v1.5**: 添加难度系统和多游戏模式支持
- **v1.0**: 基础用户系统和简单排行榜 