#include "app_boot.h"
#include "lcd.h"
#include "fonts.h"

/**
 * @brief 开机动画函数
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return true表示成功，false表示失败
 */
bool play_boot_animation(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        /* 字体和颜色参数设置 */
        bool color_add = true;   /* 为真时文字颜色递增，反之递减 */
        static int textColor = BOOT_TEXT_COLOR_MIN;     /* 文字颜色 */

        /* 皮卡丘和进度条参数设置 */
        int pikachu = 1;        /* 第几只皮卡丘 */
        char pikachu_path[50];  /* 皮卡丘路径 */
        int disp_bar_x = 0;     /* 进度条显示的起始x坐标 */
        int bg_width = 0, bg_height = 0;        /* 背景图像的长和宽 */
        int pikachu_width = 0, pikachu_height = 0;      /* 皮卡丘的长和宽 */

        /* 给3个区都画上壁纸 */
        lcd_show_bmp(LCD_DISPLAY(fb_base, 0), BOOT_WALLPAPER_PATH, 0, 0, &bg_width, &bg_height, false, 1);
        lcd_show_bmp(LCD_DISPLAY(fb_base, 1), BOOT_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
        lcd_show_bmp(LCD_DISPLAY(fb_base, 2), BOOT_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);

        /* 显示第一只皮卡丘 */
        sprintf(pikachu_path, "%s%d.bmp", BOOT_ANIMATION_PATH, pikachu);  /* %s%s.bmp 出现段错误 */
        lcd_show_bmp(LCD_DISPLAY(fb_base, ++view_block), pikachu_path, disp_bar_x, BOOT_PROGRESS_BAR_Y,
                                          &pikachu_width, &pikachu_height, false, 1);     /* 如果用++view_block则皮卡丘不从最左边开始跑 */
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

        /* 进度条变化过程 */
        while (disp_bar_x <= bg_width - pikachu_width) {
                disp_bar_x += (int)(rand() % 35 + 1);
                disp_bar_x = (disp_bar_x > bg_width - pikachu_width) ? (bg_width - pikachu_width) : disp_bar_x;
                ++view_block;
                ++pikachu;
                pikachu = (pikachu > BOOT_PIKACHU_NUM) ? 1 : pikachu;

                /* 绘制字符 */
                if (disp_bar_x < bg_width - pikachu_width) {
                        DispString_EN(LCD_DISPLAY(fb_base, view_block), BOOT_TEXT_X, BOOT_TEXT_Y,
                                                        BOOT_TEXT_LOADING, textColor);
                }
                else {
                        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), BOOT_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
                        DispString_EN(LCD_DISPLAY(fb_base, view_block), BOOT_TEXT_X, BOOT_TEXT_Y,
                                                        BOOT_TEXT_FINISH, textColor);
                }

                /* 绘制进度条 */
                lcd_rectangle(LCD_DISPLAY(fb_base, view_block), BOOT_PROGRESS_BAR_X, BOOT_PROGRESS_BAR_Y,
                                                disp_bar_x, pikachu_height, BOOT_PROGRESS_BAR_COLOR);

#ifdef BOOT_DEBUG
                if (view_block % 3 == 0) {
                        lcd_rectangle(LCD_DISPLAY(fb_base, view_block), 200, 100,
                                                100, 100, LCD_RED);
                }
                if (view_block % 3 != 0) {
                        lcd_rectangle(LCD_DISPLAY(fb_base, view_block), 200, 100,
                                                100, 100, LCD_GREEN);
                }
#endif

                /* 绘制皮卡丘 */
                lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), pikachu_path, disp_bar_x,
                                                  BOOT_PROGRESS_BAR_Y, NULL, NULL, false, 1);
                LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

                /* 开机动画结束 */
                if (disp_bar_x == bg_width - pikachu_width) {
                        break;
                }

                /* 读入下一张皮卡丘图像 */
                sprintf(pikachu_path, "%s%d.bmp", BOOT_ANIMATION_PATH, pikachu);

               /* 改变字符颜色 */
                if (color_add) {        /* 颜色递增 */
                        textColor += BOOT_TEXT_COLOR_ADD;
                        if (textColor > BOOT_TEXT_COLOR_MAX) {
                                textColor = BOOT_TEXT_COLOR_MAX;
                                color_add = false;
                        }
                }
                else {                          /* 颜色递减 */
                        textColor -= BOOT_TEXT_COLOR_ADD;
                        if (textColor < BOOT_TEXT_COLOR_MIN) {
                                textColor = BOOT_TEXT_COLOR_MIN;
                                color_add = true;
                        }
                }

                usleep(70000);
        }

        view_block = 0;
        // munmap(fb_base, LCD_BUFF_SIZE * LCD_BLOCK);
        // close(fd_lcd);
        usleep(800000);

        return true;
}
