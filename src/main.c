/*
  * @file: main.c
  * @author: cjying
  * @brief
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "lcd.h"
#include "app_boot.h"
#include "touch_screen.h"
#include "debug.h"
#include "app_desktop.h"
#include "app_unlock.h"
#include "app_draw.h"
#include "app_photo.h"

int main(int argc, char *argv[]) {
        /*------------------------------开启显示屏幕-----------------------------*/
        int fd_lcd;
        int *fb_base = NULL;
        struct fb_var_screeninfo vinfo;
        static struct fb_fix_screeninfo finfo;

        // printf("xres: %d\nyres: %d\nxres_virtual: %d\nyres_virtual: %d\nxoffset: %d\nyoffset: %d\n",
        // vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual, vinfo.xoffset, vinfo.yoffset);

        fd_lcd = open(LCD_PATH, O_RDWR);
        if (fd_lcd == -1) {
                perror("open lcd failed!");
                return -1;
        }
        else {
                fb_base = mmap(NULL, LCD_BUFF_SIZE * LCD_BLOCK, PROT_READ | PROT_WRITE,
                                                    MAP_SHARED, fd_lcd, 0);
                if (fb_base == MAP_FAILED) {
                        perror("mmap failed!");
                        close(fd_lcd);
                        return -1;
                }
                else {
                        lcd_clean(fb_base, LCD_WHITE);
                }

                /* 获取framebuffer可变信息参数，比如屏幕分辨率，像素格式等 */
                if (ioctl(fd_lcd, FBIOGET_VSCREENINFO, &vinfo) != 0) {
                        perror("get vscreeninfo failed!");
                        munmap(fb_base, LCD_BUFF_SIZE * LCD_BLOCK);
                        close(fd_lcd);
                        return -1;
                }
                /* 获取framebuffer固定信息参数 */
                if (ioctl(fd_lcd, FBIOGET_FSCREENINFO, &finfo) != 0) {
                        perror("get fscreeninfo failed!");
                        munmap(fb_base, LCD_BUFF_SIZE * LCD_BLOCK);
                        close(fd_lcd);
                        return -1;
                }
        }

        unsigned int line_width  = vinfo.xres * vinfo.bits_per_pixel / 8;
        unsigned int pixel_width = vinfo.bits_per_pixel / 8;
        int screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
        int buffer_num = finfo.smem_len / screen_size;
        printf("buffer_num = %d\n", buffer_num);

        /* 使能多 buffer */
        vinfo.yres_virtual = LCD_BLOCK * vinfo.yres;
        ioctl(fd_lcd, FBIOPUT_VSCREENINFO, &vinfo);  /* 设置可变参数信息 */

        printf("xres: %d\nyres: %d\nxres_virtual: %d\nyres_virtual: %d\nxoffset: %d\nyoffset: %d\nbits_per_pixel: %d\n",
        vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual, vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel);
        printf("分辨率: %d*%d\n"
                        "像素深度 bpp: %d\n"
                        "一行的字节数: %d\n"
                        "像素格式: R<%d %d> G<%d %d> B<%d %d>\n",
                        vinfo.xres, vinfo.yres, vinfo.bits_per_pixel,
                        finfo.line_length,
                        vinfo.red.offset, vinfo.red.length,
                        vinfo.green.offset, vinfo.green.length,
                        vinfo.blue.offset, vinfo.blue.length);

        lcd_clean(fb_base, LCD_BLACK);
        sleep(1);

        /*------------------------------ 开机动画 -----------------------------*/
// #ifdef TOUCH_DEBUG
        if (!play_boot_animation(fd_lcd, fb_base, &vinfo)) {
                printf("boot animation play error\r\n");
        }
// #endif

        /*------------------------------ 初始化触摸屏状态机 -----------------------------*/
        pthread_t id = InitTouch_pthread();

        INTERFACE_TYPE interface = INTERFACE_UNLOCK;  // TODO
        while (1) {
                switch (interface)
                {
                        /* 解锁界面 */
                        case INTERFACE_UNLOCK:
                        {
                                if (unlock_interface(fd_lcd, fb_base, &vinfo)) {
                                        interface = INTERFACE_DESKTOP;          /* 跳转到桌面界面 */
                                }
                                else {
                                        printf("unlock interface error\n");
                                }
                                break;
                        }
                        /* 桌面 */
                        case INTERFACE_DESKTOP:
                        {
                                interface = desktop_interface(fd_lcd, fb_base, &vinfo);
                                break;
                        }
                        /* 绘画 */
                        case INTERFACE_DRAW:
                        {
                                if (draw_interface(fd_lcd, fb_base, &vinfo)) {
                                        interface = INTERFACE_DESKTOP;
                                }
                                else {
                                        printf("draw interface error\n");
                                }
                                break;
                        }
                        /* 相册 */
                        case INTERFACE_PHOTO:
                        {
                                if (photo_interface(fd_lcd, fb_base, &vinfo)) {
                                        interface = INTERFACE_DESKTOP;
                                }
                                else {
                                        printf("draw interface error\n");
                                }
                                break;
                        }
                        default:
                                break;
                }
        }


        munmap(fb_base, LCD_BUFF_SIZE * LCD_BLOCK);
        close(fd_lcd);
        close(fd_ts);

        return 0;
}