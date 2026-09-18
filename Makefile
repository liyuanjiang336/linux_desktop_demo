#
# Makefile
#
# Ubuntu 20.04 desktop build (default):
#   make clean && make -j$(nproc)
#
# i.MX6ULL cross build:
#   make clean && make PLATFORM=imx6ull -j$(nproc) RUN_JOBS=-j$(nproc)
#

PLATFORM ?= ubuntu
export LVGL_DIR_NAME ?= lvgl
export LVGL_DIR ?= ${shell pwd}

COMMON_WARNINGS := -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers \
	-Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith \
	-fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized \
	-Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits \
	-Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual \
	-Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar \
	-Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic \
	-Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion \
	-Wclobbered -Wdeprecated -Wempty-body -Wshift-negative-value \
	-Wstack-usage=2048 -Wno-unused-value

ifeq ($(PLATFORM),ubuntu)
	# Native Ubuntu desktop build. lv_drv_conf.h selects the legacy
	# USE_MONITOR=1 configuration, which is implemented by lv_drivers/sdl/sdl.c.
	# Do NOT define USE_SDL at the same time: lv_drivers v8.1 rejects
	# USE_MONITOR && USE_SDL with a preprocessor #error.
	ifeq ($(origin CC), default)
		CC := gcc
	endif
	export CC
	export CFLAGS := -O2 -g $(COMMON_WARNINGS) -I$(LVGL_DIR) \
		-DLV_DESKTOP_SIM=1 \
		$(shell pkg-config --cflags sdl2 dbus-1 2>/dev/null)
	export LDFLAGS := -lm -lpthread $(shell pkg-config --libs sdl2 dbus-1 2>/dev/null)
	BUILD_APPS ?= 0
else ifeq ($(PLATFORM),imx6ull)
	# Original i.MX6ULL cross-compile configuration.
	export CC := arm-buildroot-linux-gnueabihf-gcc
	export CFLAGS := -O3 -g0 -I$(LVGL_DIR) $(COMMON_WARNINGS) \
		-I ~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/include \
		-I ~/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/lib/dbus-1.0/include
	export LDFLAGS := -lm -ldbus-1 -lpthread
	BUILD_APPS ?= 1
else
	$(error Unsupported PLATFORM '$(PLATFORM)'. Use PLATFORM=ubuntu or PLATFORM=imx6ull)
endif

BIN = 100ask_lvgl_Main
RUN_JOBS =
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

# lv_drivers release/v8.1 lv_drivers.mk does not add lv_drivers/sdl/*.c.
# The Ubuntu desktop entry point uses sdl_init/sdl_display_flush/sdl_mouse_read,
# so explicitly compile the SDL backend for the native desktop build.
ifeq ($(PLATFORM),ubuntu)
CSRCS += $(wildcard $(LVGL_DIR)/lv_drivers/sdl/*.c)
endif

include $(LVGL_DIR)/lv_lib_png/lv_lib_png.mk
include $(LVGL_DIR)/lv_100ask_modules/lv_100ask_modules.mk

OBJEXT ?= .o
AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
MAINOBJ = $(MAINSRC:.c=$(OBJEXT))
SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

# Standalone application binaries. They still use the embedded framebuffer path,
# therefore they are enabled by default only for PLATFORM=imx6ull.
SRC_DIR  +=  lv_100ask_app/src/general_app/general_file_manager \
			 lv_100ask_app/src/general_app/general_2048_game \
			 lv_100ask_app/src/general_app/general_game_memory \
			 lv_100ask_app/src/general_app/general_game_snake \
			 lv_100ask_app/src/general_app/general_game_tiles \
			 lv_100ask_app/src/general_app/general_calc \
			 lv_100ask_app/src/general_app/general_widgets \
			 lv_100ask_app/src/general_app/general_benchmark \
			 lv_100ask_app/src/general_app/general_music_player \
			 lv_100ask_app/src/general_app/general_about

ifeq ($(PLATFORM),imx6ull)
SRC_DIR  +=  lv_100ask_app/src/imx6ull_app/imx6ull_set_lcd_brightness \
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_time \
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_wlan \
			 lv_100ask_app/src/imx6ull_app/imx6ull_set_lan
endif

.PHONY: all default apps clean check-deps prepare-runtime

all: check-deps default prepare-runtime
ifeq ($(BUILD_APPS),1)
	$(foreach dir,$(SRC_DIR),$(MAKE) $(RUN_JOBS) -C $(dir);)
endif

check-deps:
ifeq ($(PLATFORM),ubuntu)
	@command -v pkg-config >/dev/null 2>&1 || { echo "ERROR: pkg-config is required"; exit 1; }
	@pkg-config --exists sdl2 || { echo "ERROR: SDL2 development package not found. Install: sudo apt install libsdl2-dev"; exit 1; }
	@pkg-config --exists dbus-1 || { echo "ERROR: DBus development package not found. Install: sudo apt install libdbus-1-dev"; exit 1; }
endif

prepare-runtime:
ifeq ($(PLATFORM),ubuntu)
	@if [ ! -e "$(LVGL_DIR)/icon" ]; then \
		ln -s assets/icon "$(LVGL_DIR)/icon"; \
		echo "Created runtime asset link: icon -> assets/icon"; \
	fi
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
	mkdir -p $(LVGL_DIR)/obj $(LVGL_DIR)/bin
	mv *.o $(LVGL_DIR)/obj/
	mv $(BIN) $(LVGL_DIR)/bin/

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) ./bin/* ./obj/*
	$(foreach dir,$(SRC_DIR),$(MAKE) -C $(dir) clean;)
