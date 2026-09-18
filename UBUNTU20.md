# Ubuntu 20.04 编译与运行

本分支支持在 Ubuntu 20.04 桌面环境直接使用本机 GCC 编译，并通过 SDL2 窗口显示 LVGL 桌面，不再要求 `/dev/fb0` 和 `/dev/input/eventX`。

## 1. 安装依赖

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    pkg-config \
    libsdl2-dev \
    libdbus-1-dev \
    dbus-x11 \
    git
```

## 2. 获取代码

```bash
git clone --recursive https://github.com/liyuanjiang336/linux_desktop_demo.git
cd linux_desktop_demo
git checkout ubuntu20.04-support
git submodule update --init --recursive
```

如果仓库已经克隆：

```bash
git fetch origin
git checkout ubuntu20.04-support
git pull
git submodule update --init --recursive
```

## 3. 编译

Ubuntu 20.04 为默认构建模式：

```bash
make clean
make -j$(nproc)
```

编译成功后生成：

```text
bin/100ask_lvgl_Main
```

## 4. 运行

程序仍使用原项目的 DBus 进程间通信机制，因此先启动 DBus session：

```bash
export $(dbus-launch)
./bin/100ask_lvgl_Main
```

程序会创建一个 1024x600 的 SDL2 窗口，并使用鼠标作为 LVGL 指针输入设备。

如果当前桌面会话已经存在 `DBUS_SESSION_BUS_ADDRESS`，可以直接运行：

```bash
./bin/100ask_lvgl_Main
```

## 5. 常见问题

### SDL2 找不到

```text
ERROR: SDL2 development package not found
```

安装：

```bash
sudo apt install libsdl2-dev
```

### DBus 头文件找不到

```text
fatal error: dbus/dbus.h: No such file or directory
```

安装：

```bash
sudo apt install libdbus-1-dev
```

根 Makefile 使用 `pkg-config` 自动获取 Ubuntu 的 DBus 头文件及链接路径，不再依赖原 i.MX6ULL SDK 中写死的 include 路径。

### 没有 DBus session

安装并启动：

```bash
sudo apt install dbus-x11
export $(dbus-launch)
```

### 子模块未初始化

```bash
git submodule update --init --recursive
```

## 6. i.MX6ULL 交叉编译

原开发板构建方式仍保留：

```bash
make clean
make PLATFORM=imx6ull -j$(nproc) RUN_JOBS=-j$(nproc)
```

该模式继续使用 `arm-buildroot-linux-gnueabihf-gcc`、framebuffer `/dev/fb0` 和 evdev。

## 7. 当前平台差异

| 项目 | Ubuntu 20.04 | i.MX6ULL |
|---|---|---|
| 编译器 | gcc | arm-buildroot-linux-gnueabihf-gcc |
| 显示 | SDL2 窗口 | /dev/fb0 |
| 输入 | SDL2 鼠标 | evdev |
| 分辨率 | 1024x600 | 1024x600 |
| DBus | 系统 libdbus-1 | SDK libdbus-1 |
| APP 子程序默认构建 | 否 | 是 |

Ubuntu 模式目前优先保证主桌面程序 `100ask_lvgl_Main` 可以本机构建和运行；原项目中的独立 APP 子程序仍按嵌入式 framebuffer 架构保留，因此 Ubuntu 默认不批量构建这些 APP。
