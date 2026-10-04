# cpp-games

6 个独立的 C++ 2D 小游戏合集。每个游戏是一个单独的 Visual Studio 解决方案，代码规模从单文件到十余个头文件不等，涵盖 2D 生存、射击对战、动作平台、联机打字竞速、SDL2 塔防式经营等类型。

本文档是技术纪要：记录每个项目的程序流程、架构与设计模式、自定义类、C++ 特性与资源导入方式、构建运行方法，以及已核实的缺陷。所有结论均标注 `文件:行号`，可回溯到源码。

---

## 1. 仓库总览

### 1.1 目录结构

```
cpp-games/
├── 1.提瓦特幸存者/
│   ├── 1.提瓦特幸存者.sln
│   └── 提瓦特幸存者/
│       ├── 提瓦特幸存者.vcxproj
│       ├── main.cpp              # 全部代码（554 行，8 个类）
│       ├── img/  (34 PNG)
│       └── ...
├── 2.植物明星大乱斗/
│   ├── 2.植物明星大乱斗.sln
│   └── 植物明星大乱斗/            # 22 个 .h + 1 个 .cpp
├── 3.空洞武士/
│   ├── 3.空洞武士.sln
│   └── 空洞武士/                  # 20 个 .h + 13 个 .cpp
├── 4.哈基米大冒险/
│   ├── 4.哈基米大冒险.sln
│   ├── client/                    # 客户端（8 个 .h + 1 个 .cpp）
│   ├── serve/                     # 服务端（1 个 .cpp）
│   ├── thirdparty/httplib.h       # cpp-httplib 单头文件
│   └── 哈基米大冒险/              # 空壳工程，无任何源文件
├── 5.生化危鸡/
│   ├── 5.生化危鸡.sln
│   ├── 生化危鸡/                  # 10 个 .h + 1 个 .cpp
│   └── thirdparty/SDL2*           # SDL2 及扩展，随仓库附带
└── 6.拼好饭传奇/
    ├── 6.拼好饭传奇.sln
    ├── 拼好饭传奇/                # 16 个 .h + 14 个 .cpp
    └── Thirdparty/SDL2*
```

仓库内共 **6 个 `.sln`、8 个 `.vcxproj`**。4 号游戏有 3 个工程（客户端 / 服务端 / 一个不含任何源文件的空壳工程）。

### 1.2 技术栈分界

| 项目 | 渲染/窗口 | 音频 | 网络 | C++ 标准 |
|---|---|---|---|---|
| 1.提瓦特幸存者 | EasyX | MCI | 无 | 未指定（工具集默认） |
| 2.植物明星大乱斗 | EasyX | MCI | 无 | 未指定 |
| 3.空洞武士 | EasyX | MCI | 无 | 未指定 |
| 4.哈基米大冒险 | EasyX | MCI | cpp-httplib | 未指定 |
| 5.生化危鸡 | SDL2 | SDL2_mixer | 无 | 未指定 |
| 6.拼好饭传奇 | SDL2 | SDL2_mixer | 无 | **stdcpp17** |

关键分界：

- **1–4 使用 EasyX**（`#include <graphics.h>`），并在 `util.h` 中以 `#pragma comment(lib, ...)` 手工链接两个 Win32 系统库：`MSIMG32.lib`（提供 `AlphaBlend`，用于透明混合绘制）与 `Winmm.lib` / `WINMM.lib`（提供 MCI 音频命令接口）。
- **5–6 使用 SDL2** 及其 image / mixer / ttf 三个扩展库。
- **EasyX 未随仓库附带**：仓库内不存在 `graphics.h`、`EasyXa.lib`、`conio.h`。SDL2 则完整附带在 `thirdparty/`（5 号）与 `Thirdparty/`（6 号）下，详见 §1.3。

### 1.3 构建配置横向对比

6 个工程的公共配置：

- 无 `<PlatformToolset>` 元素 → 使用 VS 2022 默认工具集 **v143**。
- `<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>`。
- 4 个配置组合：`Debug|Win32`、`Release|Win32`、`Debug|x64`、`Release|x64`。
- **`<SDLCheck>true</SDLCheck>` 出现在全部 8 个工程的每一处**。此项与 SDL2 无关，它启用的是 MSVC 的 `/sdl` 静态分析检查（启用边界检查、指针检查等），是 VS 工程的默认勾选项。判断是否使用 SDL2 只能看包含路径与链接库。
- `<SubSystem>Console</SubSystem>`（控制台子系统，非 Windows）。

差异化配置：

| 项目 | 包含路径 | 链接库 | 生效的配置 | 语言标准 |
|---|---|---|---|---|
| 1–3、4.client/serve | 无 | 仅源码内 `#pragma comment` | 全部 | 未指定 |
| 4.哈基米大冒险（空壳） | 无 | 无 | 全部 | 未指定 |
| 5.生化危鸡 | `..\thirdparty\SDL2{,_image,_mixer,_ttf}\include` | `SDL2.lib SDL2main.lib SDL2_image.lib SDL2_mixer.lib SDL2_ttf.lib` | **仅 Release\|x64** | 未指定 |
| 6.拼好饭传奇 | `..\Thirdparty\SDL2{,_image,_mixer,_ttf}\include` | 同上 | Debug\|x64 与 Release\|x64 | **stdcpp17** |

由上表可直接推出三条构建约束：

1. **1–4 号工程无法直接编译**，必须先安装 EasyX 并手工添加其 include 目录、链接 `EasyXa.lib`（ANSI 版）或 `EasyXw.lib`（Unicode 版）。工程文件里没有留下任何 EasyX 配置，也没有 `.props` 承载它。
2. **5 号工程只有 `Release|x64` 配置了 SDL 路径**，`Debug|x64` 缺失 → 切换到 Debug 配置会报找不到 `SDL.h`。
3. **两个 SDL 工程都只能用 x64 配置**：库路径写死为 `lib\x64`，Win32 配置未提供路径。
4. 附带细节：5 号用小写 `thirdparty`，6 号用大写 `Thirdparty`。Windows 文件系统不敏感，两者都能工作，但属于命名不一致。

### 1.4 设计模式横向矩阵

**图例**

| 符号 | 含义 |
|---|---|
| ✅ | 严格实现：具备该模式要求的全部参与者与协作关系 |
| ◐ | 相似或退化实现：意图一致但结构不完整，或可归入该模式但形态简化 |
| ✗ | 未使用 |

**GoF 23 种设计模式**

| 模式 | 1 | 2 | 3 | 4 | 5 | 6 |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| Abstract Factory | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Builder | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **Factory Method** | ✗ | ✗ | ✅ | ✗ | ◐ | ✗ |
| Prototype | ✗ | ✗ | ◐ | ✗ | ✗ | ✗ |
| **Singleton** | ✗ | ✗ | ✅ ×4 | ✗ | ✗ | ✅ ×3 |
| Adapter | ◐ | ◐ | ✅ | ◐ | ◐ | ◐ |
| Bridge | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Composite | ✗ | ✗ | ◐ | ✗ | ✗ | ✗ |
| Decorator | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Facade | ◐ | ◐ | ✅ | ✗ | ◐ | ✅ |
| **Flyweight** | ◐ | ✅ | ✅ | ✅ | ◐ | ✅ |
| Proxy | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Chain of Responsibility | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Command | ✗ | ◐ | ✗ | ✗ | ◐ | ◐ |
| Interpreter | ✗ | ✗ | ◐ | ✗ | ✗ | ✗ |
| Iterator | ✗ | ✗ | ✗ | ✗ | ✗ | ◐ |
| **Mediator** | ✗ | ✗ | ✗ | ✗ | ✗ | ✅ |
| Memento | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Observer | ✗ | ◐ | ◐ | ✗ | ◐ | ✗ |
| **State** | ◐ | ✗ | ✅ | ◐ | ◐ | ◐ |
| Strategy | ✗ | ✅ | ◐ | ◐ | ✗ | ✗ |
| **Template Method** | ✅ | ◐ | ✅ | ✗ | ◐ | ◐ |

**非 GoF 的架构手法**（这些才是本仓库真正反复使用的解耦手段）

| 手法 | 1 | 2 | 3 | 4 | 5 | 6 | 说明 |
|---|:---:|:---:|:---:|:---:|:---:|:---:|---|
| 回调注入（`std::function` 单槽） | ✗ | ✅ | ✅ | ◐ | ✅ | ✅ | 计时器、碰撞、动画结束统一走这个 |
| 资源集中缓存 + 字符串索引 | ✗ | ✗ | ✅ | ✗ | ◐ | ✅ | 3/6 号有真正的查找表 |
| 事件广播（一个输入分发给多个对象） | ✗ | ✗ | ◐ | ✗ | ✗ | ✅ | 6 号最完整；3 号由 `CollisionManager` 扇出。2 号 `game_scene.h:113-126` 是硬编码点名成员调用，属反向的 fan-out 而非广播 |
| 所有权反转（实体持有管理器） | ✗ | ✗ | ✅ | ✗ | ✗ | ✗ | 3 号的 `hit_box`/`hurt_box` |
| 枚举驱动的阶段流 | ◐ | ✗ | ✗ | ✅ | ◐ | ✗ | 4 号 `Stage`、5 号双指针 |
| 全局裸指针 / 全局对象充当服务定位 | ◐ | ◐ | ✗ | ✗ | ◐ | ✗ | 退化形态的服务定位器 |
| 组合式继承（角色 → 行为） | ✅ | ✅ | ✅ | ✗ | ✅ | ◐ | |
| 数据驱动资源描述表 | ✗ | ✗ | ✅ | ✗ | ◐ | ✗ | 3 号 `ImageResInfo`/`AtlasResInfo` |

