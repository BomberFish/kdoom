// bomberfish 2024
// File: i_input_raw.c
// Raw /dev/input touchscreen input for kdoom. Most code taken from FBInk's finger_trace sample.
// TODO: Control labels, dont repeat keydowns

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <dirent.h>
#include <poll.h>
#include <errno.h>
#include <../FBInk/fbink.h>
#include <../FBInk/libevdev/libevdev/libevdev.h>

#include "config.h"
#include "deh_str.h"
#include "doomtype.h"
#include "doomkeys.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_scale.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

int vanilla_keyboard_mapping = 1;

struct pollfd pfd;

// Is the shift key currently down?

static int shiftdown = 0;

FBInkInputDevice *input_devices = NULL;
int dev_cnt = 0;

struct libevdev *dev = NULL;
int evfd = -1;

bool init_failed = false;

typedef struct {
    int x;
    int y;
} Coord;

typedef struct {
    bool down;
    Coord pos;
} TouchEv;

typedef struct {
    int key;
    FBInkRect rect;
} Button;

TouchEv touch_ev;
TouchEv prev_ev;

// Coord touch;
// bool touch_down = false;

int scw = 0;
int sch = 0;

void I_GetScreenSize(int *width, int *height);

Button upKey = {
    .key = KEY_UPARROW,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

Button downKey = {
    .key = KEY_DOWNARROW,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

Button leftKey = {
    .key = KEY_LEFTARROW,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

Button rightKey = {
    .key = KEY_RIGHTARROW,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

Button fireKey = {
    .key = KEY_SPACE,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

Button enterKey = {
    .key = KEY_ENTER,
    .rect = {
        .left = 0,
        .top = 0,
        .width = 0,
        .height = 0,
    },
};

int BTN_SIZE = 100;
int BTN_PAD = 10;

Button *keys[] = {&upKey, &downKey, &leftKey, &rightKey, &fireKey, &enterKey, NULL};

__attribute__ ((weak)) int fbink_fd;
__attribute__ ((weak)) FBInkConfig fbink_cfg;

void CalcKeyPos(void) {
    printf("CalcKeyPos\n");
    for (int i = 0; i < sizeof(keys); i++) {
        if (!keys[i]) {
            break; // It's joever
        }

        keys[i]->rect.width = BTN_SIZE;
        keys[i]->rect.height = BTN_SIZE;

        keys[i]->rect.left = (BTN_SIZE * i) + (BTN_PAD * i);
        keys[i]->rect.top = (sch - BTN_SIZE) + BTN_PAD;
        printf("%d %d %d %d\n", keys[i]->rect.left, keys[i]->rect.top, keys[i]->rect.width, keys[i]->rect.height);
    }
}

void PlaceKeys(void) {
    printf("PlaceKeys\n");
    for (int i = 0; i < sizeof(keys); i++) {
        if (!keys[i]) {
            break; // It's joever
        }

        printf("Placing key %d\n", i);
        FBInkRect rect = keys[i]->rect;
        fbink_fill_rect_rgba(fbink_fd, &fbink_cfg, &rect, NULL, 0x00, 0x00, 0x00, 0xFFu);
    }
}

void I_InitInput(void) {
    // PlaceKeys();
    printf("I_InitInput\n");
    I_GetScreenSize(&scw, &sch);
    BTN_PAD = (scw / 6) / 10;
    BTN_SIZE = (scw / 6) - BTN_PAD;

    CalcKeyPos();
    PlaceKeys();

    input_devices = fbink_input_scan(INPUT_TOUCHSCREEN, 0U, 0U, &dev_cnt);
    printf("Found %d input devices\n", dev_cnt);
    for (int i = 0; i < dev_cnt; i++) {
        printf("Device %d: %s [fd%d]\n", i, input_devices[i].name, input_devices[i].fd);
    }
    if (input_devices == NULL || dev_cnt < 1) {
        printf("No input devices found\n");
        return;
    }

    for (FBInkInputDevice* device = input_devices; device < input_devices + dev_cnt; device++) {
        printf("Device: %s\n", device->name);
        // YOLO, assume there's only one touchscreen
        if (device->matched) {
            evfd = device->fd;
        }
    }
    if (evfd == -1) {
        printf("No touchscreen found\n");
        return;
    }
    free(input_devices);

    printf("Using device [fd%d] for input\n", evfd);
    dev    = libevdev_new();
	int rc = libevdev_set_fd(dev, evfd);
	if (rc < 0) {
		sprintf(stderr, "Failed to initialize libevdev (%s)", strerror(-rc));
        init_failed = true;
        return;
	} else {
        printf("libevdev initialized\n");
    }

    if (libevdev_grab(dev, LIBEVDEV_GRAB) != 0) {
		sprintf(stderr, "Cannot read input events because the input device is currently grabbed by something else!");
        init_failed = true;
		return;
	} else {
        printf("libevdev grabbed :blobfoxcheer:\n");
    }

    printf("Initialized libevdev for device %s\n", libevdev_get_name(dev));

    pfd.fd            = evfd;
	pfd.events        = POLLIN;
}

void I_GetEvent(void) {
    event_t event;

    if (init_failed) {
        return;
    }

    int poll_num = poll(&pfd, 1, 0); // Doesn't matter if we time out, we can let the game run without inputs

    if (poll_num == -1) {
        if (errno == EINTR) {
            return;
        }
        sprintf(stderr, "poll: %m");
    } else if (poll_num > 0) {
        if (pfd.revents & POLLIN) {
            struct input_event ev;
            int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (rc == 0) {
                printf("Event: type %d code %d value %d\n", ev.type, ev.code, ev.value);
                if (ev.type == EV_ABS) {
                    switch (ev.code) {
                        case ABS_MT_POSITION_X:
                            touch_ev.pos.x = ev.value;
                            break;
                        case ABS_MT_POSITION_Y:
                            touch_ev.pos.y = ev.value;
                            break;
                        case ABS_MT_PRESSURE:
                            if (ev.value > 0) {
                                touch_ev.down = true;
                            } else {
                                touch_ev.down = false;
                            }
                            break;
                    }
                    printf("Touch %s: (%d, %d) \n", touch_ev.down ? "DOWN" : "UP", touch_ev.pos.x, touch_ev.pos.y);

                    for (int i = 0; i < sizeof(keys); i++) {
                        if (!keys[i]) {
                            break; // It's joever
                        }

                        // if (prev_ev.down == true && touch_ev.down == true) {
                        //     continue;
                        // }

                        Coord touch = touch_ev.pos;

                        if (touch.x >= keys[i]->rect.left && touch.x <= keys[i]->rect.left + keys[i]->rect.width &&
                            touch.y >= keys[i]->rect.top && touch.y <= keys[i]->rect.top + keys[i]->rect.height) {
                            printf("Key %d %s\n", keys[i]->key, touch_ev.down ? "DOWN" : "UP");
                            event.type = touch_ev.down ? ev_keydown : ev_keyup;
                            event.data1 = keys[i]->key;

                            D_PostEvent(&event);
                        }
                    }

                    prev_ev = touch_ev;
                                
                }
            }
        }
    } else {
        return;
    }
}

void I_ShutdownInput(void) {
    printf("I_ShutdownInput\n");
    if (dev != NULL) {
        libevdev_grab(dev, LIBEVDEV_UNGRAB);
        libevdev_free(dev);
    }
    if (evfd != -1) {
        close(evfd);
    }
}
