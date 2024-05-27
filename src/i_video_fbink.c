// bomberfish 2024

#include "config.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "z_zone.h"

#include "doomkeys.h"
#include "tables.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "../FBInk/fbink.h"
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

// Configuration for FBInk
FBInkConfig fbink_cfg = {
    // We need this set so FBInk doesn't freak out
    // when we give it DOOM's video buffer.
    // Honestly have no idea why it works, but it does :)
    ignore_alpha: true,
};

// Video buffer
byte *I_VideoBuffer = NULL;

// File descriptor for the e-ink framebuffer
int fbink_fd = -1;

FBInkRect screen = {
    left: 0,
    top: 0,
    width: SCREENWIDTH,
    height: SCREENHEIGHT,
};

FBInkRect screenLarger = {
    left: 0,
    top: 0,
    width: SCREENWIDTH + 50,
    height: SCREENHEIGHT + 50,
};

// Variables required by the game

int usemouse = 0;

struct color {
    uint32_t b:8;
    uint32_t g:8;
    uint32_t r:8;
    uint32_t a:8;
};

// static struct color colors[256];

int	X_width;
int X_height;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

// Initialize the video system
void I_InitGraphics(void) {
    printf("I_InitGraphics\n");
    // Open the framebuffer
    fbink_fd = open("/dev/fb0", O_RDWR);
    if (fbink_fd < 0) {
        fprintf(stderr, "couldnt open the framebuffer!!!\n");
        exit(1);
    }
    printf("fbink_fd: %d\n", fbink_fd);

    // Initialize FBInk
    int ret = fbink_init(fbink_fd, &fbink_cfg);
    if (ret < 0 || ret == ENOSYS) {
        fprintf(stderr, "fbink_init failed: %d\n", ret);
        exit(1);
    }
    printf("fbink_init: %d\n", ret);

    // Clear the screen
    ret = fbink_cls(fbink_fd, &fbink_cfg, &screenLarger, false);
    printf("fbink_cls: %d\n", ret);

    // Allocate video buffer
    I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    // Finish up
    screenvisible = true;

    extern int I_InitInput(void);
    I_InitInput();
}

// Shutdown the video system
void I_ShutdownGraphics(void) {
    printf("I_ShutdownGraphics\n");
    // Not sure what this does over just doing a close() but FBInk recommends doing it
    fbink_close(fbink_fd);
}

// Update the screen.
// this is where the magic happens (bazinga)
void I_FinishUpdate(void) {
    printf("I_FinishUpdate\n");
    int ret;

    // clearing the screen on each frame would technically look better,
    // but since the refresh rate on the e-ink is so bad,
    // we can't afford to do that without it looking like hot trash
    // ret = fbink_cls(fbink_fd, &fbink_cfg, &screenLarger, false);
    // printf("fbink_cls: %d\n", ret);

    // Print the video buffer to the screen
    ret = fbink_print_raw_data(
        fbink_fd,
        (unsigned char*)I_VideoBuffer,
        SCREENWIDTH,
        SCREENHEIGHT,
        SCREENWIDTH*SCREENHEIGHT,
        0,
        0,
        &fbink_cfg
    );
    printf("fbink_print_raw_data: %d\n", ret);
}

// Super simple function to read the video buffer
void I_ReadScreen(byte *scr) {
    printf("I_ReadScreen\n");
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

// Functions that are useless to us, but the game expects them to exist
void I_StartFrame(void) {printf("I_StartFrame\n");}
__attribute__((weak)) void I_GetEvent(void) {printf("I_GetEvent\n");}
__attribute__((weak)) void I_StartTic(void) {printf("I_StartTic\n");}
void I_UpdateNoBlit(void) {printf("I_UpdateNoBlit\n");}
void I_SetPalette(byte *palette) {printf("I_SetPalette\n");}
int I_GetPaletteIndex(int r, int g, int b) { return 0; }
void I_BeginRead(void) {printf("I_BeginRead\n");}
void I_EndRead(void) {printf("I_EndRead\n");}
void I_SetWindowTitle(char *title) {printf("I_SetWindowTitle\n");}
void I_GraphicsCheckCommandLine(void) {printf("I_GraphicsCheckCommandLine\n");}
void I_SetGrabMouseCallback(grabmouse_callback_t func) {printf("I_SetGrabMouseCallback\n");}
void I_EnableLoadingDisk(void) {printf("I_EnableLoadingDisk\n");}
void I_BindVideoVariables(void) {printf("I_BindVideoVariables\n");}
void I_DisplayFPSDots(boolean dots_on) {printf("I_DisplayFPSDots\n");}
void I_CheckIsScreensaver(void) {printf("I_CheckIsScreensaver\n");}
