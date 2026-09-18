# CHANGELOG

本文件用于记录 `linux_desktop_demo` 的功能修改、平台适配、问题修复和已知限制。

---

## 2026-09-18 - Ubuntu 20.04 / SDL2 桌面适配

目标：在不破坏原 i.MX6ULL framebuffer/evdev 架构的前提下，使工程可以在 Ubuntu 20.04 桌面环境使用本机 GCC + SDL2 编译和运行。

### 1. 新增 Ubuntu 20.04 构建模式

修改文件：

- `Makefile`
- `.github/workflows/ubuntu20-build.yml`
- `UBUNTU20.md`

主要修改：

- 增加 `PLATFORM` 构建选项：
  - `PLATFORM=ubuntu`：Ubuntu 20.04 本机构建，当前默认模式。
  - `PLATFORM=imx6ull`：保留原 i.MX6ULL 交叉编译模式。
- Ubuntu 使用本机 `gcc`。
- i.MX6ULL 继续使用：
  - `arm-buildroot-linux-gnueabihf-gcc`
  - `/dev/fb0`
  - `/dev/input/eventX`
- Ubuntu 依赖使用 `pkg-config` 自动获取：
  - SDL2
  - DBus
- 增加 Ubuntu 20.04 GitHub Actions 编译流程。
- 增加 `UBUNTU20.md`，记录 Ubuntu 依赖安装、编译和运行方式。

### 2. SDL2 显示和鼠标输入支持

修改文件：

- `main.c`
- `lv_drv_conf.h`
- `Makefile`

主要修改：

- Ubuntu 模式不再使用 framebuffer 和 evdev。
- 显示切换为 LVGL `lv_drivers` 的 SDL2/monitor backend。
- 鼠标使用 `sdl_mouse_read()`。
- 分辨率保持：
  - 1024 x 600
- i.MX6ULL 模式仍保持：
  - `fbdev_init()`
  - `fbdev_flush()`
  - `evdev_init()`
  - `evdev_read()`

### 3. 修复 lv_drivers v8.1 SDL 源文件未参与编译

问题：

原 `lv_drivers/release/v8.1/lv_drivers.mk` 不会自动加入：

```text
lv_drivers/sdl/*.c
```

因此曾出现：

```text
undefined reference to 'sdl_init'
undefined reference to 'sdl_display_flush'
undefined reference to 'sdl_mouse_read'
```

处理：

Ubuntu 构建时显式加入：

```make
CSRCS += $(wildcard $(LVGL_DIR)/lv_drivers/sdl/*.c)
```

### 4. 修复 USE_MONITOR / USE_SDL 配置冲突

问题：

LVGL v8.1 的 SDL 驱动禁止同时启用：

```c
USE_MONITOR = 1
USE_SDL     = 1
```

否则 `sdl.c` 会直接触发：

```c
#error "Cannot enable both MONITOR and SDL at the same time."
```

处理：

- Ubuntu 模式使用旧版兼容入口 `USE_MONITOR=1`。
- 不再额外定义 `USE_SDL=1`。
- 由 `lv_drv_conf.h` 根据 `LV_DESKTOP_SIM` 自动选择驱动。

### 5. 修复 LV_TICK_CUSTOM 与 SDL tick 链接冲突

问题：

当前 `lv_conf.h` 使用：

```c
#define LV_TICK_CUSTOM 1
```

LVGL 时间由：

```c
custom_tick_get()
```

提供，但旧版 SDL backend 内部仍会调用：

```c
lv_tick_inc()
```

因此曾出现：

```text
undefined reference to 'lv_tick_inc'
```

处理：

Ubuntu SDL 模式增加兼容 `lv_tick_inc()` 空实现。

实际 LVGL 时间仍然只由 `custom_tick_get()` 提供，避免双重 tick 累加。

### 6. 增加 Ubuntu 启动阶段日志

修改文件：

- `main.c`

增加启动阶段日志，例如：

```text
[INIT] icon directory ready: ./icon
[INIT] lv_init
[INIT] SDL monitor_init
[INIT] SDL monitor ready
[INIT] display registered
[INIT] input registered
[INIT] PNG decoder ready
[INIT] DBus skipped in Ubuntu simulator
[INIT] creating desktop icons
[INIT] desktop icons ready
```

用于快速定位初始化阶段崩溃位置。

### 7. 修复桌面图标初始化 Segmentation fault

问题来源：

当前工程使用的 `lv_100ask_modules` 为较老版本。

旧版 `lv_100ask_demo_init_icon()` 存在以下问题：

1. 使用：

```c
opendir("./")
```

扫描工程根目录，而不是图标目录。

2. 未检查文件名长度就执行：

```c
de->d_name + strlen(de->d_name) - 4
```

当遇到 `bin`、`obj` 等长度小于 4 的目录名时会产生越界访问。

3. 背景对象 `img_gb` 可能未初始化，之后仍可能执行：

```c
lv_obj_move_background(img_gb);
```

以上问题会导致：

```text
Segmentation fault (core dumped)
```

处理：

新增：

- `desktop_sim.c`
- `desktop_sim.h`

Ubuntu 模式不再直接使用旧版 `lv_100ask_demo_init_icon()`，改用安全实现：

```c
desktop_sim_init();
```

新的 Ubuntu 桌面初始化逻辑：

