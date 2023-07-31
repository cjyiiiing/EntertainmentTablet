#include "lcd.h"

u_int32_t view_block = 0;
sFONT *font = &Font32x48;    /* 开机动画中的字体 */


/**
 * @brief 清屏所有
 *
 * @param fb_base 显存映射的基地址
 * @param color      用于清屏的颜色
 */
void lcd_clean(int *fb_base, unsigned int color) {
        for (int i = 0; i < LCD_SIZE * LCD_BLOCK; ++i) {
                *(fb_base + i) = color;
        }
}

/**
 * @brief 清屏
 *
 * @param fb_base 显存映射的基地址
 * @param color      用于清屏的颜色
 */
void lcd_clean_oneblock(int *fb_base, unsigned int color) {
        for (int i = 0; i < LCD_SIZE; ++i) {
                *(fb_base + i) = color;
        }
}

/**
 * @brief 显示bmp图像
 * @attention bmp图形要求宽度必须是4的倍数，否则显示变形
 *
 * @param fb_base       LCD文件映射后的基地址
 * @param bmp_name      bmp的路径名
 * @param x     显示的起始x坐标
 * @param y     显示的起始y坐标
 * @param w     图像的宽度
 * @param h     图形的高度
 * @param return_lcd_buf       是否返回 lcd_buf
 * @param zip 压缩率，比如4表示长宽都缩小为原来的1/4
 * @return lcd_buf 或者 NULL
*/
int *lcd_show_bmp(int32_t *fb_base, const int8_t *bmp_name, int32_t x, int32_t y, int *w, int *h, bool return_lcd_buf, int zip) {
        int fd_bmp = 0;
        bitmap_header bmp_header;       /* bmp文件头 */
        bitmap_info bmp_info;   /* 位图信息头 */
        u_int32_t width, height;        /* 图像的宽和高 */
        int ret = 0;

        /* 打开图像 */
        fd_bmp = open(bmp_name, O_RDONLY);
        if (fd_bmp == -1) {
                perror("open bmp error");
                return false;
        }

        /* 获取BMP文件头(14个字节)，必须要先读文件信息才能读位图信息 */
        read(fd_bmp, &bmp_header, 14);

        /* 检验是否是BMP文件 */
        if (0 != memcmp(bmp_header.type, "BM", 2)) {
                perror("it's not a bmp file");
                close(fd_bmp);
                return false;
        }

        /* 获取位图信息头(40个字节) */
        if (sizeof(bitmap_info) != read(fd_bmp, &bmp_info, 40)) {
                perror("read bitmap_info error");
                close(fd_bmp);
                return false;
        }

        // printf("width: %d\theight:%d\tdepth:%hd\tsize:%d\n",
	// 											bmp_info.width,
	// 											bmp_info.height,
	// 											bmp_info.bit_count,
	// 											bmp_info.size_img);

        /* 获得图像的宽和高 */
        if (bmp_info.width % 4 != 0) {
                perror("bmp width must be multiple of 4");
                close(fd_bmp);
                return false;
        }
        width = bmp_info.width;
        height = (bmp_info.height > 0) ? bmp_info.height : -bmp_info.height;
        if (w != NULL) *w = width;
        if (h != NULL) *h = height;

        /* 读取bmp图像的RGB数据 */
        char bmp_buf[width * height * 3];
        ret = read(fd_bmp, bmp_buf, sizeof(bmp_buf));
        if (ret == -1) {
                perror("read fd_bmp failed");
                close(fd_bmp);
                return false;
        }

        /* 将3通道表示在转换为RGB888的格式 */
        // int lcd_buf[width * height];
        int *lcd_buf = (int *)malloc(width * height * sizeof(int));
        for (int i = 0; i < width * height; ++i) {
                *(lcd_buf + i) = (bmp_buf[3 * i + 2] << 16) | (bmp_buf[3 * i + 1] << 8) | (bmp_buf[3 * i]);
        }

        /* 关闭bmp图像 */
        close(fd_bmp);

        /* 处理超出边界问题 */
        x = (x < 0) ? 0 : x;
        x = (x >= LCD_WIDTH) ? (LCD_WIDTH - 1) : x;
        y = (y < 0) ? 0 : y;
        y = (y >= LCD_HEIGHT) ? (LCD_HEIGHT - 1) : y;
        if (zip > 1) {
                width /= zip;
                height /= zip;
        }
        width = (x + width > LCD_WIDTH) ? (LCD_WIDTH - x) : width;
        height = (y + height > LCD_HEIGHT) ? (LCD_HEIGHT - y) : height;

        /* 将基地址偏移到起始坐标 */
        fb_base = fb_base + (y * LCD_WIDTH + x);

        /* 将数据写入LCD文件 */
        if (bmp_info.height > 0) {      /* 倒向的位图 */
                for (int j = 0; j < height; ++j) {
                        for (int i = 0; i < width; ++i) {
                                fb_base[(height - 1 - j) * LCD_WIDTH + i] = lcd_buf[j * bmp_info.width * zip + i * zip];
                                // printf("%d\t%d\t%d\t%d\r\n", i, j, (height - 1 - j) * LCD_WIDTH + i, j * width * zip + i * zip);
                        }
                }
        }
        else {          /* 正向的位图 */
                for (int j = 0; j < height; ++j) {
                        for (int i = 0; i < width; ++i) {
                                fb_base[j * LCD_WIDTH + i] = lcd_buf[j * bmp_info.width * zip + i * zip];
                        }
                }
        }

        if (return_lcd_buf) {
                return lcd_buf;
        }

        free(lcd_buf);

        return NULL;
}

