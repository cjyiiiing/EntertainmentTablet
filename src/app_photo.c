/**
 * @file app_photo.c
 * @author cjying
 * @brief 界面1：总览界面：
 *                               (1) 会有四张被压缩的照片显示在画面中
 *                               (2) 左滑或者右滑屏幕可以每次切换两张图图片
 *                               (3) 上滑退出应用到桌面
 *                               (4) 点击任意张图片进入卷轴子界面
 *                界面2：卷轴界面：
 *                              (1) 横向卷轴，一张卷轴包含三张照片。
 *                              (2) 显示在画面中只会有一张图的宽度,拖动卷轴可以移动卷轴可视位置
 *                              (3) 拖动到卷轴边缘后会刷新卷轴中的图片，实现卷轴可以查看所有照片。
*/

#include "app_photo.h"
#include "touch_screen.h"
#include "debug.h"

/* 相册中保存所有图像名称的变量，用于排序 */
// char imgPathList[PHOTO_IMGPATHLISTLEN][PHOTO_PATHLEN];

/* 存储图像节点的链表 */
DCLinkList *photoList;
/* 相册的图片总数 */
int photoImgNum = 0;
/* 保存总览界面当前需要显示的图像的序号 */
static int curPhotoIndexList[PHOTO_OVERVIEW_NUM] = {0, 1, 2, 3};
/* 保存卷轴界面前一图片、当前图片、后一图片的序号 */
static int curScrollIndexList[3];
/* 保存卷轴3张图像的缓冲区 */
static int scrollBuf[LCD_HEIGHT][LCD_WIDTH * 3] = {0};

/* 总览界面PHOTO_OVERVIEW_NUM张图的位置 */
int overview_imgRange[PHOTO_OVERVIEW_NUM][4] = {
        {PHOTO_FRAM_1_X_START, PHOTO_FRAM_1_X_END, PHOTO_FRAM_1_Y_START, PHOTO_FRAM_1_Y_END},
        {PHOTO_FRAM_2_X_START, PHOTO_FRAM_2_X_END, PHOTO_FRAM_2_Y_START, PHOTO_FRAM_2_Y_END},
        {PHOTO_FRAM_3_X_START, PHOTO_FRAM_3_X_END, PHOTO_FRAM_3_Y_START, PHOTO_FRAM_3_Y_END},
        {PHOTO_FRAM_4_X_START, PHOTO_FRAM_4_X_END, PHOTO_FRAM_4_Y_START, PHOTO_FRAM_4_Y_END}
};

/* 总览界面单击的图片位置序号 */
int overview_imgIndex[PHOTO_OVERVIEW_NUM + 1] = {
        OVERVIEW_FRAM_1,
        OVERVIEW_FRAM_2,
        OVERVIEW_FRAM_3,
        OVERVIEW_FRAM_4,
        OVERVIEW_NONE,
};

/**
 * @brief 相册app界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return true成功，false失败
*/
bool photo_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        /* 每次都从第一页开始 */
        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                curPhotoIndexList[i] = i;
        }

        PHOTO_KEY key = PHOTO_OVERVIEW;
        bool back = false;              /* 是否退回到桌面 */

        photo_GetImgPaths();

        while (1) {
                switch (key) {
                        case PHOTO_OVERVIEW:
                        {
                                key = photo_overview(fd_lcd, fb_base, vinfo);
                                break;
                        }
                        case PHOTO_SCROLL:
                        {
                                key = photo_scroll(fd_lcd, fb_base, vinfo);
                                break;
                        }
                        case PHOTO_BACKTODESKTOP:
                        {
                                back = true;
                                break;
                        }
                        default:
                                break;
                }

                if (back) {
                        break;
                }
        }

        destroyList(&photoList);

        return true;
}

