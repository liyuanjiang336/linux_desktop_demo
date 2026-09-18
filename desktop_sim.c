#include "desktop_sim.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lvgl/lvgl.h"
#include "lv_100ask_modules/lv_100ask_modules.h"

#define DESKTOP_ICON_DIR        "./icon/"
#define DESKTOP_BG_IMAGE        "./icon/net.ask100.lvgl.bg.png"
#define DESKTOP_ICON_SIZE       64
#define DESKTOP_ICON_ROW_COUNT  4
#define DESKTOP_ICON_COL_COUNT  6
#define DESKTOP_ICON_ROW_SPACE  60
#define DESKTOP_ICON_COL_SPACE  90
#define DESKTOP_ICON_HOR_RES    (4 + (DESKTOP_ICON_SIZE * DESKTOP_ICON_COL_COUNT) + \
                                 (DESKTOP_ICON_COL_SPACE * (DESKTOP_ICON_COL_COUNT - 1)))
#define DESKTOP_ICON_VER_RES    (4 + (DESKTOP_ICON_SIZE * DESKTOP_ICON_ROW_COUNT) + \
                                 (DESKTOP_ICON_ROW_SPACE * (DESKTOP_ICON_ROW_COUNT - 1)))

static void desktop_time_timer_cb(lv_timer_t * timer)
{
    lv_obj_t * label = timer->user_data;
    time_t rawtime;
    struct tm * info;

    time(&rawtime);
    info = localtime(&rawtime);
    if(info == NULL) return;

    lv_label_set_text_fmt(label,
                          "   %02d:%02d  %04d-%02d-%02d",
                          info->tm_hour,
                          info->tm_min,
                          info->tm_year + 1900,
                          info->tm_mon + 1,
                          info->tm_mday);
}

static void desktop_top_bar_create(lv_obj_t * parent)
{
    static lv_style_t top_style;
    static int style_initialized;

    if(!style_initialized) {
        lv_style_init(&top_style);
        lv_style_set_pad_all(&top_style, 0);
        lv_style_set_bg_opa(&top_style, LV_OPA_TRANSP);
        lv_style_set_text_font(&top_style, &lv_font_montserrat_16);
        lv_style_set_border_opa(&top_style, LV_OPA_TRANSP);
        lv_style_set_radius(&top_style, 0);
        lv_style_set_text_color(&top_style, lv_color_hex(0xffffff));
        style_initialized = 1;
    }

    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, LV_PCT(100), 30);
    lv_obj_add_style(panel, &top_style, 0);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t * label_time = lv_label_create(panel);
    lv_label_set_text(label_time, "   --:--  ---- -- --");
    lv_obj_align(label_time, LV_ALIGN_LEFT_MID, 0, 0);
    lv_timer_create(desktop_time_timer_cb, 1000, label_time);

    lv_obj_t * panel_icon = lv_obj_create(panel);
    lv_obj_set_size(panel_icon, 200, 25);
    lv_obj_set_layout(panel_icon, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(panel_icon, LV_BASE_DIR_RTL, 0);
    lv_obj_set_flex_flow(panel_icon, LV_FLEX_FLOW_ROW);
    lv_obj_align(panel_icon, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(panel_icon, &top_style, 0);

    lv_obj_t * spacer = lv_label_create(panel_icon);
    lv_label_set_text(spacer, " ");

    lv_obj_t * battery = lv_label_create(panel_icon);
    lv_label_set_text(battery, LV_SYMBOL_BATTERY_EMPTY);

    lv_obj_t * charge = lv_label_create(battery);
    lv_obj_set_style_text_font(charge, &lv_font_montserrat_14, 0);
    lv_label_set_text(charge, LV_SYMBOL_CHARGE);
    lv_obj_center(charge);

    lv_obj_t * wifi = lv_label_create(panel_icon);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
}

static int has_png_extension(const char * name)
{
    size_t len;

    if(name == NULL) return 0;
    len = strlen(name);
    if(len < 5) return 0;

    return strcmp(name + len - 4, ".png") == 0;
}

static void copy_without_png_extension(char * dst, size_t dst_size, const char * src)
{
    size_t len;

    if(dst_size == 0) return;
    dst[0] = '\0';
    if(src == NULL) return;

    len = strlen(src);
    if(len >= 4 && strcmp(src + len - 4, ".png") == 0) len -= 4;
    if(len >= dst_size) len = dst_size - 1;

    memcpy(dst, src, len);
    dst[len] = '\0';
}

static const char * service_display_name(const char * service_name)
{
    const char * dot;

    if(service_name == NULL) return "";
    dot = strrchr(service_name, '.');
    return dot != NULL ? dot + 1 : service_name;
}

static void desktop_icon_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * hidden_label = lv_obj_get_child(obj, 0);
    if(hidden_label == NULL) return;

    const char * service_name = lv_label_get_text(hidden_label);
    if(service_name == NULL || service_name[0] == '\0') return;

    const char * enable_dbus = getenv("LV_DESKTOP_ENABLE_DBUS");
    if(enable_dbus == NULL || enable_dbus[0] != '1') {
        printf("[DESKTOP] clicked %s (DBus disabled)\n", service_name);
        return;
    }

    char object_path[128];
    int n = snprintf(object_path, sizeof(object_path), "/%s", service_name);
    if(n <= 0 || (size_t)n >= sizeof(object_path)) {
        printf("[WARN] service name too long: %s\n", service_name);
        return;
    }

    for(char * p = object_path; *p != '\0'; ++p) {
        if(*p == '.') *p = '/';
    }

    dbus_method_call(service_name, object_path, service_name, "states", 1, 0);
}