/**
 * @brief       绘制实心矩形
 *
 * @param fb_base       LCD文件映射后的基地址
 * @param x     起始x坐标
 * @param y     起始y坐标
 * @param w     矩形宽度
 * @param h     矩形高度
 * @param color 矩形颜色
*/
void lcd_rectangle(int *fb_base, int x, int y, int w, int h, int color) {
        /* 参数合法性判断 */
        assert(x >= 0 && x < LCD_WIDTH);
        assert(y >= 0 && y < LCD_HEIGHT);

        /* 确保矩形不会超出边界 */
        w = (x + w > LCD_WIDTH) ? (LCD_WIDTH - x) : w;
        h = (y + h > LCD_HEIGHT) ? (LCD_HEIGHT - y) : h;

        /* 绘制实心矩形 */
        for (int j = y; j < y + h; ++j) {
                for (int i = x; i < x + w; ++i) {
                        fb_base[j * LCD_WIDTH + i] = color;
                }
        }
}

/**
 * @brief       绘制空心矩形
 *
 * @param fb_base       LCD文件映射后的基地址
 * @param x     起始x坐标
 * @param y     起始y坐标
 * @param w     矩形宽度
 * @param h     矩形高度
 * @param lw   线条宽度
 * @param color 矩形颜色
*/
void lcd_hollowRectangle(int *fb_base, int x, int y, int w, int h, int lw, int color) {
        /* 参数合法性判断 */
        assert(x >= 0 && x < LCD_WIDTH);
        assert(y >= 0 && y < LCD_HEIGHT);

        /* 确保矩形不会超出边界 */
        w = (x + w > LCD_WIDTH) ? (LCD_WIDTH - x) : w;
        h = (y + h > LCD_HEIGHT) ? (LCD_HEIGHT - y) : h;

        /* 绘制空心矩形 */
        for (int j = y - lw; j < y; ++j) {
                for (int i = x - lw; i < x + w + lw; ++i) {
                        fb_base[j * LCD_WIDTH + i] = color;
                }
        }
        for (int j = y + h; j < y + h + lw + 1; ++j) {
                for (int i = x - lw; i < x + w + lw; ++i) {
                        fb_base[j * LCD_WIDTH + i] = color;
                }
        }
        for (int j = y; j < y + h; ++j) {
                for (int i = x - lw; i < x; ++i) {
                        fb_base[j * LCD_WIDTH + i] = color;
                }
                for (int i = x + w; i < x + w + lw; ++i) {
                        fb_base[j * LCD_WIDTH + i] = color;
                }
        }
}

