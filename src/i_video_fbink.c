// bomberfish 2024
// File: i_video_fbink.c
// FBInk video for kdoom

#include "config.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "z_zone.h"

#include "doomkeys.h"
#include "tables.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "../FBInk/fbink.h"
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

int scale_factor = 2; // TODO: Scale based on res?

int frame = 0;

// Configuration for FBInk
FBInkConfig fbink_cfg = {
    // We need this set so FBInk doesn't freak out
    // when we give it DOOM's video buffer.
    // Honestly have no idea why it works, but it does :)
    .ignore_alpha = true,
// Log levels
#ifdef DEBUG
    .is_verbose = true,
    .is_quiet = false,
#else
    .is_verbose = false,
    .is_quiet = true,
#endif
    // E-ink waveform
    .wfm_mode = WFM_A2,
    .dithering_mode = HWD_ORDERED,
};

// Video buffer
byte *I_VideoBuffer = NULL;
byte *I_VideoBuffer_FB = NULL;

// File descriptor for the e-ink framebuffer
int fbink_fd = -1;

FBInkRect screen = {
    .left = 0,
    .top = 0,
    .width = SCREENWIDTH,
    .height = SCREENHEIGHT,
};

FBInkRect screen_scaled;

FBInkRect screen_padded = {
    .left = 0,
    .top = 0,
    .width = SCREENWIDTH + 50,
    .height = SCREENHEIGHT + 50,
};

// Variables required by the game

int usemouse = 0;

struct color {
  uint32_t b : 8;
  uint32_t g : 8;
  uint32_t r : 8;
  uint32_t a : 8;
};

// static struct color colors[256];

int X_width;
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

void PlaceKeys(void);

FBInkState fbink_state;

bool norefresh = false;

void I_GetScreenSize(int *width, int *height) {
  fbink_get_state(&fbink_cfg, &fbink_state);
  uint32_t w = fbink_state.screen_width;
  uint32_t h = fbink_state.screen_height;
  printf("Resolution: %d*%d\n", w, h);
  *width = (int)w;
  *height = (int)h;
}

// Initialize the video system
void I_InitGraphics(void) {
  usleep(500000); // sleep 0.5s
  printf("I_InitGraphics\n");

  // Open the framebuffer
  fbink_fd = open("/dev/fb0", O_RDWR);
  if (fbink_fd < 0) {
    fprintf(stderr, "couldnt open the framebuffer!!!\n");
    exit(1);
  }
  printf("fbink_fd: %d\n", fbink_fd);

  // Check cli args

  if (M_CheckParm("-nodither")) {
      fbink_cfg.dithering_mode = HWD_PASSTHROUGH;
  }

  if (M_CheckParm("-norefresh")) {
      norefresh = true;
  }

  // Initialize FBInk
  int ret = fbink_init(fbink_fd, &fbink_cfg);
  if (ret < 0 || ret == ENOSYS) {
    fprintf(stderr, "fbink_init failed: %d\n", ret);
    exit(1);
  }
  printf("fbink_init: %d\n", ret);

  // Get resolution
  fbink_get_state(&fbink_cfg, &fbink_state);
  uint32_t w = fbink_state.screen_width;
  uint32_t h = fbink_state.screen_height;
  printf("Resolution: %d*%d\n", w, h);

  // Calculate scale factor
  scale_factor = w / SCREENWIDTH;

  screen_scaled = (FBInkRect){
      .left = 0,
      .top = 0,
      .width = w,
      .height = h,
  };

  for (int i = 0; i < 3; i++) { // Prevent menu ghosting
    // Clear the screen
    ret = fbink_cls(fbink_fd, &fbink_cfg, &screen_scaled, false);
    printf("fbink_cls: %d\n", ret);
  }

  // Allocate video buffer
  I_VideoBuffer = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
  I_VideoBuffer_FB = (byte *)Z_Malloc((SCREENWIDTH * scale_factor) *
                                          (SCREENHEIGHT * scale_factor),
                                      PU_STATIC, NULL);

  // Finish up
  screenvisible = true;

  // Initialize input
  extern int I_InitInput(void);
  I_InitInput();
}

// Shutdown the video system
void I_ShutdownGraphics(void) {
  printf("I_ShutdownGraphics\n");
  // Not sure what this does over just doing a close() but FBInk recommends
  // doing it
  fbink_close(fbink_fd);
}

