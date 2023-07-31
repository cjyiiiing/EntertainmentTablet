#ifndef __APP_UNLOCK_H__
#define __APP_UNLOCK_H__

#include <pthread.h>
#include <linux/input.h>
#include <string.h>
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "touch_screen.h"
#include "lcd.h"
#include "debug.h"

#define UNLOCK_WALLPAPER_PATH   "./data/unlock/unlock_wallpaper.bmp"  /* 壁纸 */
#define UNLOCK_FEEDBACK_PATH    "./data/unlock/fb.bmp"  /* 按键反馈效果图 */

#define UNLOCK_TEXT_X           400             /* 显示密码的起始x坐标 */
#define UNLOCK_TEXT_Y           53                /* 显示密码的起始y坐标 */

/* 按键类型 */
typedef enum UNLOCK_KEY {
        UNLOCK_0 = 0,
        UNLOCK_1,
        UNLOCK_2,
        UNLOCK_3,
        UNLOCK_4,
        UNLOCK_5,
        UNLOCK_6,
        UNLOCK_7,
        UNLOCK_8,
        UNLOCK_9,
        UNLOCK_CONFIRM,
        UNLOCK_BACKSPACE,
        UNLOCK_NONE,
}UNLOCK_KEY;

// /* ------ 按键坐标范围 ------ */
// /* 确认键 */
// #define UNLOCK_CONFIRM_X_START          90
// #define UNLOCK_CONFIRM_X_END              170
// #define UNLOCK_CONFIRM_Y_START          340
// #define UNLOCK_CONFIRM_Y_END              420

// /* 返回键 "BackSpace" */
// #define UNLOCK_BACKSPACE_X_START       300
// #define UNLOCK_BACKSPACE_X_END           380
// #define UNLOCK_BACKSPACE_Y_START       340
// #define UNLOCK_BACKSPACE_Y_END           420

// /* 0 */
// #define UNLOCK_0_X_START        195
// #define UNLOCK_0_X_END            275
// #define UNLOCK_0_Y_START        340
// #define UNLOCK_0_Y_END            420

// /* 1 */
// #define UNLOCK_1_X_START        90
// #define UNLOCK_1_X_END            170
// #define UNLOCK_1_Y_START        240
// #define UNLOCK_1_Y_END            320

// /* 2 */
// #define UNLOCK_2_X_START        195
// #define UNLOCK_2_X_END            275
// #define UNLOCK_2_Y_START        240
// #define UNLOCK_2_Y_END            320

// /* 3 */
// #define UNLOCK_3_X_START        300
// #define UNLOCK_3_X_END            380
// #define UNLOCK_3_Y_START        240
// #define UNLOCK_3_Y_END            320

// /* 4 */
// #define UNLOCK_4_X_START        90
// #define UNLOCK_4_X_END            170
// #define UNLOCK_4_Y_START        140
// #define UNLOCK_4_Y_END            220

// /* 5 */
// #define UNLOCK_5_X_START        195
// #define UNLOCK_5_X_END            275
// #define UNLOCK_5_Y_START        140
// #define UNLOCK_5_Y_END            220

// /* 6 */
// #define UNLOCK_6_X_START        300
// #define UNLOCK_6_X_END            380
// #define UNLOCK_6_Y_START        140
// #define UNLOCK_6_Y_END            220

// /* 7 */
// #define UNLOCK_7_X_START        90
// #define UNLOCK_7_X_END            170
// #define UNLOCK_7_Y_START        45
// #define UNLOCK_7_Y_END            125

// /* 8 */
// #define UNLOCK_8_X_START        195
// #define UNLOCK_8_X_END            275
// #define UNLOCK_8_Y_START        45
// #define UNLOCK_8_Y_END            125

// /* 9 */
// #define UNLOCK_9_X_START        300
// #define UNLOCK_9_X_END            380
// #define UNLOCK_9_Y_START        45
// #define UNLOCK_9_Y_END            125

/* 密码长度 */
#define UNLOCK_PASSWORD_LEN            6

/**
 * @brief 将按键拼接为对应的XY坐标范围
 *
 * @param key 按键: 0, 1, 2, ..., 9, CONFIRM, BACKSPACE
 * @return 按键坐标范围
*/
#define UNLOCK_GET_KEY_RANGE_X_START(key)       (UNLOCK_##key##_X_START)
#define UNLOCK_GET_KEY_RANGE_X_END(key)           (UNLOCK_##key##_X_END)
#define UNLOCK_GET_KEY_RANGE_Y_START(key)       (UNLOCK_##key##_Y_START)
#define UNLOCK_GET_KEY_RANGE_Y_END(key)            (UNLOCK_##key##_Y_END)

/**
 * @brief  将按键拼接为对应的按键类型
 *
 * @param key 按键: 0, 1, 2, ..., 9, CONFIRM, BACKSPACE
 * @return 按键值
*/
#define UNLOCK_GET_KEY_TYPE(key)                                (UNLOCK_##key)

static char unlock_GetKey(int x, int y);
bool unlock_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo);

#endif // !__APP_UNLOCK_H__