/**
 * @brief 相册app的总览界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return 界面类型
*/
static PHOTO_KEY photo_overview(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        PHOTO_KEY itface = PHOTO_OVERVIEW;      /* 界面类型 */
        OVERVIEW_KEY key = OVERVIEW_NONE;       /* 总览界面的单击结果 */

        /* 把图像放到对应位置（2 * 2的排布） */
        lcd_clean(fb_base, PHOTO_OVERVIEW_BG_COLOR);
        // photo_DrawPhotoFrame(fb_base, PHOTO_FRAME_WIDTH, PHOTO_FRAME_COLOR);
        photo_PutImgs(fd_lcd, fb_base, vinfo);
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

        /* 根据触摸状态响应 */
        while(1) {
                if (touch.response) {
                        continue;
                }
                switch(touch.state) {
                        case STATE_SINGLE_CLICK:        /* 单击，用户要看哪张图片 */
                        {
                                key = photo_GetKey(touch.x_latest, touch.y_latest);
                                if (key == OVERVIEW_NONE) {
                                        touch.response = true;
                                        break;
                                }
                                // if ((curPhotoIndexList[key] > 0) && (curPhotoIndexList[key] < photoImgNum - 1)) {
                                curScrollIndexList[0] = curPhotoIndexList[key] - 1;
                                curScrollIndexList[1] = curPhotoIndexList[key];
                                curScrollIndexList[2] = curPhotoIndexList[key] + 1;
                                if (curScrollIndexList[1] < photoImgNum) {      /* 确保点到了有缩略图的地方 */
                                        itface = PHOTO_SCROLL;
                                }
                                // }
                                touch.response = true;
                                break;
                        }
                        case STATE_SLIDING:             /* 划动，切换缩略图 */
                        {
                                switch (touch.motion) {
                                        case TC_RIGHT:           /* 往右划，实际是往左翻 */
                                        {
                                                if (curPhotoIndexList[0] > PHOTO_OVERVIEW_NUM - 1) {
                                                        /* 要展示的缩略图的序号前移 */
                                                        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                                                                curPhotoIndexList[i] -= PHOTO_OVERVIEW_NUM;
                                                        }

                                                        /* 把图像放到对应位置（2 * 2的排布） */
                                                        lcd_clean(fb_base, PHOTO_OVERVIEW_BG_COLOR);
                                                        // photo_DrawPhotoFrame(fb_base, PHOTO_FRAME_WIDTH, PHOTO_FRAME_COLOR);
                                                        photo_PutImgs(fd_lcd, fb_base, vinfo);
                                                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);
                                                }
                                                touch.response = true;
                                                break;
                                        }
                                        case TC_LEFT:          /* 往左划，实际是往右翻 */
                                        {
                                                if (curPhotoIndexList[0] < photoImgNum - PHOTO_OVERVIEW_NUM) {
                                                        /* 要展示的缩略图的序号后移 */
                                                        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                                                                curPhotoIndexList[i] += PHOTO_OVERVIEW_NUM;
                                                        }

                                                        /* 把图像放到对应位置（2 * 2的排布） */
                                                        lcd_clean(fb_base, PHOTO_OVERVIEW_BG_COLOR);
                                                        // photo_DrawPhotoFrame(fb_base, PHOTO_FRAME_WIDTH, PHOTO_FRAME_COLOR);
                                                        photo_PutImgs(fd_lcd, fb_base, vinfo);
                                                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);
                                                }
                                                touch.response = true;
                                                break;
                                        }
                                        case TC_UP:             /* 退回到桌面 */
                                        {
                                                itface = PHOTO_BACKTODESKTOP;
                                                touch.response = true;
                                                break;
                                        }
                                        default:
                                        {
                                                touch.response = true;
                                                break;
                                        }
                                }
                                break;
                        }
                        default:
                        {
                                touch.response = true;
                                break;
                        }
                }

                // /* 点到了没有缩略图的地方 */
                // if ((itface == PHOTO_SCROLL) && (curScrollIndexList[1] >= photoImgNum)) {
                //         continue;
                // }

                /* 选择了其他界面，退出循环 */
                if (itface != PHOTO_OVERVIEW) {
                        break;
                }
        }

        return itface;
}