### 1.5 全局结论

1. **享元（Flyweight）是唯一被 6 个项目全部采用的思想**：所有项目都把图片帧、动画图集、音频这类"可共享的不变数据"从逻辑对象里剥离出来统一持有。但只有 3 号和 6 号做成了带查找表的资源管理器，1 号和 2 号是裸全局指针，5 号是半成品，因此矩阵中前者标 ✅、后者标 ◐。
2. **工厂（Factory Method）是全仓库最薄弱的环节**。6 个项目全部用 `new` 直接构造对象。唯一名副其实的工厂方法在 3 号：`CollisionBox` 的构造函数与析构函数都是 private（`collision_box.h:48-49`），只声明了 `friend class CollisionManager`（`collision_box.h:8`），构造入口被强制收敛到 `CollisionManager::create_collision_box()`（`collision_manager.cpp:15-19`）。5 号有一处内联 if/else 按权重选敌人类型，形态接近简单工厂，但没有独立工厂角色。
3. **单例只出现在 3 号（4 个）和 6 号（3 个）**，且两处写法完全一致：静态裸指针 + `if (!manager) manager = new X();` 的懒加载，既非线程安全，也未处理重复创建。1、2、4、5 号用全局变量 / 全局对象承担同样的职责。
4. **状态模式只有 3 号是严格实现**：19 个 `StateNode` 子类 + 按 id 索引的状态表 + `on_enter`/`on_update`/`on_exit` 三段生命周期。其余项目都是枚举或条件分支驱动的阶段流，详见 §8 常见误判。
5. **回调注入是最主要的解耦手段**，出现在 2、3、5、6 号。但所有回调槽都是**单槽**（一个 `std::function<void()>` 成员），不是观察者。
6. **4 号是全仓库唯一零继承的项目**，没有任何类派生关系，也没有虚函数，只有一个 `Timer` 内部用到的 `std::function`。
7. **中介者（Mediator）全仓库只出现一次**，在 6 号（`CursorMgr`），也是该项目中耦合最松的部分。

---

## 2. 1.提瓦特幸存者

### 2.1 玩法与程序流程

单文件实现（`main.cpp`，554 行）的俯视角生存游戏：玩家横向移动射击，敌人从两侧刷新，存活时间越长分数越高。

程序流程为单层 `while(1)` 无限循环，每帧依次：

1. `peekmessage` 抽取并处理输入队列。
2. 逐个 `enemy->on_update(delta)`，其中执行随机行为分支（`main.cpp:228-232`）。
3. 遍历子弹与敌人做手工 AABB 相交判定（`Enemy::CheckBulletCollision`，`main.cpp:239` 起）。
4. 清理死亡敌人：`delete enemy`（`main.cpp:512`）。
5. 全局资源清理（`main.cpp:548-551`）。

动画由 `Animation` 对象驱动，`Player` 构造时硬编码帧数 `new Animation(atlas_player_right, 45)`（`main.cpp:87`）。

### 2.2 架构与设计模式

项目全部代码在 `main.cpp` 内，无头文件拆分。

- **模板方法（Template Method）✅** — `Button::ProcessEvent`（`main.cpp:311`）是具体方法，内部 `switch (msg.message)` 处理 `WM_MOUSEMOVE`/`WM_LBUTTONDOWN`/`WM_LBUTTONUP` 的完整骨架，在 `WM_LBUTTONUP` 分支调用虚函数 `OnClick()`（`main.cpp:325`）。派生类 `StartGameButton`（`main.cpp:367`）与 `QuitGameButton`（`main.cpp:380`）只重写 `OnClick`，其余行为全部复用基类。这是本项目中唯一严格成立的 GoF 模式。
- **享元（Flyweight）◐** — `atlas_player_left`、`atlas_player_right`、`atlas_enemy_left`、`atlas_enemy_right` 四个全局 `Atlas*`（`main.cpp:50-53`）被所有 `Player`/`Enemy` 实例共享。共享事实成立，但没有池、没有查找表、没有引用计数，属退化形态。
- **适配器（Adapter）◐** — `main.cpp:18-21` 定义 `inline void putimage_alpha(int, int, IMAGE*)`，内部调用 Win32 `AlphaBlend`（`:21`，由 `:12` 的 `#pragma comment(lib,"MSIMG32.LIB")` 引入），实现带透明通道的混合绘制，游戏内 3 处调用（`:70`、`:153`、`:271`）。不透明背景与按钮另用 EasyX 原生 `putimage`（`:335`、`:338`、`:341`、`:521`、`:530`）。属适配器雏形，但未形成独立适配器类 |
- **外观（Facade）◐** — `main()` 承担了资源加载、游戏循环、碰撞、清理四类职责，形态上是一层外观，但没有隐藏的复杂子系统。
- **状态（State）◐** — `Button` 内部用 `Status::{Idle, Hovered, Pushed}` 枚举 + `if` 维护，无状态对象、无多态转移，不构成 State 模式。
- 其余模式未使用。**无单例**：资源以全局裸指针形式存在。

### 2.3 自定义类

| 类 | 位置 | 说明 |
|---|---|---|
| `Atlas` | `main.cpp:25` | 帧序列容器，`std::vector<IMAGE*> frame_list`（`:47`） |
| `Animation` | `main.cpp:55` | 按时间间隔轮播 `Atlas` 中的帧 |
| `Player` | `main.cpp:79` | 玩家，四向移动与射击 |
| `Bullet` | `main.cpp:184` | 子弹 |
| `Enemy` | `main.cpp:200` | 敌人，行为分支在 `on_update` |
| `Button` | `main.cpp:301` | 按钮基类，模板方法宿主 |
| `StartGameButton` | `main.cpp:367` | `: public Button`，仅重写 `OnClick` |
| `QuitGameButton` | `main.cpp:380` | `: public Button`，仅重写 `OnClick` |

继承体系只有一条链：`Button` ← `StartGameButton` / `QuitGameButton`。其余类之间是组合关系。

### 2.4 C++ 特性与资源导入

- 资源加载：`loadimage` 读 PNG，帧序列通过 `_T("img/player_right_%d.png")` 形式模板 + 帧数循环加载（`main.cpp:429`）。
- 绘制：`BeginBatchDraw` / `FlushBatchDraw` 批绘制，配合 `putimage_alpha` 或 `AlphaBlend`。
- 音频：MCI，通过 `#pragma comment(lib,"Winmm.lib")`（`main.cpp:13`）与 `#pragma comment(lib,"MSIMG32.LIB")`（`main.cpp:12`）引入系统库。
- 资源总量：**34 个 PNG + 1 个 MP3**。

### 2.5 构建与运行

