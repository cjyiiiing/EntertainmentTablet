#include "app_unlock.h"

/* 锁屏密码 */
static char pw[7] = "123456";

/* 按键范围: x_start, x_end, y_start, y_end */
static int keyRange[12][4] = {
    {195, 275, 340, 420}, /* 0 */
    {90, 170, 240, 320},  /* 1 */
    {195, 275, 240, 320}, /* 2 */
    {300, 380, 240, 320}, /* 3 */
    {90, 170, 140, 220},  /* 4 */
    {195, 275, 140, 220}, /* 5 */
    {300, 380, 140, 220}, /* 6 */
    {90, 170, 45, 125},   /* 7 */
    {195, 275, 45, 125},  /* 8 */
    {300, 380, 45, 125},  /* 9 */
    {90, 170, 340, 420},  /* 确认键 */
    {300, 380, 340, 420}, /* backspace */
};

/* 按键类型 */
static char keyType[13] = {
    UNLOCK_0,
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
};

/**
 * @brief 解锁界面
 *
 * @param fd_lcd LCD文件描述符
 * @param fb_base LCD文件映射后的地址
 * @param vinfo framebuffer可变信息参数
 * @return true表示解锁成功，false表示解锁失败
 */
bool unlock_interface(int fd_lcd, int *fb_base, struct fb_var_screeninfo *vinfo)
{
        char key;                                   /* 按键值 */
        int pwLen = 0;                              /* 密码长度 */
        char pwBuf[UNLOCK_PASSWORD_LEN + 1] = "";   /* 输入的密码 */
        char dispBuf[UNLOCK_PASSWORD_LEN + 1] = ""; /* 显示的密码 */
        bool pwCorrect = false;                     /* 密码是否正确 */
        int icon_x = 0, icon_y = 0;                 /* 按键反馈图像显示的起始坐标 */
        bool show_icon = true;                      /* 显示按键反馈图像 */

        /* 更新状态机 */
        // Renew_tcState();

        /* 显示背景 */
        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), UNLOCK_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

        /* 根据触摸屏状态机的状态（单击、按下）执行相应的操作 */
        while (1)
        {
                // printf("touch.x_latest: %d\ttouch.y_latest: %d\n", touch.x_latest, touch.y_latest);
                if (touch.response)
                {
                        continue;
                }
                switch (touch.state)
                {
                case STATE_SINGLE_CLICK:
                {
                        printf("STATE_SINGLE_CLICK\n");

                        /* 获取键值 */
                        key = unlock_GetKey(touch.x_latest, touch.y_latest);

                        /* 根据键值生成密码 */
                        switch (key)
                        {
                        case UNLOCK_0:
                        case UNLOCK_1:
                        case UNLOCK_2:
                        case UNLOCK_3:
                        case UNLOCK_4:
                        case UNLOCK_5:
                        case UNLOCK_6:
                        case UNLOCK_7:
                        case UNLOCK_8:
                        case UNLOCK_9:
                        {
                                if (pwLen < UNLOCK_PASSWORD_LEN)
                                {
                                        /* 拼接输入的密码 */
                                        char ch[2] = "";
                                        snprintf(ch, 2, "%d", key);
                                        strcat(pwBuf, ch);
                                        /* 拼接显示的密码 */
                                        strcat(dispBuf, "*");
                                        /* 密码长度增加 */
                                        ++pwLen;
#ifdef UNLOCK_PRINTF
                                        printf("ch: %s\n", ch);
                                        printf("pwBuf: %s\n", pwBuf);
                                        printf("dispBuf: %s\n", dispBuf);
#endif
                                }
                                break;
                        }
                        case UNLOCK_CONFIRM:
                        {
                                /* 输入密码长度等于真实密码 */
                                if (pwLen == UNLOCK_PASSWORD_LEN)
                                {
                                        /* 密码正确 */
                                        if (strcmp(pw, pwBuf) == 0)
                                        {
                                                pwCorrect = true;
                                        }
                                        /*  密码错误，清空输入 */
                                        else
                                        {
                                                strcpy(pwBuf, "");
                                                strcpy(dispBuf, "");
                                                pwLen = 0;
                                        }
                                }
                                break;
                        }
                        case UNLOCK_BACKSPACE:
                        {
                                if (pwLen > 0)
                                {
                                        pwBuf[pwLen - 1] = '\0';
                                        dispBuf[pwLen - 1] = '\0';
                                        --pwLen;
                                }
                                break;
                        }
                        default:
                                break;
                        }

                        /* 刷新图像 */
                        ++view_block;
                        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), UNLOCK_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
                        DispString_EN(LCD_DISPLAY(fb_base, view_block), UNLOCK_TEXT_X, UNLOCK_TEXT_Y, dispBuf, LCD_BLACK);
                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);

                        /* 已响应 */
                        touch.response = true;
                        show_icon = true;

                        break;
                }
                case STATE_PRESSED:
                {
                        if (!show_icon)
                        {
                                continue;
                        }

                        /* 获取键值 */
                        key = unlock_GetKey(touch.x_latest, touch.y_latest);

                        /* 根据键值确定反馈图像显示的位置 */
                        if (key < 12)
                        {
                                icon_x = keyRange[key][0];
                                icon_y = keyRange[key][2];
                                /* 显示按键反馈图像 */
                                if (icon_x != 0 || icon_y != 0)
                                {
                                        ++view_block;
                                        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), UNLOCK_WALLPAPER_PATH, 0, 0, NULL, NULL, false, 1);
                                        lcd_show_bmp(LCD_DISPLAY(fb_base, view_block), UNLOCK_FEEDBACK_PATH, icon_x, icon_y, NULL, NULL, false, 1);
                                        DispString_EN(LCD_DISPLAY(fb_base, view_block), UNLOCK_TEXT_X, UNLOCK_TEXT_Y, dispBuf, LCD_BLACK);
                                        LCD_SET_DISPLAY(fd_lcd, vinfo, view_block);
                                }
                        }

                        show_icon = false;

                        break;
                }
                }

                /* 密码正确，退出循环 */
                if (pwCorrect)
                {
                        break;
                }
        }

        view_block = 0;

        return true;
}

