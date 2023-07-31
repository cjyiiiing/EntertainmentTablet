#include "app_draw.h"
#include "lcd.h"
#include "touch_screen.h"

/* 调色盘图像的大小 */
int draw_palette_w = 0, draw_palette_h = 0;

/* 调色盘颜色 */
int *paletteBuf = NULL;

/* 画笔颜色和大小 */
int brushColor = LCD_BLACK;
int brushSize = DRAW_BRUSH_S_R;

/**
 * @brief 绘画app界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return true成功，false失败
*/
bool draw_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        /* 显示画板 */
        draw_ShowBoard(fd_lcd, fb_base, vinfo);

        DRAW_KEY key = DRAW_NONE;
        bool back = false;              /* 是否退回到桌面 */

        while (1) {
                if (touch.response) {
                        continue;
                }

                switch (touch.state) {
                        case STATE_SINGLE_CLICK:
                        {
                                int color = LCD_BLACK;
                                key = draw_GetKey(touch.x_latest, touch.y_latest);

                                if (key == DRAW_BACK) {         /* 退回到桌面 */
                                        back = true;
                                }

                                /* 已响应 */
                                touch.response = true;
                                break;
                        }
                        case STATE_PRESSED:
                        case STATE_DRAGING:
                        {
                                /* 画笔选择那块区域不允许绘制 */
                                if ((touch.y_latest < DRAW_ERASER_Y_END)) {
                                        continue;
                                }
                                lcd_circular(LCD_DISPLAY(fb_base, view_block), touch.x_latest, touch.y_latest, brushSize, brushColor);
                                break;
                        }
                        default:
                        {
                                /* 已响应 */
                                touch.response = true;
                                break;
                        }
                }

                if (back) {
                        if (draw_IfSaveImg(fd_lcd, fb_base, vinfo)) {   /* 如果需要保存绘图 */
                                /* 用当前时间作为保存图像的文件名 */
                                time_t t = time(NULL);          /* 获取当前系统时间 */
                                struct tm* curTime = localtime(&t);     /* 将时间转换为 struct tm 结构体类型，以便获取日期和时间信息 */
                                char imgName[64];       /* 保存图像用的文件名 */
                                sprintf(imgName, "%s%04d-%02d-%02d_%02d-%02d-%02d.bmp", PHOTO_PATH,
                                                curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
                                                curTime->tm_hour, curTime->tm_min, curTime->tm_sec);
                                // draw_SaveBmp("./data/photo/a.bmp", LCD_DISPLAY(fb_base, view_block));
                                printf("%s\n", imgName);
                                draw_SaveBmp(imgName, LCD_DISPLAY(fb_base, view_block));
                        }
                        break;
                }
        }

        return true;
}

/**
 * @brief 显示画板
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return void
*/
void draw_ShowBoard(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        /* 显示背景板 */
        lcd_show_bmp(LCD_DISPLAY(fb_base, ++view_block), DRAW_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);

        /* 显示画笔大小选择 */
        lcd_circular(LCD_DISPLAY(fb_base, view_block), DRAW_BRUSH_S_X, DRAW_BRUSH_S_Y, DRAW_BRUSH_S_R, LCD_BLACK);
        lcd_circular(LCD_DISPLAY(fb_base, view_block), DRAW_BRUSH_M_X, DRAW_BRUSH_M_Y, DRAW_BRUSH_M_R, LCD_BLACK);
        lcd_circular(LCD_DISPLAY(fb_base, view_block), DRAW_BRUSH_L_X, DRAW_BRUSH_L_Y, DRAW_BRUSH_L_R, LCD_BLACK);

        /* 显示画笔颜色选择 */
        paletteBuf = lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), DRAW_PALETTE_PATH, DRAW_PALETTE_X, DRAW_PALETTE_Y, &draw_palette_w, &draw_palette_h, true, 1);

        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);
}

