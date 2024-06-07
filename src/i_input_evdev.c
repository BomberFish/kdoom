// bomberfish 2024
// File: i_input_raw.c
// Raw /dev/input touchscreen input for kdoom. Most code taken from FBInk's finger_trace sample.
// TODO: Add controls, make the code look nicer.

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

void kill_processes_by_name(const char *process_name);

int vanilla_keyboard_mapping = 1;

struct pollfd pfd;

// Is the shift key currently down?

static int shiftdown = 0;

FBInkInputDevice *input_devices = NULL;
int dev_cnt = 0;

struct libevdev *dev = NULL;
int evfd = -1;

bool init_failed = false;

void I_InitInput(void) {
    printf("I_InitInput\n");
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