1. 安装 EasyX。
2. 打开 `1.提瓦特幸存者.sln`。
3. **需手工配置**：项目属性 → C/C++ → 常规 → 附加包含目录，填入 EasyX 的 `include`；链接器 → 常规 → 附加依赖项，填 `EasyXa.lib`；链接器 → 常规 → 附加库目录，填 EasyX 的 `lib`（工程文件中未预置这些配置）。
4. 配置选 `Debug|x64`，生成后运行。工作目录需为 `提瓦特幸存者\`（资源路径 `img/` 是相对路径）。

### 2.6 已知问题

| 位置 | 问题 |
|---|---|
| `main.cpp:549` 与 `:551` | `delete atlas_player_right;` 出现两次（548–551 依次为 `atlas_player_left`、`atlas_player_right`、`atlas_enemy_left`、`atlas_player_right`）→ **重复删除** |
| `main.cpp:548-551` | `atlas_enemy_right` 从未被删除 → **内存泄漏**（`main.cpp:53` 定义，`:429` 附近分配） |
| `main.cpp:236` | `~Enemy()` 中 `delete anim_left, anim_right;` —— 逗号运算符优先级低于 `delete`，等价于 `(delete anim_left), anim_right`，**`anim_right` 泄漏** |
| `main.cpp:40-44` | `~Atlas()` 对 `IMAGE*` 调用 `delete`。EasyX 提供的释放接口是 `deleteimage()`，绕过它直接 `delete` 依赖 EasyX 内部以 `new` 分配，实现细节一变即失效 |
| `main.cpp:41` | `for (int i = 0; i < frame_list.size(); i++)` —— `int` 与 `size_t` 比较，有符号/无符号混用 |
| `main.cpp:87` | 帧数 `45` 硬编码在 `Player` 构造函数里，与 `Atlas` 实际帧数无关联校验 |

---

## 3. 2.植物明星大乱斗

### 3.1 玩法与程序流程

《植物大战僵尸》式双人同屏对战。两名玩家各选一名角色（豌豆射手 / 向日葵），在一行格子上互相射击、争夺阳光资源。

多文件结构（22 个 `.h` + 1 个 `.cpp`）。程序流程：

1. `SceneManager` 驱动当前场景，场景切换通过 `switch_to(SceneType)`（`scene_manager.h:23-37`）。
2. 三个场景：`MenuScene`（选角色）、`SelectorScene`（选关/说明）、`GameScene`（对局）。
3. `GameScene` 持有 `std::vector` 形式的两名玩家、子弹、平台、粒子、状态栏，逐帧 `on_update` → 碰撞 → `on_draw`。
4. 定时逻辑全部由 `Timer` 成员驱动，每个实体自带若干 `Timer`。

### 3.2 架构与设计模式

- **策略（Strategy）✅** — 碰撞形状被提升为独立的 `struct CollisionShape`（`platform.h:9`），挂在 `Platform`（`platform.h:7`）上。角色与平台的相交判定不写死在角色里，而是由 `Platform` 携带的形状描述决定，不同形状即不同策略。这是本项目中唯一严格成立的模式。
- **模板方法（Template Method）◐** — `Player`（`player.h:23`）构造函数内统一装配 4 个计时器并绑定回调（`player.h:28-42`），派生类 `PeashooterPlayer`（`peashooter_player.h:17`）、`SunflowerPlayer`（`sunflower_player.h:19`）只补充差异参数。骨架在基类构造函数中，但没有独立的模板方法接口，形态不完整。
- **享元（Flyweight）✅** — `Animation`、`Atlas`、`Timer`、场景实例等以 `extern` 全局声明共享（12 个头文件共 **87 条 `extern` 声明**，分布：`selector_scene.h` 30 条、`game_scene.h` 14 条、`sunflower_player.h` 11 条、`peashooter_player.h` 9 条、`player.h` 8 条，其余各 1–3 条）。共享成立，但通过全局变量而非查找表实现。
- **观察者（Observer）◐** / **命令（Command）◐** — `Timer` 的回调是单槽 `std::function`，属于回调注入，不构成观察者或命令（详见 §7）。
- **适配器（Adapter）◐** / **外观（Facade）◐** — `util.h` 提供 `putimage_ex` / `load_audio` / `play_audio` / `stop_audio` 一组 `inline` 自由函数，内部调用 `AlphaBlend`（`MSIMG32.LIB`，`util.h:3`）与 `mciSendString`（`main.cpp:13` 处链接 `Winmm.lib`）。是对 Win32 接口的一层封装，属适配器/外观的雏形。
- **状态（State）✗** — 无状态类。`SceneManager` 的阶段切换是 `switch` 到三个 `extern Scene*` 全局指针（`scene_manager.h:25-35`），`SceneType` 枚举（`scene_manager.h:10-12`）只是选择子，不承载行为。
- **单例（Singleton）✗** — 职责由全局变量承担。

### 3.3 自定义类

| 类 | 位置 | 说明 |
|---|---|---|
| `Scene` | `scene.h:5` | 场景基类，`on_enter`/`on_exit`/`on_update`/`on_draw`/`on_input` |
| `MenuScene` | `menu_scene.h:14` | `: public Scene` |
| `SelectorScene` | `selector_scene.h:46` | `: public Scene` |
| `GameScene` | `game_scene.h:29` | `: public Scene`，对局逻辑 |
| `SceneManager` | `scene_manager.h:8` | 场景切换，`switch_to` 内 `switch` 映射到全局指针 |
| `Player` | `player.h:23` | 角色基类，构造函数内装配 4 个 `Timer` |
| `PeashooterPlayer` | `peashooter_player.h:17` | `: public Player` |
| `SunflowerPlayer` | `sunflower_player.h:19` | `: public Player` |
| `Bullet` | `bullet.h:10` | 子弹基类 |
| `PeaBullet` | `pea_bullet.h:8` | `: public Bullet` |
| `SunBullet` | `sun_bullet.h:10` | `: public Bullet` |
| `SunBulletEx` | `sun_bullet_ex.h:10` | `: public Bullet` |
| `Platform` | `platform.h:7` | 平台，携带 `CollisionShape` |
| `CollisionShape` | `platform.h:9` | 碰撞形状描述（策略载体） |
| `Particle` | `particle.h:7` | 粒子 |
| `StatusBar` | `status_bar.h:4` | 血条/资源条 |
| `Timer` | `timer.h:5` | 单槽回调计时器 |
| `Animation` / `Atlas` / `Camera` / `Vector2` | `animation.h:8` / `atlas.h:5` / `camera.h:5` / `vector2.h:4` | 基础设施 |

继承体系有 4 条链：`Scene` ← 3 个场景，`Player` ← 2 个角色，`Bullet` ← 3 种子弹。

### 3.4 C++ 特性与资源导入

- `Timer` 基于 `std::function` 回调，支持 `set_wait_time` / `set_one_shot` / `set_callback` / `restart`。
- 角色无敌闪烁通过定时器翻转布尔量（`player.h:40-42`），每帧据该布尔量决定绘制原图还是素描帧（`player.h:128-129`）。
- 资源：`loadimage` 加载 PNG，`mciSendString` 控制 MP3。
- 资源总量：**112 个 PNG + 11 个 MP3 + 1 个 TTF**。

### 3.5 构建与运行

1. 安装 EasyX 并按 §2.5 补齐 include 目录与 `EasyXa.lib`。
2. 打开 `2.植物明星大乱斗.sln`，配置 `Debug|x64`。
3. 工作目录设为 `植物明星大乱斗\`，否则图片/音频相对路径失效。
4. 双人同屏，需同一键盘上两组按键。

### 3.6 已知问题

| 位置 | 问题 |
|---|---|
| `player.h:28` + `peashooter_player.h:54` | 基类构造函数 `player.h:28` 执行 `timer_attack_cd.set_wait_time(attack_cd)`，此时 `attack_cd` 还是 `player.h:417` 的默认值 `500`；派生类构造函数体内 `peashooter_player.h:54` 才赋 `attack_cd = 100`，**为时已晚**。结果是豌豆射手与向日葵的攻击冷却实际都是 500ms，配置值不生效 |
| `player.h:40-42` | `timer_invulnerable_blink` 只设了 `set_wait_time(75)` 与 `set_callback`，**漏掉 `set_one_shot(true)`**（对比同为闪烁用途的 `player.h:34-37` 就设了）。计时器持续周期性触发 `is_showing_sketch_frame = !is_showing_sketch_frame` |
| `player.h:128-129` | 配合上一条，`sketch_image(current_animation->get_frame(), &img_sketch)` 在首次受击后每帧执行，内部 `Resize` 反复重建位图 → 每秒约 13 次位图分配 |
| `selector_scene.h:236` | `mciSendString(_T("play ui_confirm from θ"), ...)` —— `from` 后是希腊字母 `θ`（U+03B8）而非数字 `0`，MCI 无法解析起始位置，**确认音效不播放** |
| `scene_manager.h:18-21` | `set_current_scene` 只调 `on_enter()` 而不先调 `on_exit()`，与 `switch_to`（`:24` 先 `on_exit`）行为不一致 |
| `scene_manager.h:24` | `switch_to` 直接 `current_scene->on_exit()`，未判空。首次进入若未经 `set_current_scene` 初始化即调用，会解引用空指针 |
| `player.h`、`peashooter_player.h`、`sunflower_player.h` | 三个基类均无虚析构函数，但实例经 `Scene`/`GameScene` 内的基类指针销毁 → 未定义行为 |
| 全局 | **87 条 `extern` 声明**分布在 12 个头文件，资源与场景共享完全依赖全局变量，无封装与初始化顺序保障 |

---

## 4. 3.空洞武士

### 4.1 玩法与程序流程

横版动作平台跳跃游戏。玩家与敌人各有 12 / 7 种状态，通过连招、翻滚、下蹲、二段跳等机制对抗，含子弹时间（Bullet Time）机制。

程序流程：`StateMachine::on_update` 驱动当前状态（`state_machine.cpp:7-16`），`CharacterManager` 遍历角色更新，角色内部的攻击/受击区域交由 `CollisionManager` 统一检测。

### 4.2 架构与设计模式

本项目是全仓库模式实现最完整的一个。

- **状态（State）✅** — 19 个状态类，共享 `StateNode`（`state_node.h:4`）基类，该基类声明带空实现的虚函数 `on_enter` / `on_update` / `on_exit`（`state_node.h:9-11`）。
  - 12 个敌人状态（`enemy_state_nodes.h`）：`EnemyAimState`(:6)、`EnemyDashInAirState`(:18)、`EnemyDashOnFloorState`(:31)、`EnemyDeadState`(:44)、`EnemyFallState`(:52)、`EnemyIdleState`(:61)、`EnemyJumpState`(:74)、`EnemyRunState`(:86)、`EnemySquatState`(:100)、`EnemyThrowBarbState`(:112)、`EnemyThrowSilkState`(:124)、`EnemyThrowSwordState`(:136)。
  - 7 个玩家状态（`player_state_nodes.h`）：`PlayerAttackState`(:5)、`PlayerDeadState`(:21)、`PlayerFallState`(:33)、`PlayerIdleState`(:42)、`PlayerJumpState`(:51)、`PlayerRollState`(:60)、`PlayerRunState`(:73)。
  - 转移表是扁平字符串索引：`register_state(id, node)`（`state_machine.cpp:28-30`）、`switch_to(id)`（`:22-26`）、`set_entry(id)`（`:18-20`）。
- **模板方法（Template Method）✅** — `StateNode` 的三段式生命周期骨架由 `StateMachine::on_update` 固定驱动（`state_machine.cpp:7-16`）：首次更新时 `need_init` 触发 `on_enter()`（`:10-13`），之后每帧 `on_update(delta)`。状态子类只填具体步骤。
- **工厂方法（Factory Method）✅** — `CollisionBox` 的构造函数与析构函数均为 private（`collision_box.h:48-49`），只通过 `friend class CollisionManager`（`collision_box.h:8`）授予构造权限，外部无法 `new`。唯一出口是 `CollisionManager::create_collision_box()`（`collision_manager.cpp:15-19`），它同时把新盒子登记进 `collision_box_list`；销毁对应 `destroy_collision_box()`（`:21-25`）。这是全仓库唯一名副其实的工厂方法，且确有生产调用点：`Character` 构造函数经工厂创建自己的两个碰撞盒（`character.cpp:6-7`），析构时经 `destroy_collision_box` 回收（`character.cpp:23`）。
- **单例（Singleton）✅ ×4** — 四个管理器，采用完全一致的写法（静态裸指针 + 懒加载）：
  - `ResourcesManager`：`resources_manager.h:10` 声明 `instance()`，`:18` 定义 `static ResourcesManager* manager`。
  - `CollisionManager`：`collision_manager.h:7` / `:16`；`collision_manager.cpp:5-10` 为 `if (!manager) manager = new CollisionManager();`。
  - `CharacterManager`：`character_manager.h:6` / `:21`。
  - `BulletTimeManager`：`bullet_time_manager.h:11` / `:19`。
- **适配器（Adapter）✅** — `util.h:7` 的 `struct Rect` 加一组 `inline` 自由函数，把 `AlphaBlend`（`MSIMG32.lib`，`util.h:5`）与 `mciSendString`（`WINMM.lib`，`util.h:4`）适配为统一的绘制/播放接口；`util.h` 是本项目唯一的适配层文件。
- **外观（Facade）✅** — `util.h` 向游戏逻辑层只暴露"贴图 + 矩形 + 相机"，隐藏 GDI/MCI 细节；`ResourcesManager` 把资源描述表、图集装载、水平翻转三个内部环节（`load()` / `find_atlas` / `find_image`，`resources_manager.h:12-15`）统一成对外的加载与查询入口。
- **享元（Flyweight）✅** — `resources_manager.cpp:17` 的 `struct ImageResInfo` 与 `:22` 的 `struct AtlasResInfo` 构成数据驱动的资源描述表，`ResourcesManager` 集中持有并按名查表，动画与角色共享同一份图集。
- **原型（Prototype）◐** — 图集被当作"命名原型"共享，多个实体引用同一 `Atlas`，但未实现克隆接口。
- **组合（Composite）◐** — `struct AnimationGroup`（`character.h:77`）把多个动画聚合成组，具备树形语义的雏形，但未统一 `Character` 与 `AnimationGroup` 的接口。
- **策略（Strategy）◐** — 攻击方式由状态类区分（`ThrowBarb` / `ThrowSilk` / `ThrowSword` / `Roll`），策略身份由状态 id 隐式携带，非独立策略对象。
- **解释器（Interpreter）◐** — 敌人行为分支与概率表（`enemy.cpp` 中按权重随机选择行为）具备"表驱动解释"特征，但没有文法对象。

### 4.3 自定义类

**角色层**

| 类 | 位置 | 说明 |
|---|---|---|
| `Character` | `character.h:11` | 角色基类，虚函数 `on_input`/`on_update`/`on_render`/`on_hurt`（`:67-71`） |
| `Player` | `player.h:4` | `: public Character` |
| `Enemy` | `enemy.h:6` | `: public Character` |
| `AnimationGroup` | `character.h:77` | `struct`，动画组合 |

**状态层**：`StateNode`（`state_node.h:4`）、`StateMachine`（`state_manachine.h:7`）+ 19 个状态类（见 §4.2）

**管理单例**：`ResourcesManager`（`resources_manager.h:8`）、`CollisionManager`（`collision_manager.h:5`）、`CharacterManager`（`character_manager.h:4`）、`BulletTimeManager`（`bullet_time_manager.h:4`）

**碰撞层**：`CollisionBox`（`collision_box.h:7`）持有 `position`、`size`、`enabled`、`layer_src`、`layer_dst` 与单槽回调 `on_collide`（`collision_box.h:23`）。层级过滤采用**非对称交叉判定**：`process_collide()` 中仅当 `src->layer_dst == dst->layer_src` 时才算命中（`collision_manager.cpp:33`），即每个盒子同时声明"我是什么"（`layer_src`）与"我能撞到什么"（`layer_dst`）。AABB 判定在 `:36-45`，回调在 **dst** 上触发（`:47-48`）。`on_debug_render()`（`:54-62`）提供碰撞盒可视化调试。

**其他**：`Barb`（`barb.h:5`）、`Sword`（`sword.h:5`）、`Animation` + `struct Frame`（`animation.h:7` / `:98`）、`Atlas`（`atlas.h:5`）、`Timer`（`timer.h:5`）、`Vector2`（`vector2.h:5`）、`Rect`（`util.h:7`）

**所有权反转**：每个 `Character` 持有两个指向自身碰撞盒的指针——`hit_box`（攻击判定，`character.h:98`）与 `hurt_box`（受击判定，`character.h:99`），经 `get_hit_box()` / `get_hurt_box()`（`character.h:46-51`）暴露。碰撞盒反过来持有命中回调。因此角色不需要自己遍历子弹，碰撞完全由 `CollisionManager` 集中分发。这是全仓库唯一采用该手法的地方。

### 4.4 C++ 特性与资源导入

- 多态：4 个管理器 + 角色 + 状态 + 碰撞盒共 4 层虚函数体系。
- 回调：`CollisionBox::set_on_collide(std::function<void()>)`、`Timer` 回调，用于把"碰撞结果"与"角色行为"解耦。
- 所有权反转：`Character` 持有指向碰撞盒的指针，碰撞盒反过来持有点击回调——角色不遍历子弹，碰撞由管理器集中分发。
- 资源：`loadimage` 加载 PNG，`mciSendString` 播放 MP3；资源描述表以结构体数组声明。
- 资源总量：**136 个 PNG + 20 个 MP3**（6 个项目中最多）。

### 4.5 构建与运行

1. 安装 EasyX 并补齐 include 与 `EasyXa.lib`。
2. 打开 `3.空洞武士.sln`，配置 `Debug|x64`。
3. 工作目录设为 `空洞武士\`。

### 4.6 已知问题

| 位置 | 问题 |
|---|---|
| `resources_manager.cpp:108` | `flip_atlas("enemy_run_left", "enemy_runt_right");` —— 第二个字符串 `enemy_runt_right` 拼写错误（`runt`），是对 `:110` `flip_atlas("enemy_run_left", "enemy_run_right")` 的一处笔误重复调用。**不影响渲染**：`:110` 已正确生成右向图集，`enemy.cpp:135` 请求的 `enemy_run_right` 能正常取到。实际后果是白白多生成一份 8 帧、永无人请求的 `enemy_runt_right` 图集（`flip_atlas` 在 `:193` 无条件写入 `atlas_pool[dst_id]`），属无效内存占用 |
| `resources_manager.cpp:177` 与 `:184` | 内部辅助函数 `flip_image` / `flip_atlas` 用 `image_pool[src_id]`、`atlas_pool[src_id]` 查**源** id（写入侧 `dst_id` 用 `[]` 无害）。若源 id 拼错，会先插入 `nullptr` 再在 `:180-190` / `:186-193` 解引用 → 崩溃。当前源 id 均正确，属潜伏缺陷。与对外的 `find_atlas` / `find_image`（`:142-155`，用 `find()` 并返回 `nullptr`）形成鲜明对比：**同一文件内两种查找风格** |
| `enemy.cpp:316` | `play_audio(_T("ememy_hurt_2"), false);` —— `ememy` 拼写错误（应为 `enemy`），受击音效不播放 |
| `state_node.h:9-11` | `StateNode` 的 `on_enter`/`on_update`/`on_exit` 是虚函数，但**没有虚析构函数** |
| `state_manachine.h:10` + `state_machine.cpp:5` | `~StateMachine() = default;` 不释放已注册的 19 个 `StateNode*` → 状态节点全部泄漏；受上一条影响也无法通过基类指针正确析构 |
| `state_machine.cpp:24` | `switch_to` 中 `current_state = state_pool[id];` 用 `std::map::operator[]` 访问。id 拼错时会**静默插入一个 `nullptr` 条目**并把当前状态置空（`:25` 与 `:8` 有判空守卫，不会崩溃，但故障被静默吞掉，且表中持续堆积空条目）。`find`/`at` 才是此处应有的语义 |
| `state_manachine.h` vs `state_machine.cpp` | 头文件名拼作 `state_manachine.h`（多一个 `n`），实现文件是 `state_machine.cpp`，两者命名不一致 |
| `bullet_time_manager.h` vs `bullen_time_manager.cpp` | 同类问题：头文件是 `bullet_time_manager.h`，实现文件拼作 `bullen_time_manager.cpp`（`bullen`）。3 号项目存在两处头/源命名不一致 |
| `collision_manager.cpp:27-51` | `process_collide()` 为 O(n²) 双层循环遍历全部碰撞盒对，每帧无条件全量检测 |
| `collision_manager.cpp:47-48` | `on_collide` 是单槽 `std::function`，同一盒子只能对一种碰撞作出反应 |
| `Character` 及 4 个管理器 | 均无虚析构函数；4 个单例的析构函数为 private 且静态指针从不重置，进程内无法回收 |

---

## 5. 4.哈基米大冒险

### 5.1 玩法与程序流程

**全仓库唯一的联机项目**，也是唯一零继承、零虚函数的项目。

玩法是打字竞速：两名玩家在同一台机器上抢着把 `text.txt` 的内容逐字符敲对，敲对字符即推进自身进度，双方进度经 HTTP 同步到服务端，折算成沿固定折线路径奔跑的角色。

**客户端**（`client/client.cpp`，371 行）流程：

1. `initgraph(1280, 720)` 开窗（`:166`），装载资源（`:172` → `load_resources`，`:65-116`）。
2. 向服务端登录：POST `/login` 取回玩家序号（`:122`），再 POST `/query_text` 取回全文并按行切分、统计总字符数 `num_total_char`（`:137-144`）。
3. 启动一条分离（`detach`）的后台线程，以 **10Hz** 轮询自身进度并拉取对方进度（`:146-160`，睡眠 `1000000000/10`，见 `:158`）。
4. 主循环目标 **144 FPS**（`:206` 设定帧时长，`:365-367` 补睡眠）。
5. 阶段由 `enum class Stage { Waiting, Ready, Racing }`（`:12-16`）驱动：等双方就位 → 1 秒倒数 → 比赛。输入在 `stage != Racing` 时被丢弃（`:213-215`）。
6. 渲染用 `BeginBatchDraw` / `FlushBatchDraw` 批绘制（`:209` / `:362`），背景与角色走 `camera_scene`，UI 走 `camera_ui`；两名角色按 y 坐标排序决定遮挡（`:291-298`）。

**服务端**（`serve/server.cpp`，63 行）：启动时整份读入 `text.txt`（`:14-25`），暴露 4 条路由，`server.listen("0.0.0.0", 25565)`（`:60`）。

| 路由 | 位置 | 行为 |
|---|---|---|
| `POST /login` | `:29-38` | 两人都已就位则返回 `-1`；否则按空位分配序号 `1`/`2` 并返回 |
| `POST /query_text` | `:40-42` | 返回 `text.txt` 全文 |
| `POST /update_1` | `:44-50` | 写入玩家 1 进度，返回玩家 2 进度 |
| `POST /update_2` | `:52-58` | 写入玩家 2 进度，返回玩家 1 进度 |

### 5.2 架构与设计模式

**本项目没有任何严格的 GoF 实现。**

- **享元（Flyweight）✅（就"共享"这一事实而言）** — 16 个全局 `Atlas`（`:38-53`）、6 个全局 `IMAGE`（`:55-60`）被两名 `Player` 共享；`Path`（`:26-31`）作为全局不可变几何数据共享。无查找表、无池管理。
- **适配器（Adapter）◐** — `util.h` 把 `AlphaBlend`（`MSIMG32.lib`）与 `mciSendString`（`WINMM.lib`，`util.h:5`）封装为 `putimage_ex` / `load_audio` / `play_audio` / `stop_audio`（`util.h:13-38`）。`putimage_ex` 额外做了相机偏移换算并缓存 `BLENDFUNCTION` 静态量（`util.h:14`），比 1/2/3 号更接近真正的适配器。
- **状态（State）◐** — `Stage` 枚举（`client.cpp:12-16`）+ `if` 分支（`:242-245`、`:247-248`、`:343`）。阶段对象为全局变量 `stage`（`:19`），行为不封装在状态类中。
- **策略（Strategy）◐** — 角色沿 `Path` 前进的算法封装在 `Path::get_position_at_progress`（`path.h:19-35`）内，可更换不同路径策略，但未抽象为策略接口。
- **命令（Command）◐** — HTTP 路由的 4 个 lambda（`server.cpp:29/40/44/52`）是无状态的请求处理函数，不具备命令对象的封装与可回放性。
- **模板方法（Template Method）✗** — 无继承体系。
- **单例（Singleton）✗** — 全局对象与全局指针。

### 5.3 自定义类

**客户端（8 个头文件，零继承）**

| 类 | 位置 | 说明 |
|---|---|---|
| `Path` | `path.h:5` | 折线路径。构造函数累加各段长度得 `total_length`（`:10-14`）；`get_position_at_progress`（`:19-35`）按**弧长**把 0~1 的进度重参数化为路径上的坐标，两端做钳制（`:20-21`）。**不是 A\* 寻路**，是固定折线上的等弧长插值 |
| `Player` | `player.h:5` | 构造函数接收 8 个 `Atlas*`（两角色 × 四向 × 闲置/奔跑），装配出 8 个 `Animation`（`:12-45`）；`on_update`（`:48-80`）朝目标点移动 |
| `Animation` / `struct Frame` | `animation.h:7` / `:88` | 帧序列播放 |
| `Atlas` | `atlas.h:5` | 图集，`load` 用 `_stprintf_s` 按模板批量加载（`:10-19`） |
| `Camera` | `camera.h:4` | `look_at` 使目标居中（`:25-27`） |
| `Timer` | `timer.h:5` | 单槽回调计时器，**本项目唯一使用 `std::function` 的地方**（`:23-25`） |
| `struct Rect` | `util.h:8` | 绘制矩形 |
| `Vector2` | `vector2.h:4` | 二维向量，支持 `normalize` / `length` / `approx` |

**服务端**：无自定义类，只有 `httplib::Server` + 4 个 lambda + `std::mutex g_mutex`（`server.cpp:6`）。该互斥锁是必需的——cpp-httplib 默认用线程池处理请求，4 条路由会并发读写 `progress_1` / `progress_2`（`:30`、`:45`、`:53` 三处加锁）。

### 5.4 C++ 特性与资源导入

- **网络**：cpp-httplib 单头文件 `thirdparty/httplib.h`，两端各 `#include "../thirdparty/httplib.h"`（`server.cpp:1`、`client.cpp:1`）。客户端设 `set_keep_alive(true)`（`client.cpp:120`）复用连接。
- **并发**：客户端用 `std::atomic<int> progress_1/progress_2`（`:22-23`）避免后台线程与主循环的数据竞争；`std::thread` + `detach` 跑轮询线程（`:146-160`）。服务端用 `std::mutex` 保护共享进度。
- **配置与数据文件**：`config.cfg` 内容即服务器地址，被客户端原样读入（`:104-115`）；`text.txt` 被服务端原样读入（`:14-25`），客户端按行切分。
- **编码转换**：`std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>`（`:350`）把 UTF-8 文本转 `wstring` 供 `outtextxy` 输出；已完成部分用偏移 `+2` 画灰色底影实现描边效果（`:355-359`）。
- **字体**：`AddFontResourceEx(_T("resources/IPix.ttf"), FR_PRIVATE, NULL)`（`:66`）加载像素字体，`settextstyle(28, 0, _T("IPix"))`（`:168`）。
- **资源**：`loadimage` 读 PNG，`mciSendString` 播 MP3；16 个图集各 4 帧（`:68-83`）。
- 资源总量：**70 个 PNG + 11 个 MP3 + 1 个 TTF**。