/**
 * @brief 相册app的卷轴界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return 界面类型
*/
static PHOTO_KEY photo_scroll(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        PHOTO_KEY itface = PHOTO_SCROLL;        /* 界面类型 */
        int scrollLeft = LCD_WIDTH, scrollRight = LCD_WIDTH * 2;        /* 要显示的卷轴的左右边界 */
        bool pressFlag = false;         /* 用于只取第一次PRESS的x_latest作为x_last */

        /* 把前一图片、当前图片、后一图片读入到卷轴缓冲区中 */
        photo_DrawScrollBuf(0);
        photo_DrawScrollBuf(1);
        photo_DrawScrollBuf(2);

        /* 显示卷轴当前部分的内容 */
        photo_showCurScroll(LCD_DISPLAY(fb_base, ++view_block), scrollLeft, scrollRight);
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

        int x_last = 0;    /* 上一次触摸的位置 */
        int x_delta = 0;

        while (1) {
                if (touch.response) {
                        continue;
                }

                switch (touch.state) {
                        case STATE_PRESSED:
                        {
                                /* 只取第一个PRESS时的值作为初始坐标 */
                                if (!pressFlag) {
                                        x_last = touch.x_latest;
                                        pressFlag = true;
#ifdef PHOTO_PRINTF
                                        printf("x_last: %d\n", x_last);
#endif
                                }
                                break;
                        }
                        case STATE_DRAGING:     /* 拖动，移动卷轴 */
                        {
                                int x_latest = touch.x_latest;  /* 用保存下来的值更新x_last，而不是用touch.x_latest来更新 */
                                x_delta = x_latest - x_last;
                                scrollLeft -= x_delta;
#ifdef PHOTO_PRINTF
                                printf("touch.x_latest: %d\tx_last: %d\n", touch.x_latest, x_last);
                                printf("x_delta: %d\n", x_delta);
#endif
                                /* 左边没有图片了(卷轴的第一部分没有图片)
                                     防止卷轴界面的第一张图还能往左划 */
                                if ((curScrollIndexList[0] < 0) && (scrollLeft <= LCD_WIDTH)) {
                                        scrollLeft = LCD_WIDTH;
                                }
                                else if (scrollLeft < PHOTO_SCROLL_LEFT_BOUNDARY) {
                                        if (curScrollIndexList[0] <= 0) {       /* 左边没有图片了(卷轴的第一部分有图片) */
                                                scrollLeft = PHOTO_SCROLL_LEFT_BOUNDARY;
                                        }
                                        else {          /* 左边还有图片，刷新卷轴 */
                                                curScrollIndexList[0]--;
                                                curScrollIndexList[1]--;
                                                curScrollIndexList[2]--;
                                                photo_DrawScrollBuf(0);
                                                photo_DrawScrollBuf(1);
                                                photo_DrawScrollBuf(2);
                                                scrollLeft = LCD_WIDTH;
                                        }
                                }
                                /* 右边没有图片了(卷轴的第三部分没有图片)
                                     防止卷轴界面的最后一张图还能往右划*/
                                else if ((curScrollIndexList[2] > photoImgNum - 1) && (scrollLeft >= LCD_WIDTH)) {
                                        scrollLeft = LCD_WIDTH;
                                }
                                else if (scrollLeft >= PHOTO_SCROLL_RIGHT_BOUNDARY) {
                                        if (curScrollIndexList[2] >= photoImgNum - 1) {       /* 左边没有图片了 */
                                                scrollLeft = PHOTO_SCROLL_RIGHT_BOUNDARY;
                                        }
                                        else {          /* 左边还有图片，刷新卷轴 */
                                                curScrollIndexList[0]++;
                                                curScrollIndexList[1]++;
                                                curScrollIndexList[2]++;
                                                photo_DrawScrollBuf(0);
                                                photo_DrawScrollBuf(1);
                                                photo_DrawScrollBuf(2);
                                                scrollLeft = LCD_WIDTH;
                                        }
                                }
                                scrollRight = scrollLeft + LCD_WIDTH;
                                photo_showCurScroll(LCD_DISPLAY(fb_base, ++view_block), scrollLeft, scrollRight);
                                LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);
                                x_last = x_latest;
                                // touch.response = true;
                                pressFlag = false;
                                break;
                        }
                        case STATE_RELEASE:
                        {
                                touch.response = true;
                                // pressFlag = false;
                                break;
                        }
                        case STATE_SLIDING:     /* 划动，判断是否退出相册 */
                        {
                                if (touch.motion == TC_UP) {
                                        itface = PHOTO_OVERVIEW;
                                }
                                touch.response = true;
                                break;
                        }
                        default:
                        {
                                touch.response = true;
                                break;
                        }
                }

                /* 选择了其他界面，退出循环 */
                if (itface != PHOTO_SCROLL) {
                        break;
                }
        }
        return itface;
}

