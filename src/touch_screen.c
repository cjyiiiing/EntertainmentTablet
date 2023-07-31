#include "touch_screen.h"
#include "debug.h"

static u_int32_t time_start = 0;        /*  由于touch中记录的数值太大，与该值相减使数值变小  */
static pthread_t child_id;      /* 用于运行触摸屏状态机的子线程 */
TOUCH touch;
int fd_ts = 0;  /* 触摸屏文件 */

/**
 * @brief 创建线程，用于执行触摸屏状态机
 *
 * @param void
 * @return 线程id
*/
pthread_t InitTouch_pthread(void) {
        int ret = 0;

        time_start = time(NULL);

        /* 创建子线程，用于执行触摸屏状态机 */
        ret = pthread_create(&child_id, NULL, Touch_stateMachine, NULL);
        if (ret < 0) {
                perror("pthread_create error");
                return -1;
        }

        /* 分离线程，线程终止时资源自动释放给系统 */
        ret = pthread_detach(child_id);
        if (ret < 0) {
                perror("pthread_detach error");
                return -1;
        }

        return child_id;
}

/**
 * @brief 初始化触摸屏数据结构体
 *
 * @param void
 * @return void
*/
void Init_tcState(void) {
        touch.response = true;          /* 初始化为已被响应 */
        touch.x_pre = touch.y_pre = 0;
        touch.x_latest = touch.y_latest = 0;
        touch.x = touch.y = 0;
        touch.motion = TC_NONE;
        touch.state = STATE_NOT_PRESSED;
}

/**
 * @brief 重置触摸屏状态机
 *
 * @param void
 * @return void
 * @note 保留上次按下的坐标和时间，用于判断是否是双击
*/
void Renew_tcState(void) {
#ifdef TOUCH_PRINTF
        printf("Renew_tcState\n");
#endif
        touch.response = false;

        touch.x_pre = touch.x_latest;
        touch.y_pre = touch.y_latest;
        touch.time_pre = touch.time_latest;

        touch.x = touch.y = 0;
        touch.x_latest = touch.y_latest = 0;
        touch.time = touch.time_latest = 0;

        touch.motion = TC_NONE;
        touch.state = STATE_NOT_PRESSED;
}