### 5.5 构建与运行

服务端与客户端是两个独立工程，需分别构建、同时运行。

1. 安装 EasyX 并补齐 include 与 `EasyXa.lib`（两个工程都要）。
2. 构建 `serve/serve.vcxproj` → 产物 **`server.exe`**（工程名是 `server`，目录名是 `serve`，注意区分）。`server.cpp:17` 用 `MessageBox` 报错，因此虽是控制台子系统也会弹窗。
3. 构建 `client/client.vcxproj` → `client.exe`。
4. 客户端工作目录必须为 `client\`，且 `client\config.cfg` 内写入服务器地址（如 `http://127.0.0.1:25565`）。
5. 服务端工作目录必须为 `serve\`，且 `serve\text.txt` 存在，否则启动即 `MessageBox` 报错并 `return -1`（`server.cpp:16-19`）。
6. 启动 `server.exe`，再启动**两个** `client.exe`。第二个实例会因 `/login` 返回 `-1` 被拒（`client.cpp:130-133`）。
7. 服务端不提供重置逻辑，两人到齐后 `/login` 恒返回 `-1`（`server.cpp:32-35`），**每局之间必须重启 `server.exe`**。

> 注意：`4.哈基米大冒险\哈基米大冒险\` 是一个**只含 `.vcxproj` 与 `.filters`、不含任何源文件**的空壳工程，会一并出现在解决方案中。构建整个解决方案时它不产出任何东西，可从解决方案中移除。

### 5.6 已知问题

| 位置 | 问题 |
|---|---|
| `client.cpp:149` | `std::string route = (id_player == 1) ? "/update_1" : "update_2";` —— 三元表达式的 else 分支**漏了前导斜杠**，成为相对路径 `update_2`。玩家 2 的进度**永远上报失败**；同时后台线程会把 `result` 判为失败，对手进度不更新。该玩家自身进度仍在本地递增（`:227`）并在 `:250-251` 判定胜利，但画面上不会移动 |
| `player.h:70-75` | 移动分支的 `switch (facing)` 四个 case 分支全部指向 `anim_idle_*`（`:71-74`），与静止分支 `:59-62` 完全相同，**奔跑动画 `anim_run_*` 从未被选中**。角色始终播放闲置动画 |
| `client.cpp:104-115` | `config.cfg` 用 `str_stream << file.rdbuf()` 原样读入，**不做去空白**。文件若以换行结尾（Windows 编辑器默认行为），地址尾部会带上 `\r\n` |
| `atlas.h:30` | `if (idx<0 || idx>img_list.size()) return nullptr;` —— 边界条件写成 `>` 而非 `>=`，`idx == img_list.size()` 时越界访问 |
| `atlas.h:21` | 方法名拼写错误 `claer()`（应为 `clear`），且全项目无调用点 |
| `atlas.h:16` | 帧索引从 `i + 1` 起（`:16`），而 `resources` 中的文件从 `_1.png` 起编号——该约定全靠约定，无断言校验 |
| `client.cpp:219` | `str_line[idx_char]` 在 `idx_char` 超出当前行长度时越界。空行会导致 `std::string::operator[]` 读到 `'\0'`，该字符无法与任何 `WM_CHAR` 匹配 → **空行导致进度无法推进（软锁）** |
| `client.cpp:143` | `num_total_char += (int)str_line.length();` —— `size_t` 转 `int` 后累加到 `int`，无溢出校验 |
| `client.cpp:213-215` | `if (stage != Stage::Racing) continue;` 丢弃输入的同时也丢弃了消息循环的推进语义，逻辑可读性差（功能上无 bug） |
| `server.cpp:37` | `(progress_1 >= 0) ? (progress_2 = 0) : (progress_1 = 0);` 用三元表达式做副作用，可读性差；配合 `g_mutex` 语义正确 |
| `client.cpp:1` / `server.cpp:1` | `#include "../thirdparty/httplib.h"` 用相对路径包含第三方库，换目录结构即失效 |

