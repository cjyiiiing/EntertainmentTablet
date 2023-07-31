#include "app_desktop.h"

/* app范围: x_start, x_end, y_start, y_end */
static int appRange[APP_TYPE_NUM][4] = {
        {45,      135,    30,    145},            /* pokemon */
        {195,    285,    30,    145},            /* 相册 */
        {345,    435,    30,    145},            /* 绘画 */
        {725,    800,    410,  480},            /* 退出键 */
};

/* app类型 */
static char appType[APP_TYPE_NUM + 1] = {
        INTERFACE_POKEMON,           /* pokemon */
        INTERFACE_PHOTO,                  /* 相册 */
        INTERFACE_DRAW,                     /* 绘画 */
        INTERFACE_UNLOCK,               /* 解锁界面 */
        INTERFACE_NONE,                     /* 无 */
};

/**
 * @brief 桌面界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return INTERFACE_TYPE 类型
*/
INTERFACE_TYPE desktop_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo) {
        INTERFACE_TYPE interfaceType = INTERFACE_DESKTOP;       /* 选择的界面 */

        /* 显示桌面界面 */
        lcd_show_bmp(LCD_DISPLAY(fb_base, ++view_block), DESKTOP_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

        while (1) {
                if (touch.response) {
                        continue;
                }

                switch (touch.state) {
                        case STATE_SINGLE_CLICK:
                        {
                                interfaceType = desktop_GetKey(touch.x_latest, touch.y_latest);
                                touch.response = true;
                                break;
                        }
                        default:
                        {
                                touch.response = true;
                                break;
                        }
                }

                if ((interfaceType != INTERFACE_NONE) && (interfaceType != INTERFACE_DESKTOP)) {
                        break;
                }
        }

        view_block = 0;

        return interfaceType;
}

/**
 * @brief 获取app类型
 *
 * @param x 输入x坐标
 * @param y 输入y坐标
 * @return app类型
*/
static char desktop_GetKey(int x, int y) {
        for (int i = 0; i < APP_TYPE_NUM; ++i) {
                if ((x >= appRange[i][0]) && (x <= appRange[i][1]) && (y >= appRange[i][2]) && (y <= appRange[i][3])) {
                        return appType[i];
                }
        }
        return appType[APP_TYPE_NUM];
}