/**
 * @brief 获取按键类型
 *
 * @param x 输入x坐标
 * @param y 输入y坐标
 * @return 按键类型
 */
static char unlock_GetKey(int x, int y)
{
        for (int i = 0; i < 12; ++i)
        {
                if ((x >= keyRange[i][0]) && (x <= keyRange[i][1]) && (y >= keyRange[i][2]) && (y <= keyRange[i][3]))
                {
                        return keyType[i];
                }
        }
        return keyType[12];

        // /* 0 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(0)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(0)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(0)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(0))) {
        //         return UNLOCK_0;
        // }

        // /* 1 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(1)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(1)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(1)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(1))) {
        //         return UNLOCK_1;
        // }

        // /* 2 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(2)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(2)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(2)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(2))) {
        //         return UNLOCK_2;
        // }

        // /* 3 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(3)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(3)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(3)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(3))) {
        //         return UNLOCK_3;
        // }

        // /* 4 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(4)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(4)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(4)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(4))) {
        //         return UNLOCK_4;
        // }

        // /* 5 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(5)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(5)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(5)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(5))) {
        //         return UNLOCK_5;
        // }

        // /* 6 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(6)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(6)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(6)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(6))) {
        //         return UNLOCK_6;
        // }

        // /* 7 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(7)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(7)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(7)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(7))) {
        //         return UNLOCK_7;
        // }

        // /* 8 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(8)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(8)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(8)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(8))) {
        //         return UNLOCK_8;
        // }

        // /* 9 */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(9)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(9)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(9)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(9))) {
        //         return UNLOCK_9;
        // }

        // /* CONFIRM */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(CONFIRM)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(CONFIRM)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(CONFIRM)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(CONFIRM))) {
        //         return UNLOCK_CONFIRM;
        // }

        // /* BACKSPACE */
        // if ((x >= UNLOCK_GET_KEY_RANGE_X_START(BACKSPACE)) && (x <= UNLOCK_GET_KEY_RANGE_X_END(BACKSPACE)) &&
        //       (y >= UNLOCK_GET_KEY_RANGE_Y_START(BACKSPACE)) && (y<= UNLOCK_GET_KEY_RANGE_Y_END(BACKSPACE))) {
        //         return UNLOCK_BACKSPACE;
        // }

        // return UNLOCK_NONE;
}