/**
 * @brief 获取按键情况
 *
 * @param x     输入x坐标
 * @param y     输入y坐标
 * @param color 传出参数，如果选择的是画笔则有该值
 * @return DRAW_KEY 类型
*/
DRAW_KEY draw_GetKey(int x, int y) {
        /* 调色盘 */
        if ((x >= DRAW_PALETTE_X) && (x <= DRAW_PALETTE_X + draw_palette_w)
              && (y >= DRAW_PALETTE_Y) && (y <= DRAW_PALETTE_Y + draw_palette_h)) {
                brushColor = paletteBuf[(y - DRAW_PALETTE_Y) * draw_palette_w + x - DRAW_PALETTE_X];
                return DRAW_PALETTE;
        }
        /* 选择S号画笔 */
        else if ((x >= DRAW_BRUSH_S_X - DRAW_BRUSH_S_R) && (x <= DRAW_BRUSH_S_X + DRAW_BRUSH_S_R)
              && (y >= DRAW_BRUSH_S_Y - DRAW_BRUSH_S_R) && (y <= DRAW_BRUSH_S_Y + DRAW_BRUSH_S_R)) {
                brushSize = DRAW_BRUSH_S_R;
                return DRAW_BRUSH_S;
        }
        /* 选择M号画笔 */
        else if ((x >= DRAW_BRUSH_M_X - DRAW_BRUSH_M_R) && (x <= DRAW_BRUSH_M_X + DRAW_BRUSH_M_R)
              && (y >= DRAW_BRUSH_M_Y - DRAW_BRUSH_M_R) && (y <= DRAW_BRUSH_M_Y + DRAW_BRUSH_M_R)) {
                brushSize = DRAW_BRUSH_M_R;
                return DRAW_BRUSH_M;
        }
        /* 选择L号画笔 */
        else if ((x >= DRAW_BRUSH_L_X - DRAW_BRUSH_L_R) && (x <= DRAW_BRUSH_L_X + DRAW_BRUSH_L_R)
              && (y >= DRAW_BRUSH_L_Y - DRAW_BRUSH_L_R) && (y <= DRAW_BRUSH_L_Y + DRAW_BRUSH_L_R)) {
                brushSize = DRAW_BRUSH_L_R;
                return DRAW_BRUSH_L;
        }
        /* 选择橡皮擦 */
        else if ((x >= DRAW_ERASER_X_START) && (x <= DRAW_ERASER_X_END)
              && (y >= DRAW_ERASER_Y_START) && (y <= DRAW_ERASER_Y_END)) {
                brushColor = LCD_WHITE;
                return DRAW_ERASER;
        }
        /* 选择退出 */
        else if ((x >= DRAW_BACK_X_START) && (x <= DRAW_BACK_X_END)
              && (y >= DRAW_BACK_Y_START) && (y <= DRAW_BACK_Y_END)) {
                return DRAW_BACK;
        }

        return DRAW_NONE;
}


/**
 * @brief 保存为bmp图像
 *
 * @param fb_base       显存映射的基地址
 * @param filename      要保存成的文件名
 * @return void
*/
static void draw_SaveBmp(const char *filename, int *fb_base) {
        int image_size = DRAW_W * DRAW_H * 4;
        bmp_image_t* image = malloc(sizeof(bitmap_header) + sizeof(bitmap_info) + image_size);

        if (image) {
                // 设置 bmp 文件头部信息
                image->bmp_header.type[0] = 0x42;       /* B */
                image->bmp_header.type[1] = 0x4D;       /* M */
                image->bmp_header.size = sizeof(bitmap_header) + sizeof(bitmap_info) + image_size;
                image->bmp_header.reserved1 = 0;
                image->bmp_header.reserved2 = 0;
                image->bmp_header.offbits = sizeof(bitmap_header) + sizeof(bitmap_info);

                // 设置 bmp 图像信息头部信息
                image->bmp_info.size = sizeof(bitmap_info);
                image->bmp_info.width = DRAW_W;
                image->bmp_info.height = -DRAW_H;  /* 注意：这里使用负号，表示是正向位图 */
                image->bmp_info.planes = 1;
                image->bmp_info.bit_count = 32;  // 一个像素点需要多少个 bit 数据来描述
                image->bmp_info.compression = 0;
                image->bmp_info.size_img = image_size;
                image->bmp_info.X_pel = 0;
                image->bmp_info.Y_pel = 0;
                image->bmp_info.clrused = 0;
                image->bmp_info.clrImportant = 0;

                // 将图像数据拷贝到结构体缓冲区中
                // int *imgBuf = (int *)malloc(DRAW_H * DRAW_W * 4);
                // for (int i = DRAW_H * DRAW_W; i > 0; --i) {
                //         imgBuf[DRAW_H * DRAW_W - i] = fb_base[i - 1];
                // }
                // memcpy(image->data, imgBuf, image_size);
                memcpy(image->data, fb_base, image_size);
        }

        FILE* fp = fopen(filename, "wb");

        if (fp) {
                fwrite(image, sizeof(bitmap_header) + sizeof(bitmap_info) + image_size, 1, fp);
                fclose(fp);
        }
}

