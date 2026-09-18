#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbus/dbus.h>

#include "lvgl/lvgl.h"

#ifdef LV_DESKTOP_SIM
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/sdl/sdl.h"
#else
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#endif

#include "lv_lib_png/lv_png.h"
#include "lv_100ask_modules/lv_100ask_modules.h"

#define DISP_HOR_RES 1024
#define DISP_VER_RES 600
#define DISP_BUF_SIZE (DISP_HOR_RES * DISP_VER_RES)

#ifdef LV_DESKTOP_SIM
/*
 * lv_conf.h uses LV_TICK_CUSTOM=1 and obtains time from custom_tick_get().
 * With LV_TICK_CUSTOM enabled LVGL does not provide lv_tick_inc(), however
 * the legacy lv_drivers v8.1 SDL backend still calls lv_tick_inc() from its
 * internal tick thread. Provide a no-op compatibility symbol for the Ubuntu
 * simulator: custom_tick_get() remains the single source of LVGL time.
 */
void lv_tick_inc(uint32_t tick_period)
{
    (void)tick_period;
}

static void prepare_desktop_runtime(void)
{
    /*
     * The original 100ASK deployment expects ./icon next to the executable.
     * In this repository the assets live in ./assets/icon, so create a local
     * compatibility symlink when running from the repository root.
     */
    if(access("./icon", F_OK) != 0 && access("./assets/icon", R_OK) == 0) {
        if(symlink("assets/icon", "icon") == 0) {
            printf("[INIT] created ./icon -> assets/icon\n");
        }
        else {
            perror("[WARN] unable to create ./icon symlink");
        }
    }

    if(access("./icon", R_OK) == 0) {
        printf("[INIT] icon directory ready: ./icon\n");
    }
    else {
        printf("[WARN] ./icon is not readable; desktop images may fail to load\n");
    }
}
#endif

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

#ifdef LV_DESKTOP_SIM
    prepare_desktop_runtime();
#endif

    printf("[INIT] lv_init\n");
    lv_init();

#ifdef LV_DESKTOP_SIM
    printf("[INIT] SDL monitor_init\n");
    monitor_init();
    printf("[INIT] SDL monitor ready\n");
#else
    fbdev_init();
#endif

    static lv_color_t buf[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
#ifdef LV_DESKTOP_SIM
    disp_drv.flush_cb = monitor_flush;
#else
    disp_drv.flush_cb = fbdev_flush;
#endif
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    lv_disp_drv_register(&disp_drv);
    printf("[INIT] display registered\n");

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
#ifdef LV_DESKTOP_SIM
    indev_drv.read_cb = sdl_mouse_read;
#else
    evdev_init();
    indev_drv.read_cb = evdev_read;
#endif
    lv_indev_drv_register(&indev_drv);
    printf("[INIT] input registered\n");

    lv_png_init();
    lv_img_cache_set_size(32);
    printf("[INIT] PNG decoder ready\n");

#ifdef LV_DESKTOP_SIM
    /*
     * The legacy 100ASK DBus handler uses a partially initialized
     * DBusObjectPathVTable. Keep it disabled by default on the native Ubuntu
     * simulator until that module is hardened. Enable explicitly with:
     *   LV_DESKTOP_ENABLE_DBUS=1 ./bin/100ask_lvgl_Main
     */
    const char *enable_dbus = getenv("LV_DESKTOP_ENABLE_DBUS");
    if(enable_dbus != NULL && enable_dbus[0] == '1') {
        printf("[INIT] DBus enabled by LV_DESKTOP_ENABLE_DBUS=1\n");
        lv_100ask_dbus_handler_init("net.ask100.lvgl.Main", "/net/ask100/lvgl/Main");
        printf("[INIT] DBus handler ready\n");
    }
    else {
        printf("[INIT] DBus skipped in Ubuntu simulator\n");
    }
#else
    lv_100ask_dbus_handler_init("net.ask100.lvgl.Main", "/net/ask100/lvgl/Main");
#endif

    printf("[INIT] creating desktop icons\n");
    lv_100ask_demo_init_icon();
    printf("[INIT] desktop icons ready\n");

    while(1) {
        if(1 == is_app_fore_ground()) {
            lv_task_handler();
            usleep(5000);
        }
        else {
            wait_for_become_front_ground();
            lv_100ask_demo_init_icon();
        }
    }

    return 0;
}

uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    return (uint32_t)(now_ms - start_ms);
}
