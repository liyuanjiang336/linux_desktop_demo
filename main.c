#include <unistd.h>
#include <time.h>
#include <sys/time.h>
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
#endif

int main(void)
{
    lv_init();

#ifdef LV_DESKTOP_SIM
    /* Ubuntu desktop: SDL2 provides both the display window and mouse input. */
    monitor_init();
#else
    /* Embedded Linux: use /dev/fb0 and evdev. */
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

    lv_png_init();
    lv_img_cache_set_size(32);

    /* DBus is retained on Ubuntu so the original desktop IPC architecture works. */
    lv_100ask_dbus_handler_init("net.ask100.lvgl.Main", "/net/ask100/lvgl/Main");

    lv_100ask_demo_init_icon();

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
