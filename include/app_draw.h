#ifndef __APP_DRAW_H__
#define __APP_DRAW_H__

#include <stdio.h>
#include <time.h>

#include "lcd.h"
#include "touch_screen.h"
#include "app_photo.h"

typedef enum DRAW_KEY {
        DRAW_PALETTE,             /* 调色盘 */
        DRAW_BRUSH_S,           /* S号画笔 */
        DRAW_BRUSH_M,          /* M号画笔 */
        DRAW_BRUSH_L,           /* L号画笔 */
        DRAW_ERASER,               /* 橡皮擦 */
        DRAW_BACK,                    /* 返回 */
        DRAW_NONE,                   /* 无选择 */
} DRAW_KEY;

#define DRAW_WALLPAPER_PATH     "./data/artmaster/artmaster.bmp"  /* 画板图图片路径 */
#define DRAW_PALETTE_PATH     "./data/artmaster/palette.bmp"  /* 调色盘路径 */

/* 画笔大小选择 */
#define DRAW_BRUSH_S_X           120             /* S号画笔 */
#define DRAW_BRUSH_S_Y            40
#define DRAW_BRUSH_S_R           12
#define DRAW_BRUSH_M_X          200             /* M号画笔 */
#define DRAW_BRUSH_M_Y           40
#define DRAW_BRUSH_M_R          18
#define DRAW_BRUSH_L_X            280            /* L号画笔 */
#define DRAW_BRUSH_L_Y            40
#define DRAW_BRUSH_L_R           24

/* 调色盘位置 */
#define DRAW_PALETTE_X          360
#define DRAW_PALETTE_Y          16

/* 橡皮擦位置 */
#define DRAW_ERASER_X_START           640
#define DRAW_ERASER_X_END               720
#define DRAW_ERASER_Y_START           0
#define DRAW_ERASER_Y_END               80

/* 退出位置 */
#define DRAW_BACK_X_START               720
#define DRAW_BACK_X_END                   800
#define DRAW_BACK_Y_START               0
#define DRAW_BACK_Y_END                   80

#define DRAW_W          LCD_WIDTH                                                       /* 画板宽度 */
// #define DRAW_H          LCD_HEIGHT - DRAW_BACK_Y_END       /* 画板高度 */
#define DRAW_H          LCD_HEIGHT       /* 画板高度 */

/* 询问是否保存绘制的图像 */
#define DRAW_SAVE_STR   "SAVE?"         /* 询问是否保存绘图 */
#define DRAW_SAVE_X          350                /* "save?"显示的起始x坐标 */
#define DRAW_SAVE_Y          150                /* "save?"显示的起始y坐标 */
#define DRAW_YES_STR      "yes"              /* 保存绘图 */
#define DRAW_YES_X             300                /* "save?"显示的起始x坐标 */
#define DRAW_YES_Y             250                /* "save?"显示的起始y坐标 */
#define DRAW_NO_STR        "no"              /* 不保存绘图 */
#define DRAW_NO_X              450                /* "save?"显示的起始x坐标 */
#define DRAW_NO_Y              250                /* "save?"显示的起始y坐标 */

bool draw_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
void draw_ShowBoard(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
DRAW_KEY draw_GetKey(int x, int y);
static void draw_SaveBmp(const char *filename, int *fb_base);
bool draw_IfSaveImg(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);

#endif // !__APP_DRAW_H__