/**
 * @brief 比较函数
*/
static int cmp(const void *a, const void *b) {
        return strcmp(*(char **)a, *(char **)b);
}

/**
 * @brief 获取相册中所有文件的路径
 *
 * @param void
 * @return true表示成功，false表示失败
*/
static bool photo_GetImgPaths(void) {
        DIR *dir;               /* 不透明的目录流结构体类型，表示打开的目录 */
        dir = opendir(PHOTO_PATH);
        struct dirent *ent;     /* 表示目录中的一个文件或者子目录 */
        struct stat pathStat;  /* 用于存储文件信息 */
        char imgPath[PHOTO_PATHLEN];    /* 存储图像路径 */
        datatype data;          /* 节点数据 */
        photoList = NULL;        /* 链表头节点 */

        int cnt = 0;
        while ((ent = readdir(dir)) != NULL) {
#ifdef PHOTO_PRINTF
                printf("%s\t%s\n", ent->d_name, ent->d_type);
#endif
                // 一直被识别为DT_UNKNOWN
                // if (ent->d_type == DT_REG) {     /* 是常规文件 */
                //         strcpy(imgPathList[cnt++], ent->d_name);
                // }

                sprintf(imgPath, "%s%s", PHOTO_PATH, ent->d_name);
                stat(imgPath, &pathStat);
                if (S_ISREG(pathStat.st_mode)) {
                        int i = 0;
                        while (ent->d_name[i] != '\0') {
                                if (ent->d_name[i] == '.') {
                                        if (strcmp(&ent->d_name[i], ".bmp") == 0) {
                                                data.num = cnt;
                                                strcpy(data.bmpName, ent->d_name);
                                                DCLinkList *node = CreateNode(data);
                                                InsertAtTail(&photoList, node);
                                                ++cnt;
                                                // strcpy(imgPathList[cnt++], ent->d_name);
                                        }
                                }
                                ++i;
                        }


                }
        }

        // for (int i = 0; i < cnt; i++) {
        //         printf("%s\n", imgPathList[i]);
        // }
        // qsort(imgPathList, cnt, sizeof(char *), cmp);  // TODO
        // printf("---------------------------------------");
        // for (int i = 0; i < cnt; i++) {
        //         printf("%s\n", imgPathList[i]);
        // }

        photoImgNum = cnt;

        closedir(dir);
}

/**
 * @brief 绘制相框
 *
 * @param
*/
static void photo_DrawPhotoFrame(int *fb_base, int lw, int color) {
        ++view_block;

        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                lcd_hollowRectangle(LCD_DISPLAY(fb_base, view_block), overview_imgRange[i][0], overview_imgRange[i][2],
                                                             overview_imgRange[i][1] - overview_imgRange[i][0],
                                                             overview_imgRange[i][3] - overview_imgRange[i][2],
                                                             lw, color);
        }
}

/**
 * @brief 把要显示的图像放到相框
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return void
*/
static void photo_PutImgs(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        DCLinkList *tmpNode = NULL;
        datatype tmpData;
        char path[50];

        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                /* 从 curPhotoIndexList 中读取当前要显示的图像的序号 */
                tmpData.num = curPhotoIndexList[i];
                /* 寻找序号对应的图像的节点 */
                tmpNode = searchNode(photoList, tmpData, cmpNode);
                if (tmpNode != NULL) {
                        /* 拼接路径 */
                        sprintf(path, "%s/%s", PHOTO_PATH, tmpNode->data->bmpName);
                        /* 显示缩略图 */
                        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), path,
                                                        overview_imgRange[i][0], overview_imgRange[i][2], NULL, NULL, false, PHOTO_OVERVIEW_NUM);
                        /* 显示相框 */
                        lcd_hollowRectangle(LCD_DISPLAY(fb_base, view_block), overview_imgRange[i][0], overview_imgRange[i][2],
                                                             overview_imgRange[i][1] - overview_imgRange[i][0],
                                                             overview_imgRange[i][3] - overview_imgRange[i][2],
                                                             PHOTO_FRAME_WIDTH, PHOTO_FRAME_COLOR);
                }
        }
}