// Update the screen.
// this is where the magic happens (bazinga)
void I_FinishUpdate(void) {
#ifdef DEBUG
  printf("I_FinishUpdate\n");
#endif
  int ret;

  if (frame == 0 && !norefresh) {
    // Clear the screen
    ret = fbink_cls(fbink_fd, &fbink_cfg, &screen_scaled, false);
    printf("fbink_cls: %d\n", ret);
    // Redraw buttons
    PlaceKeys();
  }

  // clearing the screen on each frame would technically look better,
  // but since the refresh rate on the e-ink is so bad,
  // we can't afford to do that without it looking like hot trash
  // ret = fbink_cls(fbink_fd, &fbink_cfg, &screenLarger, false);
  // printf("fbink_cls: %d\n", ret);

  // Scale video buffer by scale_factor
  for (int i = 0; i < SCREENHEIGHT; i++) { // Iterate over pixels
    for (int j = 0; j < SCREENWIDTH; j++) {
      for (int k = 0; k < scale_factor; k++) {   // duplicate lines
        for (int l = 0; l < scale_factor; l++) { // duplicate pixels
          I_VideoBuffer_FB[(i * scale_factor + k) * SCREENWIDTH * scale_factor +
                           (j * scale_factor + l)] =
              I_VideoBuffer[i * SCREENWIDTH + j];
        }
      }
    }
  }

  // Finally, print the buffer to the screen
  ret = fbink_print_raw_data(
      fbink_fd, (unsigned char *)I_VideoBuffer_FB, SCREENWIDTH * scale_factor,
      SCREENHEIGHT * scale_factor,
      (SCREENWIDTH * scale_factor) * (SCREENHEIGHT * scale_factor), 0, 0,
      &fbink_cfg);

#ifdef DEBUG
  printf("fbink_print_raw_data: %d\n", ret);
#endif

  // usleep(290000);
}

void I_StartFrame(void) {
#ifdef DEBUG
  printf("I_StartFrame\n");
#endif
  frame++;
  if (frame > TICRATE * 5) { // hopefully gcc optimizes this to a constant...
    frame = 0;
  }
}

// Super simple function to read the video buffer
void I_ReadScreen(byte *scr) {
  printf("I_ReadScreen\n");
  memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

// Input stuff
// __attribute__((weak)) void I_GetEvent(void) {

// }

void I_StartTic(void) {
    I_GetEvent();
}

// Stuff the game expects but we don't care about
void I_UpdateNoBlit(void) {
#ifdef DEBUG
  printf("(N/I) I_UpdateNoBlit\n");
#endif
}
void I_SetPalette(byte *palette) {
#ifdef DEBUG
  printf("(N/I) I_SetPalette\n");
#endif
}
int I_GetPaletteIndex(int r, int g, int b) {
#ifdef DEBUG
  printf("(N/I) I_GetPaletteIndex\n");
#endif
  return 0;
}
void I_BeginRead(void) {
#ifdef DEBUG
  printf("(N/I) I_BeginRead\n");
#endif
}
void I_EndRead(void) {
#ifdef DEBUG
  printf("(N/I) I_EndRead\n");
#endif
}
void I_SetWindowTitle(char *title) {
#ifdef DEBUG
  printf("(N/I) I_SetWindowTitle\n");
#endif
}
void I_GraphicsCheckCommandLine(void) {
#ifdef DEBUG
  printf("(N/I) I_GraphicsCheckCommandLine\n");
#endif
}
void I_SetGrabMouseCallback(grabmouse_callback_t func) {
#ifdef DEBUG
  printf("(N/I) I_SetGrabMouseCallback\n");
#endif
}
void I_EnableLoadingDisk(void) {
#ifdef DEBUG
  printf("(N/I) I_EnableLoadingDisk\n");
#endif
}
void I_BindVideoVariables(void) {
#ifdef DEBUG
  printf("(N/I) I_BindVideoVariables\n");
#endif
}
void I_DisplayFPSDots(boolean dots_on) {
#ifdef DEBUG
  printf("(N/I) I_DisplayFPSDots\n");
#endif
}
void I_CheckIsScreensaver(void) {
#ifdef DEBUG
  printf("(N/I) I_CheckIsScreensaver\n");
#endif
}
