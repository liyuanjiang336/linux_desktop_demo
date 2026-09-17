/**
 * @file lv_drv_conf.h
 * Configuration file for v8.1.0-dev
 */

#if 1 /*Set it to "1" to enable the content*/

#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

#include "lv_conf.h"

/* Platform defaults.
 * LV_DESKTOP_SIM is defined by the Ubuntu native build in the root Makefile.
 */
#ifdef LV_DESKTOP_SIM
#  define USE_EVDEV  0
#  define USE_FBDEV  0
#  define USE_MONITOR 1
#else
#  define USE_EVDEV  1
#  define USE_FBDEV  1
#endif

/*********************
 * DELAY INTERFACE
 *********************/
#define LV_DRV_DELAY_INCLUDE  <stdint.h>
#define LV_DRV_DELAY_US(us)
#define LV_DRV_DELAY_MS(ms)

/*********************
 * DISPLAY INTERFACE
 *********************/
#define LV_DRV_DISP_INCLUDE         <stdint.h>
#define LV_DRV_DISP_CMD_DATA(val)
#define LV_DRV_DISP_RST(val)
#define LV_DRV_DISP_SPI_CS(val)
#define LV_DRV_DISP_SPI_WR_BYTE(data)
#define LV_DRV_DISP_SPI_WR_ARRAY(adr, n)
#define LV_DRV_DISP_PAR_CS(val)
#define LV_DRV_DISP_PAR_SLOW
#define LV_DRV_DISP_PAR_FAST
#define LV_DRV_DISP_PAR_WR_WORD(data)
#define LV_DRV_DISP_PAR_WR_ARRAY(adr, n)

/***************************
 * INPUT DEVICE INTERFACE
 ***************************/
#define LV_DRV_INDEV_INCLUDE     <stdint.h>
#define LV_DRV_INDEV_RST(val)
#define LV_DRV_INDEV_IRQ_READ    0
#define LV_DRV_INDEV_SPI_CS(val)
#define LV_DRV_INDEV_SPI_XCHG_BYTE(data)    0
#define LV_DRV_INDEV_I2C_START
#define LV_DRV_INDEV_I2C_STOP
#define LV_DRV_INDEV_I2C_RESTART
#define LV_DRV_INDEV_I2C_WR(data)
#define LV_DRV_INDEV_I2C_READ(last_read)    0

/*********************
 *  DISPLAY DRIVERS
 *********************/
#ifndef USE_MONITOR
#  define USE_MONITOR         0
#endif

#if USE_MONITOR
#  define MONITOR_HOR_RES     1024
#  define MONITOR_VER_RES     600
#  define MONITOR_ZOOM        1
#  define MONITOR_DOUBLE_BUFFERED 0
#  define MONITOR_SDL_INCLUDE_PATH <SDL2/SDL.h>
#  define MONITOR_DUAL        0
#endif

#ifndef USE_WINDOWS
#  define USE_WINDOWS       0
#endif
#if USE_WINDOWS
#  define WINDOW_HOR_RES      480
#  define WINDOW_VER_RES      320
#endif

#ifndef USE_WIN32DRV
#  define USE_WIN32DRV       0
#endif
#if USE_WIN32DRV
#  define WIN32DRV_MONITOR_ZOOM 1
#endif

#ifndef USE_GTK
#  define USE_GTK       0
#endif

#ifndef USE_WAYLAND
#  define USE_WAYLAND       0
#endif
#if USE_WAYLAND
#  define WAYLAND_HOR_RES      480
#  define WAYLAND_VER_RES      320
#  define WAYLAND_SURF_TITLE   "LVGL"
#endif

#ifndef USE_SSD1963
#  define USE_SSD1963         0
#endif
#if USE_SSD1963
#  define SSD1963_HOR_RES     LV_HOR_RES
#  define SSD1963_VER_RES     LV_VER_RES
#  define SSD1963_HT          531
#  define SSD1963_HPS         43
#  define SSD1963_LPS         8
#  define SSD1963_HPW         10
#  define SSD1963_VT          288
#  define SSD1963_VPS         12
#  define SSD1963_FPS         4
#  define SSD1963_VPW         10
#  define SSD1963_HS_NEG      0
#  define SSD1963_VS_NEG      0
#  define SSD1963_ORI         0
#  define SSD1963_COLOR_DEPTH 16
#endif