/**
 * @brief 触摸屏状态机
 *
 * @param void
 * @return void
 *
 * @note 在触摸屏的驱动里设置 input_dev 需要上报的事件为 EV_ABS 和 EV_KEY 事件。
 * EV_KEY 是按键事件，用于上报触摸屏是否被按下，相当于把触摸屏当做一个按键。
 * EV_ABS事件(0x3)：code=ABS_X/ABS_Y/ABS_MT_SLOT/ABS_MT_TRACKING_ID/ABS_MT_POSITION_X/ABS_MT_POSITION_Y。
 * EV_KEY事件(0x1)：code=BTN_TOUCH(0x14a)，value=0x1 表示触摸屏被按下，value=0x0表示手指离开触摸屏
 * EV_SYN(0x0)同步事件，EV_KEY(0x1)按键事件，EV_ABS(0x3)绝对坐标事件
 *  include/uapi/linux/input.h
 *
*/
void *Touch_stateMachine(void *arg) {
        struct input_event tc_data;

        /* 打开触摸屏 */
        fd_ts = open(TOUCH_PATH, O_RDONLY);
        if (fd_ts == -1) {
                perror("open touch screen failed");
                // pthread_cancel(fd_ts);          /* 取消线程（执行到一个取消点时才会终止） */
                return NULL;
        }

        /* 初始化触摸屏数据结构体 */
        Init_tcState();

        while (1) {
                memset(&tc_data, 0, sizeof(struct input_event));
                read(fd_ts, &tc_data, sizeof(struct input_event));

                switch(touch.state) {
                        case STATE_NOT_PRESSED:
                        {
#ifdef TOUCH_DEBUG
                                while (1) {
                                        read(fd_ts, &tc_data, sizeof(struct input_event));
                                        printf("time: %ds%dus\ttype: %d\tcode: %d\tvalue: %d\n",
                                                        tc_data.time.tv_sec, tc_data.time.tv_usec,tc_data.type, tc_data.code, tc_data.value);
                                }
#endif
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_NOT_PRESSED ---------- \r\n");
#endif
                                /* 触摸屏按下 */
                                if (tc_data.type == EV_KEY && tc_data.code == BTN_TOUCH && tc_data.value == 1) {
                                        for (int i = 0; i < 2; ++i) {
                                                read(fd_ts, &tc_data, sizeof(struct input_event));

                                                /* 触摸屏每点击一次对应要读两次，先读x坐标再读y坐标 */
                                                if (tc_data.type == EV_ABS && tc_data.code == ABS_X) {
                                                        touch.x = tc_data.value;
                                                        touch.time = (tc_data.time.tv_sec - time_start) * 1.0 + ((tc_data.time.tv_usec / 1000) * 1.0 / 1000);
                                                }
                                                else if (tc_data.type == EV_ABS && tc_data.code == ABS_Y) {
                                                        touch.y = tc_data.value;
                                                        touch.time = (tc_data.time.tv_sec - time_start) * 1.0 + ((tc_data.time.tv_usec / 1000) * 1.0 / 1000);
                                                }

                                                if (touch.x != 0 && touch.y != 0) {
                                                        touch.x_latest = touch.x;
                                                        touch.y_latest = touch.y;
                                                        touch.state = STATE_PRESSED;
                                                        touch.response = false;
#ifdef TOUCH_PRINTF
                                                        printf("STATE_NOT_PRESSED  ->  STATE_PRESSED\r\n");
#endif
                                                }
                                        }
                                }
                                break;
                        }
                        case STATE_PRESSED:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_PRESSED ---------- \r\n");
#endif
                                /* 发生了拖动 */
                                if (tc_data.type == EV_ABS && tc_data.code == ABS_X) {
                                        touch.x_latest = tc_data.value;
                                        touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;

                                        read(fd_ts, &tc_data, sizeof(struct input_event));

                                        if (tc_data.type == EV_ABS && tc_data.code == ABS_Y) {
                                                touch.y_latest = tc_data.value;
                                                touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;
                                        }

                                        // printf("%f\t%f\n", touch.time_latest, touch.time);
                                        // printf("%f\t%f\n", tc_data.time.tv_usec / 1000, tc_data.time.tv_usec / 1000.0);
                                        // printf("%f\t%f\n", tc_data.time.tv_sec * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000,\
                                        tc_data.time.tv_sec * 1.0 + tc_data.time.tv_usec / 1000.0 * 1.0 / 1000);

                                        /* 触屏且时间间隔大于阈值判断为拖动 */
                                        if (touch.x_latest != 0 && touch.y_latest != 0 && touch.time_latest - touch.time > TOUCH_DRAGING_INTERVAL) {
                                                touch.state = STATE_DRAGING;
                                                touch.response = false;
#ifdef TOUCH_PRINTF
                                                printf("STATE_PRESSED  ->  STATE_DRAGING\r\n");
#endif
                                        }
                                }
                                /* 抬起 */
                                else if (tc_data.type == EV_KEY && tc_data.code == BTN_TOUCH && tc_data.value == 0) {
                                        if (touch.x_latest == 0 || touch.y_latest == 0) {
                                                touch.x_latest = touch.x;
                                                touch.y_latest = touch.y;
                                        }
                                        touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;
                                        touch.state = STATE_RELEASE;
#ifdef TOUCH_PRINTF
                                        printf("STATE_PRESSED  ->  STATE_RELEASE\r\n");
#endif
                                }
                                break;
                        }
                        case STATE_RELEASE:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_RELEASE ---------- \r\n");
#endif
                                int deltaX = (touch.x_latest - touch.x > 0) ? (touch.x_latest - touch.x) : (touch.x - touch.x_latest);
                                int deltaY = (touch.y_latest - touch.y > 0) ? (touch.y_latest - touch.y) : (touch.y - touch.y_latest);

                                /* 判断单击 */
                                if ((deltaX < TOUCH_SINGLE_CLICK_DIST) && (deltaY < TOUCH_SINGLE_CLICK_DIST)) {
#ifdef TOUCH_PRINTF
                                        printf("single click\r\n");
#endif
                                        touch.state = STATE_SINGLE_CLICK;
                                        touch.response = false;
                                        break;
                                }

                                /* 判断划屏的方向 */
                                if (deltaX < deltaY) {
                                        if (touch.y_latest - touch.y > TOUCH_DRAGING_DIST) {
#ifdef TOUCH_PRINTF
                                                printf("down\r");
#endif
                                                touch.motion = TC_DOWN;
                                                touch.state = STATE_SLIDING;
                                                touch.response = false;
                                                break;
                                        }
                                        else if (touch.y - touch.y_latest > TOUCH_DRAGING_DIST) {
#ifdef TOUCH_PRINTF
                                                printf("up\r");
#endif
                                                touch.motion = TC_UP;
                                                touch.state = STATE_SLIDING;
                                                touch.response = false;
                                                break;
                                        }
                                }
                                else {
                                        if (touch.x_latest - touch.x > TOUCH_DRAGING_DIST) {
#ifdef TOUCH_PRINTF
                                                printf("right\r");
#endif
                                                touch.motion = TC_RIGHT;
                                                touch.state = STATE_SLIDING;
                                                touch.response = false;
                                                break;
                                        }
                                        else if (touch.x - touch.x_latest > TOUCH_DRAGING_DIST) {
#ifdef TOUCH_PRINTF
                                                printf("left\r");
#endif
                                                touch.motion = TC_LEFT;
                                                touch.state = STATE_SLIDING;
                                                touch.response = false;
                                                break;
                                        }
                                }

                                touch.state = STATE_NOT_PRESSED;        /* 不加这行可能一直保持STATE_RELEASE状态 */

                                break;
                        }
                        case STATE_SIN_OR_DOU:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_SIN_OR_DOU ---------- \r\n");
