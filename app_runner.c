#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/sdl/sdl.h"
#include "lv_lib_png/lv_png.h"
#include "lv_demos/lv_demo.h"
#include "lv_100ask_demos/lv_100ask_demo.h"

#define APP_HOR_RES 1024
#define APP_VER_RES 600
#define APP_BUF_SIZE (APP_HOR_RES * APP_VER_RES)

void lv_tick_inc(uint32_t tick_period)
{
    (void)tick_period;
}

uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;

    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000ULL + tv_start.tv_usec) / 1000ULL;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    uint64_t now_ms = (tv_now.tv_sec * 1000000ULL + tv_now.tv_usec) / 1000ULL;
    return (uint32_t)(now_ms - start_ms);
}

static void show_unsupported_page(const char * service_name)
{
    lv_obj_t * title = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_label_set_text(title, "Ubuntu simulator");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t * name = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(name, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(name, "%s", service_name);
    lv_obj_align(name, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t * message = lv_label_create(lv_scr_act());
    lv_label_set_text(message,
                      "This application still depends on the embedded Linux\n"
                      "framebuffer / hardware environment and is not ported\n"
                      "to the Ubuntu SDL2 simulator yet.");
    lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(message, LV_ALIGN_CENTER, 0, 50);
}

static void launch_demo(const char * service_name)
{
    if(strcmp(service_name, "net.ask100.lvgl.Game2048") == 0) {
        lv_100ask_demo_2048();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.GameMemory") == 0) {
        lv_100ask_demo_memory();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Snake") == 0) {
        lv_100ask_demo_snake();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Tiles") == 0 ||
            strcmp(service_name, "net.ask100.lvgl.Tron") == 0) {
        lv_100ask_demo_tiles();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Calc") == 0) {
        lv_100ask_demo_calc();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.About") == 0) {
        lv_100ask_demo_about();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Widgets") == 0) {
        lv_demo_widgets();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Benchmark") == 0) {
        lv_demo_benchmark();
    }
    else if(strcmp(service_name, "net.ask100.lvgl.Music") == 0) {
        lv_demo_music();
    }
    else {
        show_unsupported_page(service_name);
    }
}

int main(int argc, char ** argv)
{
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <service-name>\n", argv[0]);
        return 1;
    }

    const char * service_name = argv[1];

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[APP] launching %s\n", service_name);

    lv_init();
    monitor_init();

    static lv_color_t buf[APP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, APP_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = monitor_flush;
    disp_drv.hor_res = APP_HOR_RES;
    disp_drv.ver_res = APP_VER_RES;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    lv_indev_drv_register(&indev_drv);

    lv_png_init();
    lv_img_cache_set_size(16);

    launch_demo(service_name);

    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}
