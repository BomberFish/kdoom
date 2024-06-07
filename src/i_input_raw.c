// bomberfish 2024
// File: i_input_raw.c
// Raw /dev/input touchscreen input for kdoom
// TODO: Add controls, make more device-agnostic, don't kill Xorg on every tick!

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

struct pollfd fds;

// Is the shift key currently down?

static int shiftdown = 0;

int screen_fd = -1;

void find_touchscreen(void) {
    struct dirent *entry;
    DIR *dp = opendir("/dev/input");
    if (dp == NULL) {
        perror("opendir");
    }
    // Iterate thru /dev/input/*
    while ((entry = readdir(dp))) {
        printf("entry->d_name: %s\n", entry->d_name);
        if (strstr(entry->d_name, "event") != NULL) {
            char filename[256];
            snprintf(filename, sizeof(filename), "/dev/input/%s", entry->d_name);
            int fd = open(filename, O_RDONLY|O_NONBLOCK);
            if (fd < 0) {
                perror("open");
                continue;
            }
            char name[256]; // Surely it won't be longer than this
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
                perror("ioctl");
                close(fd);
                continue;
            }
            if (strstr(name, "goodix-ts") != NULL) { // Name that shows up on my PW4. Not sure if it's the same for other devices.
                printf("Found touchscreen: %s\n", filename);
                screen_fd = fd;
                break;
            }
            close(fd);
        }
    }
    perror("not found");
}

void I_InitInput(void) {
    printf("I_InitInput\n");
    fds.events = POLLIN;
    find_touchscreen();
    if (screen_fd < 0) {
        sprintf(stderr, "Failed to find touchscreen\n");
    }
    printf("screen_fd: %d\n", screen_fd);
    fds.fd = screen_fd;
    kill_processes_by_name("Xorg"); // Kill X. Why did I do this.
    I_AtExit(I_ShutdownInput, true);
}

void I_GetEvent(void) {
    // printf("I_GetEvent\n");
    if (screen_fd < 0) {
        sprintf(stderr, "No touchscreen\n");
        return;
    }
    // To be honest, I have no clue what happens here.
    // I just copied it from StackOverflow.
    int ret = poll(&fds, 1, 5);
    if (ret > 0) {
        if (fds.revents & POLLIN) {
            struct input_event ev;
            ret = read(screen_fd, &ev, sizeof(struct input_event));
            if (ret < 0) {
                perror("read");
            } else {
                printf("ev.type: %d\n", ev.type);
                printf("ev.code: %d\n", ev.code);
                printf("ev.value: %d\n", ev.value);
            }
        }
    } else if (ret == 0) {
        // printf("event timed out\n");
    } else {
        perror("poll");
    }
}

void I_ShutdownInput(void) {
    printf("I_ShutdownInput\n");
    if (screen_fd >= 0) {
        close(screen_fd);
    }
    system("initctl start x");
}