#endif
                                break;
                        }
                        case STATE_SINGLE_CLICK:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_SINGLE_CLICK ---------- \r\n");
#endif
                                if (touch.response) {
                                        Renew_tcState();
                                }
                                break;
                        }
                        case STATE_DOUBLE_CLICK:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_DOUBLE_CLICK ---------- \r\n");
#endif
                                break;
                        }
                        case STATE_SLIDING:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_SLIDING ---------- \r\n");
#endif
                                if (touch.response) {
                                        Renew_tcState();
                                }
                                break;
                        }
                        case STATE_DRAGING:
                        {
#ifdef TOUCH_PRINTF
                                printf("---------- STATE_DRAGING ---------- \r\n");
#endif
                                /* 发生了拖动 */
                                if (tc_data.type == EV_ABS && tc_data.code == ABS_X) {
                                        touch.x_latest = tc_data.value;
                                        touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;

                                        read(fd_ts, &tc_data, sizeof(struct input_event));

                                        if (tc_data.type == EV_ABS && tc_data.code == ABS_Y) {
                                                touch.y_latest = tc_data.value;
                                                touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;
                                        }
                                }
                                /* 抬起 */
                                else if (tc_data.type == EV_KEY && tc_data.code == BTN_TOUCH && tc_data.value == 0) {
                                        touch.time_latest = (tc_data.time.tv_sec - time_start) * 1.0 + tc_data.time.tv_usec / 1000 * 1.0 / 1000;
                                        touch.state = STATE_RELEASE;
                                        touch.response = false;
#ifdef TOUCH_PRINTF
                                        printf("STATE_DRAGING  ->  STATE_RELEASE\r\n");
#endif
                                }
                                // if (touch.response) {
                                //         Renew_tcState();
                                // }
                                break;
                        }
                        default:
                                break;
                }
        }
}
