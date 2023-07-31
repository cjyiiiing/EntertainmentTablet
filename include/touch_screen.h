#ifndef __TOUCH_SCREEN_H__
#define __TOUCH_SCREEN_H__

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


#define TOUCH_PATH  "/dev/input/event1"

#define TOUCH_DRAGING_INTERVAL           0.2     /* 时间间隔大于该值则判断为拖动 */
#define TOUCH_DRAGING_DIST                      30      /* 移动距离大于该值则判断为向某个方向的滑动 */
#define TOUCH_SINGLE_CLICK_DIST           10      /* 移动距离小于该值则判断为单击 */

/* 划屏动作 */
typedef enum {
        TC_NONE,
        TC_LEFT,
        TC_RIGHT,
        TC_UP,
        TC_DOWN
}TOUCH_SLID;

/* 状态机 */
typedef enum {
	STATE_NOT_PRESSED,			/* 无操作 */
	STATE_PRESSED,					/* 按下 */
	STATE_RELEASE,					/* 释放 */
	STATE_SIN_OR_DOU,			  /* 判断单击还是双击 */
	STATE_SINGLE_CLICK,			  /* 单击 */
	STATE_DOUBLE_CLICK,			/* 双击 */
	STATE_SLIDING,				          /* 滑屏 */
	STATE_DRAGING					/* 拖动 */
}TOUCH_STATE;

/* 触摸屏数据结构体 */
typedef struct TOUCH {
        bool response;                          /* 是否被响应 */
        int x_pre, y_pre;                       /* 上一轮的坐标，用来判断双击 */
        int x_latest, y_latest;              /* 最新一次的坐标 */
        int x, y;                                         /* 第一次的坐标 */
        float time_pre;                         /* 上一轮的事件 */
        float time_latest;                    /* 最新一次的时间 */
        float time;                                  /* 第一次的时间 */
        TOUCH_SLID motion;           /* 划屏动作 */
        TOUCH_STATE state;             /* 状态机的状态 */
}TOUCH;

extern TOUCH touch;
extern int fd_ts;

pthread_t InitTouch_pthread(void);
void Init_tcState(void);
void Renew_tcState(void);
void *Touch_stateMachine(void *arg);


#endif // !__TOUCH_SCREEN_H__