void desktop_sim_init(void)
{
    static lv_style_t container_style;
    static lv_style_t icon_style;
    static lv_style_t bottom_style;
    static int styles_initialized;

    printf("[DESKTOP] create screen\n");

    lv_obj_t * screen = lv_obj_create(NULL);
    if(screen == NULL) {
        printf("[ERROR] unable to create desktop screen\n");
        return;
    }
    lv_scr_load(screen);

    if(!styles_initialized) {
        lv_style_init(&container_style);
        lv_style_set_bg_opa(&container_style, LV_OPA_TRANSP);
        lv_style_set_border_opa(&container_style, LV_OPA_TRANSP);
        lv_style_set_pad_column(&container_style, DESKTOP_ICON_COL_SPACE);
        lv_style_set_pad_row(&container_style, DESKTOP_ICON_ROW_SPACE);
        lv_style_set_pad_all(&container_style, 0);

        lv_style_init(&icon_style);
        lv_style_set_text_opa(&icon_style, LV_OPA_TRANSP);
        lv_style_set_text_font(&icon_style, &lv_font_montserrat_8);

        lv_style_init(&bottom_style);
        lv_style_set_pad_all(&bottom_style, 0);
        lv_style_set_bg_opa(&bottom_style, LV_OPA_50);
        lv_style_set_pad_left(&bottom_style, 10);
        lv_style_set_pad_right(&bottom_style, 10);
        lv_style_set_border_opa(&bottom_style, LV_OPA_TRANSP);
        lv_style_set_radius(&bottom_style, 22);

        styles_initialized = 1;
    }

    printf("[DESKTOP] create background\n");
    lv_obj_t * background = lv_img_create(screen);
    if(background != NULL) {
        lv_img_set_src(background, DESKTOP_BG_IMAGE);
        lv_obj_move_background(background);
    }

    printf("[DESKTOP] create top bar\n");
    desktop_top_bar_create(screen);

    printf("[DESKTOP] create icon container\n");
    lv_obj_t * icon_cont = lv_obj_create(screen);
    lv_obj_set_size(icon_cont, DESKTOP_ICON_HOR_RES, DESKTOP_ICON_VER_RES);
    lv_obj_set_layout(icon_cont, LV_LAYOUT_FLEX);
    lv_obj_set_style_base_dir(icon_cont, LV_BASE_DIR_LTR, 0);
    lv_obj_set_flex_flow(icon_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_center(icon_cont);
    lv_obj_add_style(icon_cont, &container_style, 0);

    lv_obj_t * bottom_panel = lv_obj_create(screen);
    lv_obj_set_size(bottom_panel, LV_PCT(70), 80);
    lv_obj_add_style(bottom_panel, &bottom_style, 0);
    lv_obj_set_layout(bottom_panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bottom_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_panel,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_align(bottom_panel, LV_ALIGN_BOTTOM_MID, 0, -15);

    printf("[DESKTOP] scan %s\n", DESKTOP_ICON_DIR);
    DIR * dr = opendir(DESKTOP_ICON_DIR);
    if(dr == NULL) {
        perror("[ERROR] unable to open icon directory");
        return;
    }

    unsigned int icon_count = 0;
    struct dirent * de;

    while((de = readdir(dr)) != NULL) {
        const char * name = de->d_name;

        if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        if(!has_png_extension(name)) continue;
        if(strcmp(name, "100ask_logo.png") == 0) continue;
        if(strcmp(name, "net.ask100.lvgl.bg.png") == 0) continue;

        char image_path[512];
        int n = snprintf(image_path, sizeof(image_path), "%s%s", DESKTOP_ICON_DIR, name);
        if(n <= 0 || (size_t)n >= sizeof(image_path)) {
            printf("[WARN] icon path too long, skipping: %s\n", name);
            continue;
        }

        char service_name[256];
        copy_without_png_extension(service_name, sizeof(service_name), name);
        if(service_name[0] == '\0') continue;

        printf("[DESKTOP] icon %u: %s\n", icon_count + 1, image_path);

        lv_obj_t * img_icon = lv_img_create(icon_cont);
        if(img_icon == NULL) continue;
        lv_img_set_src(img_icon, image_path);
        lv_obj_add_flag(img_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_style(img_icon, &icon_style, 0);
        lv_obj_add_event_cb(img_icon, desktop_icon_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t * hidden_label = lv_label_create(img_icon);
        lv_obj_set_width(hidden_label, DESKTOP_ICON_SIZE);
        lv_label_set_text(hidden_label, service_name);

        lv_obj_t * label_name = lv_label_create(screen);
        lv_obj_set_style_text_font(label_name, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label_name, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_align(label_name, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(label_name, DESKTOP_ICON_SIZE);
        lv_label_set_text(label_name, service_display_name(service_name));
        lv_obj_align_to(label_name, img_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        lv_obj_move_foreground(label_name);

        if(icon_count < 8) {
            lv_obj_t * bottom_icon = lv_img_create(bottom_panel);
            if(bottom_icon != NULL) {
                lv_img_set_src(bottom_icon, image_path);
                lv_obj_add_flag(bottom_icon, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_style(bottom_icon, &icon_style, 0);
                lv_obj_add_event_cb(bottom_icon, desktop_icon_event_cb, LV_EVENT_CLICKED, NULL);

                lv_obj_t * bottom_hidden_label = lv_label_create(bottom_icon);
                lv_obj_set_width(bottom_hidden_label, DESKTOP_ICON_SIZE);
                lv_label_set_text(bottom_hidden_label, service_name);
            }
        }

        icon_count++;
    }

    closedir(dr);
    printf("[DESKTOP] created %u icons\n", icon_count);
}
