#ifndef __APP_BOOT_H__
#define __APP_BOOT_H__

#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#include "debug.h"

/* 路径 */
#define BOOT_ANIMATION_PATH "./data/boot_anima/"   /* 保存开机动画的路径 */
#define BOOT_WALLPAPER_PATH "./data/boot_anima/boot_wallpaper.bmp"  /* 开机壁纸 */

/* 进度条 */
#define BOOT_PROGRESS_BAR_X     0       /* 进度条起始x坐标 */
#define BOOT_PROGRESS_BAR_Y     350 /* 进度条起始y坐标 */
#define BOOT_PROGRESS_BAR_COLOR 0x0037475E      /* 进度条颜色 */

/* 皮卡丘 */
#define BOOT_PIKACHU_NUM        4       /* 皮卡丘的数量 */

/* 文字 */
#define BOOT_TEXT_COLOR_MAX       0x0000FF00  /* 文字颜色数值上限 */
#define BOOT_TEXT_COLOR_MIN     0x00003300      /* 文字颜色数值下限 */
#define BOOT_TEXT_COLOR_ADD       (0x11<<8)   /* 字体变化速度 */
#define BOOT_TEXT_LOADING       "Loading..."
#define BOOT_TEXT_FINISH                "Finish"
#define BOOT_TEXT_X     300
#define BOOT_TEXT_Y     250

bool play_boot_animation(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);

#endif // !__APP_BOOT_H__