---

## 6. 5.生化危鸡

### 6.1 玩法与程序流程

SDL2 实现的横版射击。玩家固定在屏幕底部向上射击，敌人（鸡）从顶部不同高度、不同速度下落，漏过底部扣血。

单场景，全部逻辑在 `main.cpp`（无 `Scene` 类）。每帧流程：

1. 处理 `SDL_Event`（`main.cpp:211` 起处理 `SDL_QUIT` 等）。
2. 更新子弹列表、玩家输入。
3. 遍历 `chicken_list` 做命中判定与扣血（`:270-280`）。
4. 用 `std::remove_if` 清理失效对象（`:284-295`）。
5. 按 y 坐标 `std::sort` 排序以决定遮挡顺序（`:298-301`）。
6. 渲染，`TTF_RenderUTF8_Blended` 绘制分数（`:410-413`）。

### 6.2 架构与设计模式

- **简单工厂（Simple Factory）◐** — 生成器写在 `timer_generate` 的回调里（`main.cpp:109-121`），每 1.5 秒按百分比权重选型：`val = rand() % 100`，`< 50` → `new ChickenSlow()`（50%，`:113-114`），`< 80` → `new ChickenMedium()`（30%，`:115-116`），否则 `new ChickenFast()`（20%，`:117-118`）。工厂逻辑没有独立角色、直接内联在定时器回调中，属简单工厂的最简形态。这是本项目唯一像工厂的东西。
- **享元（Flyweight）◐** — `atlas_explosion` 与 `sound_explosion` 以 `extern` 全局共享（`chicken.h:7-8`），被所有 `Chicken` 实例复用。共享成立，但只有这一个资源走了全局，且无池无查找表。
- **模板方法（Template Method）◐** — `Chicken` 构造函数（`chicken.h:13-26`）固定装配"奔跑 + 爆炸"两段动画并绑定结束回调（`:20-22`），派生类只覆盖 `speed_run`（`protected`，`chicken.h:62`）。骨架在基类构造函数里，无独立模板方法接口。
- **观察者（Observer）◐** — `animation_explosion.set_on_finished([&]() { is_valid = 0; })`（`chicken.h:20-22`）是单槽回调，属回调注入而非观察者。
- **状态（State）◐** — `animation_current = (is_alive ? &animation_run : &animation_explosion);`（`chicken.h:35`）是指针三元选择，不是状态对象，也无转移逻辑。
- **适配器（Adapter）◐** — `Mix_PlayChannel`（`chicken.h:46`）、`IMG_LoadTexture` 等 SDL 调用散落在业务代码中，未形成适配层。
- **外观（Facade）◐** — `Camera`（`camera.h:7`）封装了视口偏移，可视为渲染外观的雏形。
- **单例（Singleton）✗** — 资源以 `extern` 全局变量共享。

