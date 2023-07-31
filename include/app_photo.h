#ifndef __APP_PHOTO_H__
#define __APP_PHOTO_H__

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <string.h>

#include "DCLinkList.h"
#include "lcd.h"

/* 相册app中的所有界面 */
typedef enum {
        PHOTO_OVERVIEW,                         /* 总览界面 */
        PHOTO_SCROLL,                              /* 卷轴界面 */
        PHOTO_BACKTODESKTOP,          /* 返回桌面 */
} PHOTO_KEY;

/* 总览界面的图片位置序号 */
typedef enum
{
        OVERVIEW_FRAM_1 = 0,
        OVERVIEW_FRAM_2,
        OVERVIEW_FRAM_3,
        OVERVIEW_FRAM_4,
        OVERVIEW_NONE,
} OVERVIEW_KEY;

#define PHOTO_PATH                      "./data/photo/"   /* 相片路径 */

#define PHOTO_IMGPATHLISTLEN            20      /* 数组中最多存放的路径数量 */
#define PHOTO_PATHLEN                             50      /* 路径最大长度 */

/* 总览界面 */
#define PHOTO_OVERVIEW_NUM                      4                             /* 总览界面缩略图的数量 */
#define PHOTO_OVERVIEW_BG_COLOR         LCD_WHITE       /* 总览界面的背景颜色 */
#define PHOTO_FRAME_WIDTH                         2                             /* 相框的宽度 */
#define PHOTO_FRAME_COLOR                         LCD_BLACK       /* 相框的颜色 */

/* 有四个相框,分布顺序如下
     1   3
     2   4
 */
/* 第一个相框的坐标范围 */
#define PHOTO_FRAM_1_X_START  130
#define PHOTO_FRAM_1_X_END    330
#define PHOTO_FRAM_1_Y_START  75
#define PHOTO_FRAM_1_Y_END    195

/* 第二个相框的坐标范围 */
#define PHOTO_FRAM_2_X_START  130
#define PHOTO_FRAM_2_X_END    330
#define PHOTO_FRAM_2_Y_START  275
#define PHOTO_FRAM_2_Y_END    395

/* 第三个相框的坐标范围 */
#define PHOTO_FRAM_3_X_START  460
#define PHOTO_FRAM_3_X_END    660
#define PHOTO_FRAM_3_Y_START  75
#define PHOTO_FRAM_3_Y_END    195

/* 第四个相框的坐标范围 */
#define PHOTO_FRAM_4_X_START  460
#define PHOTO_FRAM_4_X_END    660
#define PHOTO_FRAM_4_Y_START  275
#define PHOTO_FRAM_4_Y_END    395

/* 卷轴界面 */
#define PHOTO_SCROLL_LEFT_BOUNDARY         0                                    /* 卷轴最左侧的左边界 */
#define PHOTO_SCROLL_RIGHT_BOUNDARY      2 * LCD_WIDTH       /* 卷轴最左侧的右边界 */

bool photo_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
static PHOTO_KEY photo_overview(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
static PHOTO_KEY photo_scroll(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
static int cmp(const void *a, const void *b);
static bool photo_GetImgPaths(void);
static void photo_DrawPhotoFrame(int *fb_base, int lw, int color);
static void photo_PutImgs(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);
bool cmpNode(datatype listNode, datatype data);
static char photo_GetKey(int x, int y);
static void photo_DrawScrollBuf(int scrollIndex);
void photo_showCurScroll(int *fb_base, int scrollLeft, int scrollRight);

#endif // !__APP_PHOTO_H__