/**
 * @brief 显示一个英文字符
 *
 * @param fb_base       显存映射的基地址
 * @param x     起始x坐标
 * @param y     起始y坐标
 * @param ch    要显示的字符
 * @param color 显示的颜色
*/
void DispChar_EN(u_int32_t *fb_base, u_int16_t x, u_int16_t y, const char ch, const int color) {
        u_int16_t fontLength;   /* 每个字模的字节数 */
        u_int8_t *pfont;         /* 字模首地址 */
        u_int32_t lineFeed = 0; /* 当前行已显示的像素数，用于判断是否需要换行 */

        /* 超出边界限制 */
        x = (x  > LCD_WIDTH - font->Width) ? (LCD_WIDTH - font->Width) : x;
        y = (y  > LCD_HEIGHT - font->Height) ? (LCD_HEIGHT - font->Height) : y;

        /* 取出字符对应的字模数据 */
        fontLength = (font->Height * font->Width) / 8;
        pfont = (u_int8_t *)&font->table[fontLength * (ch - ' ')];

        /* 基地址偏移 */
        fb_base = fb_base + (y * LCD_WIDTH + x);

        /* 显示 */
        for (u_int32_t i = 0; i < fontLength; ++i) {
                for (u_int8_t j = 0; j < 8; ++j) {
                        if ((pfont[i] & (0x80 >> j)) != 0) {
                                fb_base[lineFeed] = color;
                        }
                        ++lineFeed;
                }

                /* 判断是否需要换行 */
                if (lineFeed == font->Width) {
                        lineFeed = 0;
                        fb_base = fb_base + LCD_WIDTH;
                }
        }
}

/**
 * @brief       显示字符串
 *
 * @param fb_base       显存映射的基地址
 * @param x     起始x坐标
 * @param y     起始y坐标
 * @param str   要显示的字符串
 * @param color 显示的颜色
*/
void DispString_EN(u_int32_t *fb_base, u_int16_t x, uint16_t y, const char *str, const int color) {
        for (u_int32_t i = 0; str[i] != '\0'; ++i) {
                /* 如果显示到了最右边则换到下一行最左侧继续显示 */
                if (x + font->Width > LCD_WIDTH) {
                        x = 0;
                        y += font->Height;
                }
                /* 如果显示到了最下面则从左上角继续显示 */
                if (y + font->Height > LCD_HEIGHT) {
                        x = 0;
                        y = 0;
                }

                DispChar_EN(fb_base, x, y, str[i], color);
                x += font->Width;
        }
}

/**
 * @brief 绘制实心圆
 *
 * @param fb_base 显存映射基地址
 * @param x 圆心x坐标
 * @param y 圆心y坐标
 * @param radius 半径
 * @param color 颜色
 * @return void
*/
void lcd_circular(u_int32_t *fb_base, u_int16_t x, uint16_t y, uint16_t radius, const int color) {
        /* 参数合法性判断，确保画面上至少会显示部分的圆 */
        assert(x + radius >= 0 && x - radius < LCD_WIDTH);
        assert(y + radius >= 0 && y - radius < LCD_HEIGHT);

        /* 绘制的起始x,y坐标 */
        u_int16_t x_start = (x - radius > 0) ? (x - radius) : 0;
        u_int16_t y_start = (y - radius > 0) ? (y - radius) : 0;
        u_int16_t x_end = (x + radius < LCD_WIDTH) ? (x + radius) : LCD_WIDTH;
        u_int16_t y_end = (y + radius < LCD_HEIGHT) ? (y + radius) : LCD_HEIGHT;

        /* 绘制实心圆 */
        for (int h = y_start; h < y_end; ++h) {
                for (int w = x_start; w < x_end; ++w) {
                        if (((w - x) * (w - x) + (h - y) * (h - y)) <= radius * radius) {
                                fb_base[h * LCD_WIDTH + w] = color;
                        }
                }
        }
}