/**
 * @brief 比较节点是否相等
 *
 * @param listNode 要比较的链表的节点
 * @param data 要找的节点
*/
bool cmpNode(datatype listNode, datatype data) {
        if (listNode.num == data.num) {
                return true;
        }
        return false;
}

/**
 * @brief 在卷轴界面判断点击了哪张图片
 *
 * @param x 输入x坐标
 * @param y 输入y坐标
 * @return 点击的图片位置序号
*/
static char photo_GetKey(int x, int y) {
        for (int i = 0; i < PHOTO_OVERVIEW_NUM; ++i) {
                if ((x >= overview_imgRange[i][0]) && (x <= overview_imgRange[i][1])
                      && (y >= overview_imgRange[i][2]) && (y <= overview_imgRange[i][3])) {
                        return overview_imgIndex[i];
                }
        }
        return overview_imgIndex[PHOTO_OVERVIEW_NUM];
}

/**
 * @brief 将bmp图片绘制到卷轴的缓冲区(scrollBuf)上
 *
 * @param  scrollIndex 卷轴的缓冲区的索引，0、1、2
 * @return void
*/
static void photo_DrawScrollBuf(int scrollIndex) {
        DCLinkList *tmpNode = NULL;
        datatype tmpData;
        char path[50];

        /* 从 curScrollIndexList 中读取当前要显示的图像的序号 */
        tmpData.num = curScrollIndexList[scrollIndex];
        /* 寻找序号对应的图像的节点 */
        tmpNode = searchNode(photoList, tmpData, cmpNode);

        /* 找到了对应图像的节点 */
        if (tmpNode != NULL) {
                /* 拼接路径 */
                sprintf(path, "%s/%s", PHOTO_PATH, tmpNode->data->bmpName);

                /* 读取图片 */
                int bmpHeight = 0;
                int *lcdBuf = (int *)malloc(LCD_WIDTH * LCD_HEIGHT * sizeof(int));
                memset(lcdBuf, LCD_BLACK, LCD_WIDTH * LCD_HEIGHT * sizeof(int));       /* 填充为黑色 */
                int ret = lcd_read_bmp(path, lcdBuf, NULL, &bmpHeight);
                if (!ret) {
                        perror("lcd_read_bmp fail");
                }

                /* 存入卷轴的缓冲区 */
                if (bmpHeight > 0) {      /* 倒向的位图 */
                        for (int j = 0; j < LCD_HEIGHT; ++j) {
                                for (int i = LCD_WIDTH * scrollIndex, row = 0; i < LCD_WIDTH * (scrollIndex + 1); ++i, ++row) {
                                        scrollBuf[LCD_HEIGHT - 1 - j][i] = lcdBuf[j * LCD_WIDTH + row];
                                }
                        }
                }
                else {         /* 正向的位图 */
                        for (int j = 0; j < LCD_HEIGHT; ++j) {
                                for (int i = LCD_WIDTH * scrollIndex, row = 0; i < LCD_WIDTH * (scrollIndex + 1); ++i, ++row) {
                                        scrollBuf[j][i] = lcdBuf[j * LCD_WIDTH + row];
                                }
                        }
                }
        }
        /* 没有找到对应图像的节点 */
        else {
                for (int j = 0; j < LCD_HEIGHT; ++j) {
                        for (int i = LCD_WIDTH * scrollIndex, row = 0; i < LCD_WIDTH * (scrollIndex + 1); ++i, ++row) {
                                scrollBuf[j][i] = LCD_BLACK;
                        }
                }
        }
}

/**
 * @brief 显示卷轴当前部分的内容
 *
 * @param fb_base LCD文件映射后的地址
 * @param scrollLeft 卷轴要显示的内容的左边界
 * @param scrollRight 卷轴要显示的内容的右边界
 * @return void
*/
void photo_showCurScroll(int *fb_base, int scrollLeft, int scrollRight) {
        for (int j = 0; j < LCD_HEIGHT; ++j) {
                for (int i = 0; i < LCD_WIDTH; ++i) {
                        fb_base[j * LCD_WIDTH + i] = scrollBuf[j][i + scrollLeft];
                }
        }
}