### 6.3 自定义类

| 类 | 位置 | 说明 |
|---|---|---|
| `Chicken` | `chicken.h:11` | 敌人基类。构造函数随机初始位置（`:24-25`）；`on_update`（`:32-38`）按存活状态选动画；`on_hurt`（`:44-47`）切换为爆炸动画并播放音效；`can_remove`（`:57-59`）供清理 |
| `ChickenFast` | `chicken_fast.h:6` | `: public Chicken`，`~ChickenFast() = default;`（`:13`） |
| `ChickenMedium` | `chicken_medium.h:6` | `: public Chicken`，`~ChickenMedium() = default;`（`:12`） |
| `ChickenSlow` | `chicken_slow.h:6` | `: public Chicken`，`~ChickenSlow() = default;`（`:12`） |
| `Animation` / `struct Frame` | `animation.h:10` / `:101` | 支持 `set_on_finished` 回调 |
| `Atlas` | `atlas.h:8` | 图集，`~Atlas()`（`:11`）释放纹理 |
| `Bullet` | `bullet.h:9` | 子弹 |
| `Camera` | `camera.h:7` | 视口 |
| `Timer` | `timer.h:5` | 计时器 |
| `Vector2` | `vector2.h:4` | 向量 |

继承体系仅一条链：`Chicken` ← 3 个速度变体，差异仅由 `speed_run` 表达。

### 6.4 C++ 特性与资源导入

- SDL2 + SDL2_image（`IMG_LoadTexture`）+ SDL2_mixer（`Mix_LoadWAV` / `Mix_PlayChannel`）+ SDL2_ttf（分数文本）。
- 算法式清理：`std::remove_if` + `erase` 移除失效对象（`main.cpp:284-295`），`std::sort` 做 y 轴排序（`:298-301`）。
- lambda 用于谓词与动画回调。
- 资源总量：**32 个 PNG + 2 个 MP3 + 1 个 TTF**（6 个项目中资源最少）。

### 6.5 构建与运行