/**
 * @brief 读入bmp图像保存到buf中
 *
 * @param bmp_name      bmp的路径名
 * @param lcd_buf 保存读入图像的buf
 * @param w 传出参数，读入图像的宽
 * @param h 传出参数，读入图像的高
 * @return true表示成功，false表示失败
 * @note 如果图像大小比显示屏小，则其余部分用黑色填充
*/
bool lcd_read_bmp(const int8_t *bmp_name, int *lcd_buf, int *w, int *h) {
        int fd_bmp = 0;
        bitmap_header bmp_header;       /* bmp文件头 */
        bitmap_info bmp_info;   /* 位图信息头 */
        u_int32_t width, height;        /* 图像的宽和高 */
        int ret = 0;

        /* 打开图像 */
        fd_bmp = open(bmp_name, O_RDONLY);
        if (fd_bmp == -1) {
                perror("open bmp error");
                return false;
        }

        /* 获取BMP文件头(14个字节)，必须要先读文件信息才能读位图信息 */
        read(fd_bmp, &bmp_header, 14);

        /* 检验是否是BMP文件 */
        if (0 != memcmp(bmp_header.type, "BM", 2)) {
                perror("it's not a bmp file");
                close(fd_bmp);
                return false;
        }

        /* 获取位图信息头(40个字节) */
        if (sizeof(bitmap_info) != read(fd_bmp, &bmp_info, 40)) {
                perror("read bitmap_info error");
                close(fd_bmp);
                return false;
        }

        /* 获得图像的宽和高 */
        if (bmp_info.width % 4 != 0) {
                perror("bmp width must be multiple of 4");
                close(fd_bmp);
                return false;
        }
        width = bmp_info.width;
        height = (bmp_info.height > 0) ? bmp_info.height : -bmp_info.height;
        if (w != NULL) *w = bmp_info.width;
        if (h != NULL) *h = bmp_info.height;

        if (bmp_info.height > 0) {
                /* 读取bmp图像的RGB数据 */
                char bmp_buf[width * height * 3];
                ret = read(fd_bmp, bmp_buf, sizeof(bmp_buf));
                if (ret == -1) {
                        perror("read fd_bmp failed");
                        close(fd_bmp);
                        return false;
                }

                /* 将3通道表示在转换为RGB888的格式 */
                for (int j = 0; j < height; ++j) {
                        for (int i = 0; i < width; ++i) {
                                int bmp_buf_index = j * width + i;
                                int lcd_buf_index = (j + (LCD_HEIGHT - height) / 2) * LCD_WIDTH + (i + (LCD_WIDTH - width) / 2);
                                *(lcd_buf + lcd_buf_index) = (bmp_buf[3 * bmp_buf_index + 2] << 16) | (bmp_buf[3 * bmp_buf_index + 1] << 8) | (bmp_buf[3 * bmp_buf_index]);
                        }
                }
        }
        else {
                /* 读取bmp图像的RGB数据 */
                char bmp_buf[width * height * 4];
                ret = read(fd_bmp, bmp_buf, sizeof(bmp_buf));
                if (ret == -1) {
                        perror("read fd_bmp failed");
                        close(fd_bmp);
                        return false;
                }

                /* 将3通道表示在转换为RGB888的格式 */
                for (int j = 0; j < height; ++j) {
                        for (int i = 0; i < width; ++i) {
                                int bmp_buf_index = j * width + i;
                                int lcd_buf_index = (j + (LCD_HEIGHT - height) / 2) * LCD_WIDTH + (i + (LCD_WIDTH - width) / 2);
                                *(lcd_buf + lcd_buf_index) = (bmp_buf[4 * bmp_buf_index + 2] << 16) | (bmp_buf[4 * bmp_buf_index + 1] << 8) | (bmp_buf[4 * bmp_buf_index]);
                        }
                }
        }


        /* 关闭bmp图像 */
        close(fd_bmp);

        return true;
}