#ifndef USE_R61581
#  define USE_R61581          0
#endif
#if USE_R61581
#  define R61581_HOR_RES      LV_HOR_RES
#  define R61581_VER_RES      LV_VER_RES
#  define R61581_HSPL         0
#  define R61581_HSL          10
#  define R61581_HFP          10
#  define R61581_HBP          10
#  define R61581_VSPL         0
#  define R61581_VSL          10
#  define R61581_VFP          8
#  define R61581_VBP          8
#  define R61581_DPL          0
#  define R61581_EPL          1
#  define R61581_ORI          0
#  define R61581_LV_COLOR_DEPTH 16
#endif

#ifndef USE_ST7565
#  define USE_ST7565          0
#endif
#ifndef USE_GC9A01
#  define USE_GC9A01          0
#endif

#ifndef USE_UC1610
#  define USE_UC1610          0
#endif
#if USE_UC1610
#  define UC1610_HOR_RES         LV_HOR_RES
#  define UC1610_VER_RES         LV_VER_RES
#  define UC1610_INIT_CONTRAST   33
#  define UC1610_INIT_HARD_RST   0
#  define UC1610_TOP_VIEW        0
#endif

#ifndef USE_SHARP_MIP
#  define USE_SHARP_MIP       0
#endif
#if USE_SHARP_MIP
#  define SHARP_MIP_HOR_RES             LV_HOR_RES
#  define SHARP_MIP_VER_RES             LV_VER_RES
#  define SHARP_MIP_SOFT_COM_INVERSION  0
#  define SHARP_MIP_REV_BYTE(b)
#endif

#ifndef USE_ILI9341
#  define USE_ILI9341       0
#endif
#if USE_ILI9341
#  define ILI9341_HOR_RES       LV_HOR_RES
#  define ILI9341_VER_RES       LV_VER_RES
#  define ILI9341_GAMMA         1
#  define ILI9341_TEARING       0
#endif

#ifndef USE_FBDEV
#  define USE_FBDEV           1
#endif
#if USE_FBDEV
#  define FBDEV_PATH          "/dev/fb0"
#endif

#ifndef USE_BSD_FBDEV
#  define USE_BSD_FBDEV       0
#endif
#if USE_BSD_FBDEV
#  define FBDEV_PATH          "/dev/fb0"
#endif

#ifndef USE_DRM
#  define USE_DRM             0
#endif
#if USE_DRM
#  define DRM_CARD            "/dev/dri/card0"
#  define DRM_CONNECTOR_ID    -1
#endif

/*********************
 *  INPUT DEVICES
 *********************/
#ifndef USE_XPT2046
#  define USE_XPT2046         0
#endif
#if USE_XPT2046
#  define XPT2046_HOR_RES     480
#  define XPT2046_VER_RES     320
#  define XPT2046_X_MIN       200
#  define XPT2046_Y_MIN       200
#  define XPT2046_X_MAX       3800
#  define XPT2046_Y_MAX       3800
#  define XPT2046_AVG         4
#  define XPT2046_X_INV       0
#  define XPT2046_Y_INV       0
#  define XPT2046_XY_SWAP     0
#endif

#ifndef USE_FT5406EE8
#  define USE_FT5406EE8       0
#endif
#if USE_FT5406EE8
#  define FT5406EE8_I2C_ADR   0x38
#endif

#ifndef USE_AD_TOUCH
#  define USE_AD_TOUCH        0
#endif

#ifndef USE_MOUSE
#  define USE_MOUSE           0
#endif
#ifndef USE_MOUSEWHEEL
#  define USE_MOUSEWHEEL      0
#endif

#ifndef USE_LIBINPUT
#  define USE_LIBINPUT        0
#endif
#if USE_LIBINPUT
#  define LIBINPUT_NAME       "/dev/input/event1"
#endif

#ifndef USE_EVDEV
#  define USE_EVDEV           0
#endif
#ifndef USE_BSD_EVDEV
#  define USE_BSD_EVDEV       0
#endif
#if USE_EVDEV || USE_BSD_EVDEV
#  define EVDEV_NAME          "/dev/input/event1"
#  define EVDEV_SWAP_AXES     0
#  define EVDEV_CALIBRATE     0
#  if EVDEV_CALIBRATE
#    define EVDEV_HOR_MIN     0
#    define EVDEV_HOR_MAX     4096
#    define EVDEV_VER_MIN     0
#    define EVDEV_VER_MAX     4096
#  endif
#endif

#ifndef USE_KEYBOARD
#  define USE_KEYBOARD        0
#endif

#endif /* LV_DRV_CONF_H */
#endif /* content enable */
