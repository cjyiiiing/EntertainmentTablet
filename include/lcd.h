#ifndef __LCD_H__
#define __LCD_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "fonts.h"

#define LCD_PATH "/dev/fb0" /* lcd设备文件路径 */

#define LCD_HEIGHT                                      480
#define LCD_WIDTH                                        800
#define LINE                                                    480
#define ROW                                                     800
#define LCD_SIZE                                          (LCD_HEIGHT * LCD_WIDTH) /* 像素大小 */
#define LCD_BUFF_SIZE                            (LCD_SIZE * 4)     /* 800*480*4 byte */

#define LCD_BLOCK       2                     /* 虚拟显存的块数，用户可以修改选择，取值是1~3 */

/* 颜色 */
#define LCD_RED             0x00FF0000    /* 红色 */
#define LCD_GREEN       0x0000FF00    /* 绿色 */
#define LCD_BLUE           0x000000FF    /* 蓝色 */
#define LCD_WHITE        0x00FFFFFF    /* 白色 */
#define LCD_BLACK        0x00000000    /* 黑色 */
#define LCD_YELLOW     0x00FFFF00    /* 黄色 */
#define LCD_PURPLE     0x00FF00FF    /* 紫色 */
#define LCD_CYAN           0x0000FFFF    /* 青色 */
#define LCD_GRAY           0x00E5CCFF   /* 灰色 */

#pragma pack(push, 2)

/* BMP文件头 14个字节 */
typedef struct bitmap_header
{
	int8_t type[2];                   // 文件类型
	int32_t size;                    // 图像文件大小
	int16_t reserved1;       // 保留字段
	int16_t reserved2;       // 保留字段
	int32_t offbits;               // bmp图像数据偏移量
}bitmap_header;

/* 位图信息头 40个字节 */
typedef struct bitmap_info
{
	int32_t size;                           /* 位图信息头大小 */
	int32_t width;                       /* 图像宽 */
	int32_t height;                     /* 图像高 */
	int16_t planes;                     /* 色彩平面数，该值总被设置为 1 */

	int16_t bit_count;              /* 色深 */
	int32_t compression;        /* 图像数据的压缩类型 */
	int32_t size_img;                /* bmp数据大小，必须是4的整数倍 */
	int32_t X_pel;                       /* 水平分辨率, 像素/米 */
	int32_t Y_pel;                       /* 垂直分辨率, 像素/米 */
	int32_t clrused;                   /* 说明位图实际使用的彩色表中的颜色索引数 */
	int32_t clrImportant;        /* 说明对图像显示有重要影响的颜色索引的数目 */
}bitmap_info;

typedef struct {
        bitmap_header bmp_header;
        bitmap_info bmp_info;
        uint8_t data[];
} bmp_image_t;

#pragma pack(pop)

extern u_int32_t view_block;
extern sFONT *font;

/**
 * @brief 将显存映射的基地址转化为A区、B区、C区
 *
 * @param fb_base LCD文件映射后的基地址
 * @param n n%30，1，2分别对应A区、B区、C区
*/
#define LCD_DISPLAY(fb_base, n)      ((fb_base) + LCD_SIZE * ((n) % LCD_BLOCK))

/**
 * @brief 设置可见区
 *
 * @param lcd   LCD文件描述符
 * @param vinfo fb_var_screeninfo类型的指针
 * @param n     n%3为0，1，2对应A区、B区、C区
*/
#define LCD_SET_DISPLAY(lcd, vinfo, n)    (vinfo)->xoffset = 0;\
                                                                                        (vinfo)->yoffset = LCD_HEIGHT * ((n) % LCD_BLOCK);\
                                                                                        ioctl((lcd), FBIOPAN_DISPLAY, (vinfo));
// #define LCD_SET_DISPLAY(lcd, vinfo, n)  do{
//                                                                                                 (vinfo)->xoffset = 0;
//                                                                                                 (vinfo)->yoffset = LCD_HEIGHT * ((n) % LCD_BLOCK);
//                                                                                                 ioctl((lcd), FBIOPAN_DISPLAY, (vinfo));
//                                                                                         }while(0)    /* 这种写法是错误的 */

void lcd_clean(int *fb_base, unsigned int color);
void lcd_clean_oneblock(int *fb_base, unsigned int color);
int *lcd_show_bmp(int32_t *fb_base, const int8_t *bmp_name, int32_t x, int32_t y, int *w, int *h, bool return_lcd_buf, int zip);
void lcd_hollowRectangle(int *fb_base, int x, int y, int w, int h, int lw, int color);
void lcd_rectangle(int *fb_base, int x, int y, int w, int h, int color);
void DispChar_EN(u_int32_t *fb_base, u_int16_t x, u_int16_t y, const char ch, const int color);
void DispString_EN(u_int32_t *fb_base, u_int16_t x, uint16_t y, const char *str, const int color);
void lcd_circular(u_int32_t *fb_base, u_int16_t x, uint16_t y, uint16_t radius, const int color);
bool lcd_read_bmp(const int8_t *bmp_name, int *lcd_buf, int *w, int *h);

#endif // !__LCD_H__