/**
 * @brief 询问是否保存绘制的图像
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return true表示要保存，false表示不要保存
*/
bool draw_IfSaveImg(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        bool back = false;
        bool save = false;

        /* 如果不加这行，状态机的状态可能仍保持在STATE_SINGLE_CLICK而未重置，
             导致后面代码执行时跳过STATE_PRESSED直接到STATE_SINGLE_CLICK */
        Renew_tcState();

        /* 先清屏，否则可能背景是桌面 */
        lcd_clean_oneblock(LCD_DISPLAY(fb_base, view_block + 1), LCD_WHITE);
        /* 显示"SAVE?" */
        DispString_EN(LCD_DISPLAY(fb_base, view_block + 1), DRAW_SAVE_X,
                                        DRAW_SAVE_Y, DRAW_SAVE_STR, LCD_RED);
        /* 显示"yes" */
        DispString_EN(LCD_DISPLAY(fb_base, view_block + 1), DRAW_YES_X,
                                        DRAW_YES_Y, DRAW_YES_STR, LCD_RED);
        /* 显示"no" */
        DispString_EN(LCD_DISPLAY(fb_base, view_block + 1), DRAW_NO_X,
                                        DRAW_NO_Y, DRAW_NO_STR, LCD_RED);
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block + 1);

        while (1) {
                switch (touch.state) {
                        case STATE_SINGLE_CLICK:
                        {
                                if ((touch.x_latest >= DRAW_YES_X) && (touch.x_latest <= DRAW_YES_X + 32 * 3) &&
                                (touch.y_latest >= DRAW_YES_Y) && (touch.y_latest <= DRAW_YES_Y + 48)) {
                                        save = true;
                                        back = true;
                                }
                                else if ((touch.x_latest >= DRAW_NO_X) && (touch.x_latest <= DRAW_NO_X + 32 * 2) &&
                                (touch.y_latest >= DRAW_NO_Y) && (touch.y_latest <= DRAW_NO_Y + 48)) {
                                        save = false;
                                        back = true;
                                }
                                touch.response = true;
                                break;
                        }
                        case STATE_PRESSED:
                        {
                                if ((touch.x_latest >= DRAW_YES_X) && (touch.x_latest <= DRAW_YES_X + 32 * 3) &&
                                (touch.y_latest >= DRAW_YES_Y) && (touch.y_latest <= DRAW_YES_Y + 48)) {
                                        lcd_rectangle(LCD_DISPLAY(fb_base, view_block + 1), DRAW_YES_X, DRAW_YES_Y,
                                                                      32 * 3, 48, LCD_RED);
                                        DispString_EN(LCD_DISPLAY(fb_base, view_block + 1), DRAW_YES_X,
                                                                        DRAW_YES_Y, DRAW_YES_STR, LCD_WHITE);
                                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block + 1);
                                }
                                else if ((touch.x_latest >= DRAW_NO_X) && (touch.x_latest <= DRAW_NO_X + 32 * 2) &&
                                (touch.y_latest >= DRAW_NO_Y) && (touch.y_latest <= DRAW_NO_Y + 48)) {
                                        lcd_rectangle(LCD_DISPLAY(fb_base, view_block + 1), DRAW_NO_X, DRAW_NO_Y,
                                                                      32 * 2, 48, LCD_RED);
                                        DispString_EN(LCD_DISPLAY(fb_base, view_block + 1), DRAW_NO_X,
                                                                        DRAW_NO_Y, DRAW_NO_STR, LCD_WHITE);
                                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block + 1);
                                }
                                break;
                        }
                        default:
                        {
                                touch.response = true;
                                break;
                        }
                }

                if (back) {
                        break;
                }
        }

        return save;
}