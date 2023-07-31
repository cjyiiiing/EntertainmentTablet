#ifndef __APP_DESKTOP_H__
#define __APP_DESKTOP_H__

#include "touch_screen.h"
#include "lcd.h"

#define DESKTOP_WALLPAPER_PATH  "./data/dtop/dtop.bmp"    /* 桌面壁纸路径 */

#define APP_TYPE_NUM    4       /* 桌面上可选的按键数量 */

/* 界面类型 */
typedef enum INTERFACE_TYPE {
        INTERFACE_POKEMON = 0,         /* pokemon */
        INTERFACE_PHOTO,                       /* 相册 */
        INTERFACE_DRAW,                          /* 绘画 */
        INTERFACE_UNLOCK,                     /* 解锁 */
        INTERFACE_NONE,                          /* 未选择 */
        INTERFACE_DESKTOP,                    /* 桌面 */
} INTERFACE_TYPE;

INTERFACE_TYPE desktop_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
static char desktop_GetKey(int x, int y);

#endif // !__APP_DESKTOP_H__