- 只扫描 `./icon/`
- 先检查文件名长度再判断 `.png`
- 不直接修改 `readdir()` 返回的文件名
- 使用完整图片路径
- 明确初始化背景对象
- 增加逐图标加载日志
- i.MX6ULL 仍继续使用原桌面初始化逻辑

### 8. 修复 Ubuntu 图标资源路径

仓库中的图标资源实际位于：

```text
assets/icon/
```

而旧桌面代码要求：

```text
./icon/
```

处理：

Ubuntu 构建/运行时自动建立：

```text
icon -> assets/icon
```

从而保持旧资源路径兼容。

### 9. Ubuntu 模式暂时禁用旧 DBus 后台线程

问题：

旧版 `lv_100ask_dbus_handler` 使用的 `DBusObjectPathVTable` 没有完整初始化，Ubuntu PC 环境运行时存在稳定性风险。

处理：

Ubuntu 主桌面默认跳过旧 DBus handler。

如需要测试旧 DBus，可使用：

```bash
LV_DESKTOP_ENABLE_DBUS=1 ./bin/100ask_lvgl_Main
```

i.MX6ULL 原 DBus 架构不变。

### 10. 增加 Ubuntu APP Runner

新增文件：

- `app_runner.c`

Ubuntu 构建现在生成两个程序：

```text
bin/100ask_lvgl_Main
bin/100ask_lvgl_AppRunner
```

桌面点击 APP 图标后：

1. 主桌面接收到 LVGL 点击事件。
2. 使用 `fork()` 创建子进程。
3. 使用 `exec()` 启动：

```text
bin/100ask_lvgl_AppRunner <service-name>
```

4. 子 APP 使用独立 SDL2 窗口运行。

这种方式比在主进程中直接切换 Demo 更接近原工程“一 APP 一进程”的架构，同时避免多个 Demo 的 LVGL timer/object 相互影响。

### 11. Ubuntu 当前已支持的 APP

以下 APP 已接入 Ubuntu SDL2 App Runner：

| 桌面 APP | Ubuntu 状态 | 实际入口 |
|---|---|---|
| Game2048 | 已支持 | `lv_100ask_demo_2048()` |
| GameMemory | 已支持 | `lv_100ask_demo_memory()` |
| Snake | 已支持 | `lv_100ask_demo_snake()` |
| Tiles | 已支持 | `lv_100ask_demo_tiles()` |
| Calc | 已支持 | `lv_100ask_demo_calc()` |
| About | 已支持 | `lv_100ask_demo_about()` |
| Widgets | 已支持 | `lv_demo_widgets()` |
| Benchmark | 已支持 | `lv_demo_benchmark()` |
| Music | 已支持 | `lv_demo_music()` |

### 12. Ubuntu 当前尚未完整移植的 APP

以下 APP 仍依赖嵌入式 Linux / i.MX6ULL 环境：

| APP | 当前 Ubuntu 状态 | 原因 |
|---|---|---|
| FileManager | 未完整移植 | 独立 APP 源码，尚未接入 SDL Runner |
| SetTime | 未完整移植 | 原实现按开发板系统环境设计 |
| SetBrightness | 未完整移植 | 依赖开发板 backlight/sysfs |
| WLAN | 未完整移植 | 依赖开发板网络配置接口 |
| LAN | 未完整移植 | 依赖开发板网络配置接口 |

点击以上图标时，Ubuntu App Runner 会显示：

```text
Ubuntu simulator

This application still depends on the embedded Linux
framebuffer / hardware environment and is not ported
to the Ubuntu SDL2 simulator yet.
```

避免出现点击无响应或直接访问不存在的硬件节点。

### 13. 鼠标点击链路确认

Ubuntu SDL2 当前已确认：

```text
SDL mouse
  -> sdl_mouse_read()
  -> LVGL indev
  -> LV_EVENT_CLICKED
  -> desktop_icon_event_cb()
  -> fork/exec AppRunner
```

点击日志示例：

```text
[DESKTOP] clicked net.ask100.lvgl.Game2048
[DESKTOP] launched net.ask100.lvgl.Game2048, pid=xxxx
```

### 14. 当前构建方式

Ubuntu 20.04：

```bash
make clean
find . -name "*.o" -delete
make -j$(nproc)
```

运行主桌面：

```bash
./bin/100ask_lvgl_Main
```

单独测试 APP Runner：

```bash
./bin/100ask_lvgl_AppRunner net.ask100.lvgl.Calc
./bin/100ask_lvgl_AppRunner net.ask100.lvgl.Game2048
```

i.MX6ULL：

```bash
make clean
make PLATFORM=imx6ull -j$(nproc) RUN_JOBS=-j$(nproc)
```

---

## 后续计划

后续修改继续追加到本文件，当前建议优先级：

1. 移植 FileManager 到 Ubuntu SDL2。
2. 移植 SetTime。
3. 为 LAN / WLAN 增加 Ubuntu 网络接口适配层。
4. 为 SetBrightness 增加 Ubuntu backlight 适配层。
5. 整理 DBus 模块，使 Ubuntu 和 i.MX6ULL 可以共享更安全的进程通信实现。
6. 补充 APP Runner 的关闭、单实例和进程管理机制。
7. 继续完善 Ubuntu CI，增加运行级 smoke test。