1. 打开 `5.生化危鸡.sln`。
2. **配置必须选 `Release|x64`**。SDL 的附加包含目录与附加依赖项只写在 `Release|x64` 这一处（`生化危鸡.vcxproj:120-126`），选 `Debug|x64` 会找不到 `SDL.h`。若需 Debug，需手工把 `:120`、`:125`、`:126` 三行补到 `Debug|x64` 对应的 `ItemDefinitionGroup`。
3. Win32 配置不可用：库路径写死 `lib\x64`。
4. 工作目录设为 `生化危鸡\`，SDL2 的 DLL 需与可执行文件同目录（`SDL2.dll` 及 image/mixer/ttf 三个扩展 DLL）。

### 6.6 已知问题

| 位置 | 问题 |
|---|---|
| `chicken.h:11` | `Chicken` **完全没有析构函数**，而 `chicken_fast.h:13`、`chicken_medium.h:12`、`chicken_slow.h:12` 都声明了 `~ChickenX() = default;` 且**均非虚函数**。`main.cpp:293` 通过 `Chicken*` 执行 `delete chicken` → **未定义行为**（若派生类将来增加成员资源，将直接泄漏或崩溃） |
| `main.cpp:317` 与 `:319-322` | **射击散布无效**。`:317` 用**未加散布**的 `angle_barrel` 构造子弹，而 `Bullet::Bullet(double angle)`（`bullet.h:11-17`）在构造时就由该角度算出 `velocity`（`:14-16`）并保存为渲染旋转角（`:37`）。`:319` 随后算出的 `angle_bullet = angle_barrel + (rand() % 30 - 15)`（±15°）只用于 `:322` 的 `set_position`，即**只影响子弹出膛的坐标**。结果：弹道永远严格沿炮管朝向（零散布、必定命中轴线），但出生点最大偏移 `length_barrel × sin(15°) ≈ 105 × 0.259 ≈ 27` 像素 |
| `main.cpp:290-295` | `std::remove_if` 的**谓词带有副作用**：`[](Chicken* chicken){ ... if (can_remove) delete chicken; return can_remove; }`。`remove_if` 要求谓词是纯函数（可能被多次求值、也可能作用于元素副本），删除逻辑写在这里违反约定。当前调用点唯一，尚未触发问题 |
| `atlas.h:11` + `main.cpp:153` | `Atlas` 是全局对象，其析构函数在 `main` 返回后、`static` 对象销毁阶段执行，**晚于 `main.cpp:153` 的 `SDL_Quit()`**。析构中释放纹理属于 use-after-free |
| `main.cpp:410-413` | 每帧为绘制分数新建 2 个 `SDL_Surface`（`TTF_RenderUTF8_Blended`）与 2 个 `SDL_Texture`（`SDL_CreateTextureFromSurface`），代码中未见对应的 `SDL_FreeSurface` / `SDL_DestroyTexture` → **每帧泄漏 2 个表面 + 2 个纹理**；且分数不变时结果完全相同，属无谓开销 |
| `chicken.h:20-22` | 回调用 `[&]` 捕获[this 语义外的所有引用]，实际只用到 `this`；应写 `[this]`。若 `Chicken` 在回调触发前被销毁则悬垂（当前由 `is_valid` 机制规避） |
| `main.cpp:276-280` | 敌人越过 `y >= 720` 即 `make_invalid()` 并扣血，无难度递增或波次机制 |

---

## 7. 6.拼好饭传奇

### 7.1 玩法与程序流程

SDL2 + C++17 实现的外卖骑手经营游戏。骑手在场景中移动，从取餐台取餐、送到微波炉加热、再送到顾客桌。

架构核心是 `Region` 体系：场景中所有可交互对象都是 `Region` 子类，注册进单例 `RegionMgr`，由 `RegionMgr` 统一驱动更新、渲染与输入分发。

程序流程（`main.cpp`）：

1. 启动时向 `RegionMgr` 注册 **15 个 `Region`**（`main.cpp:26-40`）：3 个 `DeliveryDriver`、6 个 Bundle（`ColaBundle`/`SpriteBundle`/`TbBundle`/`MbBoxBundle`/`BcBoxBundle`/`RcpBoxBundle`）、2 个 `MicrowaveOven`、4 个 `TakeoutBox`。
2. `ResMgr::instance()->load(renderer)`（`:73`）一次性载入全部资源，随后播放 BGM（`:77`）。
3. 主循环中 `RegionMgr::on_update`（`:44`）、输入同时分发给 `CursorMgr` 与 `RegionMgr`（`:93-94`）、渲染先 `RegionMgr` 再叠加 `CursorMgr`（`:51-52`）。

### 7.2 架构与设计模式

- **中介者（Mediator）✅** — `CursorMgr`（`cursor_mgr.h:5`）持有全局唯一的交互上下文：`meal_picked`（当前抓取中的餐品）、`pos_cursor`（光标位置）、`is_mouse_lbtn_down`（左键按下状态，`cursor_mgr.h:22-24`）。所有 `Region` 子类之间**不直接通信**，取餐、加热、放置全部通过读写 `CursorMgr` 的这三个字段完成。`CursorMgr` 是全仓库唯一名副其实的中介者，也是 6 号解耦最充分的部分。
- **单例（Singleton）✅ ×3** — 三者写法完全一致（静态裸指针 + 懒加载 `if (!manager) manager = new X();`，私有构造/析构）：
  - `ResMgr`：`res_mgr.h:10` / `:21`；`res_mgr.cpp:8-11`。
  - `RegionMgr`：`region_mgr.h:9` / `:23`；`region_mgr.cpp:5-9`。
  - `CursorMgr`：`cursor_mgr.h:7` / `:20`。
- **模板方法（Template Method）◐** — `Region`（`region.h:4`）声明 4 个带空实现的虚函数钩子 `on_update` / `on_render` / `on_cursor_down` / `on_cursor_up`（`region.h:11-14`），固定的遍历骨架写在 `RegionMgr::on_update`（`region_mgr.cpp:21-24`）、`on_render`（`:26-29`）、`on_input`（`:31-52`）中。骨架不在基类内，故只算相似形态。
- **外观（Facade）✅** — `ResMgr`（`res_mgr.h:8`）把 `IMG_LoadTexture`、`Mix_LoadWAV`、目录遍历、纹理/音频存储全部收进 `load` / `find_texture` / `find_audio` 三个接口，游戏逻辑只按名字取资源。
- **享元（Flyweight）✅** — `audio_pool` 与 `texture_pool` 两个 `unordered_map<string, ...>`（`res_mgr.h:22-23`）集中缓存全部纹理与音频，按字符串名共享，40 张 PNG 与 8 段 MP3 各只加载一次。
- **适配器（Adapter）◐** — `CursorMgr::on_input`（`cursor_mgr.cpp`）把 SDL 的 `SDL_Event` 翻译成游戏语义（餐品枚举、音效播放），属于事件层适配。
- **迭代器（Iterator）◐** — `RegionMgr` 用 `std::unordered_map` 的范围 for 遍历（`region_mgr.cpp:22`、`:27`、`:35`、`:44`），但类本身未提供 `begin`/`end`，未实现面向调用方的迭代器接口。
- **命令（Command）◐** — `on_cursor_down` / `on_cursor_up` 钩子把"点击"作为消息传给 `Region`，但不封装请求、无接收者分离、不可排队或回滚。
- **状态（State）◐** — 无状态类。餐品流转由 `Meal` 枚举 + 各 `Region` 内的 `switch` 表达。
- **单例之外无全局池化**；`Region` 对象由 `main.cpp` 直接 `new` 后交予 `RegionMgr`，管理器不负责销毁。

### 7.3 自定义类

| 类 | 位置 | 说明 |
|---|---|---|
| `Region` | `region.h:4` | 可交互区域基类，4 个虚钩子（`:11-14`）+ `SDL_Rect rect` |
| `DeliveryDriver` | `delivery_driver.h:6` | `: public Region`，骑手 |
| `MicrowaveOven` | `microwave_oven.h:7` | `: public Region`，微波炉，冷餐加热 |
| `TakeoutBox` | `takeout_box.h:5` | `: public Region`，顾客桌 |
| `Bundle` | `bundle.h:5` | `: public Region`，**抽象意图层，但无任何类继承它** |
| `ColaBundle` | `cola_bundle.h:4` | `: public Region`，饮料包 |
| `SpriteBundle` | `sprite_bundle.h:4` | `: public Region`，雪碧包 |
| `TbBundle` | `tb_bundle.h:4` | `: public Region` |
| `MbBoxBundle` | `mb_box_bundle.h:5` | `: public Region` |
| `BcBoxBundle` | `bc_box_bundle.h:4` | `: public Region` |
| `RcpBoxBundle` | `rcp_box_bundle.h:4` | `: public Region` |
| `RegionMgr` | `region_mgr.h:7` | 单例，按名注册与查找 `Region`，驱动三种生命周期 |
| `CursorMgr` | `cursor_mgr.h:5` | 单例，中介者，共享光标与抓取状态 |
| `ResMgr` | `res_mgr.h:8` | 单例，资源池 + `std::filesystem` 自动发现 |
| `Timer` | `timer.h:5` | 计时器 |

继承体系：`Region` ← 11 个类（3 个功能区 + 6 个 Bundle + `Bundle` 自身）。

### 7.4 C++ 特性与资源导入

- **C++17 `<filesystem>`** 是本项目使用新标准的直接原因：`res_mgr.cpp:4` 引入，`:27` 用 `std::filesystem::directory_iterator("resources")` 遍历目录，`:30` / `:34` 按扩展名分流，`:32` / `:36` 用 `path.stem().u8string()`（文件名去扩展名）作为键。**新增资源文件无需改动代码**。注意该遍历**非递归**，只扫 `resources` 顶层，子目录不会被收录。
- `SDL_Event` 处理集中在 `CursorMgr::on_input` 与 `RegionMgr::on_input`。
- `enum class Meal` 贯穿取餐、加热的全部逻辑（`meal.h`）。
- SDL2 + SDL2_image + SDL2_mixer（无 ttf 使用）。
- 资源总量：**40 个 PNG + 8 个 MP3，共 48 个**，由 `directory_iterator` 全量自动加载。

### 7.5 构建与运行

1. 打开 `6.拼好饭传奇.sln`。
2. 配置选 `Debug|x64` 或 `Release|x64` 均可（两处都配了 SDL 路径与 `stdcpp17`）。Win32 不可用（库路径为 `lib\x64`）。
3. 工作目录设为 `拼好饭传奇\`。
4. SDL2 的 4 个 DLL（`SDL2.dll`、`SDL2_image.dll`、`SDL2_mixer.dll`、`SDL2_ttf.dll`）需与可执行文件同目录。

### 7.6 已知问题

| 位置 | 问题 |
|---|---|
| `cursor_mgr.cpp:44` | `find_texture("bc_cold _picked")` —— `cold` 与 `_picked` 之间多一个空格，正确键名应为 `bc_cold_picked` |
| `res_mgr.cpp:14` 与 `:18` | `find_audio` / `find_texture` 用 `audio_pool[name]` / `texture_pool[name]` 查表，键不存在时**静默插入 `nullptr`** 并返回它。调用方无法区分"资源缺失"与"资源为空"，SDL 收到空纹理会静默不绘制。上一条的空格笔误正是被这一机制掩盖的——没有报错，只是图标不出现 |
| `region_mgr.cpp:18` | `Region* RegionMgr::find(...) { return region_pool[name]; }` 同样的 `operator[]` 问题：查不到的键会插入 `nullptr`，而这些空条目随后在 `on_update`（`:23`）与 `on_render`（`:28`）中被无条件解引用 → **崩溃**。应使用 `at` 或显式查找 |
| `region_mgr.cpp:35-39` | 命中测试循环遍历 `region_pool`，判定 `SDL_PointInRect` 后调用 `on_cursor_down()`，**没有 `break`**。任意两个矩形存在重叠时，一次点击会被派发给所有命中的 `Region`，结果取决于遍历顺序 |
| `region_mgr.cpp:44-48` | 同上，`on_cursor_up` 的循环同样缺 `break` |
| `region_mgr.cpp:22` 与 `:27` | `on_update` / `on_render` 遍历 `std::unordered_map`。`unordered_map` 的迭代顺序由实现定义，**因此渲染的 z 序不确定**：每帧可能不同，重叠对象的前后关系会跳变 |
| `cursor_mgr.cpp:17-18` | 在 `SDL_MOUSEBUTTONDOWN` 分支里读取 `event.motion.x` / `event.motion.y`。`motion` 与 `button` 是 `SDL_Event` union 的不同成员，此处能取到正确坐标**纯属两个成员中 `x`/`y` 偏移量恰好相同的布局巧合**。同一问题见 `region_mgr.cpp:36` 与 `:45` |
| `region.h:9` | `~Region() = default;` **非虚函数**。目前 15 个 `Region` 对象创建后从不释放（`main.cpp:26-40` 直接 `new`，`RegionMgr` 不销毁），所以未触发问题；一旦引入销毁逻辑即为未定义行为 |
| `region_mgr.h:20` | `~RegionMgr();` 为 private 且 `manager` 静态指针从不重置 → 单例进程内无法回收。`ResMgr`（`res_mgr.h:18`）、`CursorMgr`（`cursor_mgr.h:17`）同理 |
| `bundle.h:5` | `class Bundle : public Region` 实现并 `override` 了 `on_cursor_up` / `on_cursor_down` / `on_render`（`:10-12`），但**全项目没有任何类继承 `Bundle`**。6 个 Bundle 类（`ColaBundle` 等）直接继承 `Region` 并各自重复实现了同样的逻辑 → **`bundle.h` / `bundle.cpp` 是参与编译的死代码，正确抽象层被绕过** |
| 多处 | `Meal` 枚举的 `switch` 分派在 `cursor_mgr.cpp:40-52`、`takeout_box.cpp:41-48`、`microwave_oven.cpp:53-57`、`delivery_driver.cpp:68-84` 至少重复 4 遍。新增一种餐品需要同时修改这 4 处（以及相关 `can_place` 判定），是遗漏的高发点 |
| `res_mgr.cpp:27` | `directory_iterator("resources")` **非递归**，且路径为硬编码相对路径；工作目录不为 `拼好饭传奇\` 时静默加载 0 个资源 |

---

## 8. 常见误判：以下写法不构成对应的 GoF 模式

记录本仓库中容易被误认为某个设计模式、但结构上不满足该模式定义的地方。

### 8.1 `std::function` 单槽回调 ≠ Observer，也不是 Command

出现位置：`Timer::set_on_timeout`（`4/client/timer.h:23-25`、`2/timer.h`、`3/timer.h`）、`CollisionBox::set_on_collide`（`3/collision_box.h:23-25`）、`Animation::set_on_finished`（`5/animation.h:51-53`，成员在 `:121`，触发点 `5/animation.h:18-19`）、`Player` 内各计时器回调（`2/player.h:30`、`:36`、`:41`）、`timer_generate` / `timer_increase_num_per_gen`（`5/main.cpp:109`、`:125`）。

这是**回调注入（callback injection）**：一个对象持有一个 `std::function` 成员，由外部在初始化时填入。判据：

- 不是 Observer——没有 Subject 维护观察者列表，没有 `attach` / `detach`，一个槽只能挂一个回调，后注册的覆盖先注册的，无法多方同时订阅。
- 不是 Command——没有把"请求"封装成对象，没有接收者分离，不具备排队、日志、撤销/重做、事务等 Command 的任何能力。

矩阵中标 ◐ 的 Observer 与 Command 行对应的就是这些位置。

### 8.2 枚举驱动的阶段流 ≠ State

出现位置：4 号 `enum class Stage`（`client.cpp:12-16`）+ 全局 `stage`（`:19`）；2 号 `enum class SceneType`（`scene_manager.h:10-12`）+ `switch` 到三个 `extern Scene*` 全局指针（`:25-35`）；1 号 `Button` 的 `Status::{Idle, Hovered, Pushed}`；5 号 `animation_current = (is_alive ? &animation_run : &animation_explosion)`（`chicken.h:35`）。

State 模式要求**每个状态是一个对象**，状态对象自己持有行为，转移由 Context 统一调度、且转移逻辑可以随状态不同而不同。上述四处都是：一个枚举值 + 一段集中的 `switch`/`if`，状态数据与行为分离，转移规则写死在宿主类里。加一种状态需要改枚举**和**改 `switch`，而不是加一个类。

对照 3 号的严格实现：`StateNode` 基类 + 19 个状态子类 + `StateMachine::switch_to(id)` 统一调度（`state_machine.cpp:22-26`）。

### 8.3 `util.h` 自由函数组 ≈ Facade / Adapter，但不是完整实现

出现位置：`3/util.h`、`4/client/util.h`、`2/util.h`。

这些 `inline` 函数把 `AlphaBlend`（`MSIMG32.lib`）与 `mciSendString`（`WINMM.lib`）包装成 `putimage_ex` / `load_audio` / `play_audio` / `stop_audio`。其中 4 号的 `putimage_ex`（`util.h:13-20`）确实做了实质工作——相机偏移换算、`BLENDFUNCTION` 静态缓存、源矩形可选——判定为 Adapter ✅。其余项目只是 1:1 转发调用，Facade 的"用一个简洁接口封装复杂子系统"并未真正成立，故标 ◐。

### 8.4 共享全局资源 ≠ 完整的 Flyweight

1 号的 `atlas_player_left` 等四个全局 `Atlas*`（`main.cpp:50-53`）、5 号的 `extern Atlas atlas_explosion`（`chicken.h:7-8`）确实做到了"多实例共享同一份不变数据"，享元的**意图成立**。但 Flyweight 还要求由管理方统一维护生命周期与共享策略，而这两处是裸全局指针（1 号还存在重复删除，见 §2.6），因此只能算退化形态。3 号（`ResourcesManager` + 描述表）与 6 号（`ResMgr` 双 `unordered_map`）才有完整的池与查找，才标 ✅。

### 8.5 角色基类带虚函数 ≠ Mediator / Observer

3 号 `Character` 的 `on_input` / `on_update` / `on_render` / `on_hurt`（`character.h:67-71`）、6 号 `Region` 的 4 个虚钩子（`region.h:11-14`），都只是**多态接口定义**。是否存在 Mediator 取决于对象之间是否通过第三方通信：6 号的 `Region` 之间互不引用、一律经 `CursorMgr` 共享状态（`cursor_mgr.h:22-24`），是 Mediator；3 号的角色之间无此间接层，不算。

---

## 9. 问题汇总

按项目汇总全部已核实缺陷，定位信息见各章"已知问题"小节。

| 项目 | 资源 | 单例 | 严格 GoF 模式 | 缺陷数 |
|---|---|---|---|---|
| 1.提瓦特幸存者 | 34 PNG + 1 MP3 | 0 | 1（Template Method） | 6 |
| 2.植物明星大乱斗 | 112 PNG + 11 MP3 + 1 TTF | 0 | 1（Strategy） | 8 |
| 3.空洞武士 | 136 PNG + 20 MP3 | 4 | 7（State / Template Method / Factory Method / Singleton / Adapter / Facade / Flyweight） | 11 |
| 4.哈基米大冒险 | 70 PNG + 11 MP3 + 1 TTF | 0 | 0 | 11 |
| 5.生化危鸡 | 32 PNG + 2 MP3 + 1 TTF | 0 | 0 | 7 |
| 6.拼好饭传奇 | 40 PNG + 8 MP3 | 3 | 4（Mediator / Singleton / Flyweight / Facade） | 12 |

跨项目共性问题：

1. **单例写法统一且不健壮**——3 号与 6 号共 7 个单例全部是静态裸指针 + 非线程安全懒加载，构造/析构私有且指针从不重置。
2. **用 `operator[]` 做查找**——3 号 `state_machine.cpp:19/24`、6 号 `res_mgr.cpp:14/18` 与 `region_mgr.cpp:18` 均用 `map[key]` 代替 `at`/`find`，缺失键会静默插入空指针。6 号的空条目会被无条件解引用，属可崩溃路径。
3. **基类缺虚析构函数**——2 号 3 个角色/子弹基类、5 号 `Chicken`、6 号 `Region`，均存在经基类指针销毁的实际代码，属未定义行为。其中 5 号（`main.cpp:293` `delete chicken`）与 3 号（19 个状态节点）已在运行中触发。
4. **单文件膨胀**——1 号 554 行、8 个类全部在 `main.cpp`，无头文件拆分。
5. **EasyX 依赖未固化**——1–4 号工程不含任何 EasyX 配置，克隆仓库后无法直接编译，必须手工补齐 include 目录与 `EasyXa.lib`。SDL2 项目（5、6）自带依赖，可直接构建。
6. **SDL 配置不一致**——5 号只配了 `Release|x64`，`Debug|x64` 缺路径；6 号两个 x64 配置都配了。两者 Win32 配置均不可用。
7. **硬编码相对路径**——全部项目的资源加载均为相对工作目录的硬编码路径（如 `client.cpp:104` 的 `config.cfg`、`res_mgr.cpp:27` 的 `"resources"`、`client.cpp:68-83` 的 `resources/...`），从其他目录启动即失效，且多数缺失路径无检查。
