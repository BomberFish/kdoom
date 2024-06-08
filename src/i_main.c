//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "i_video.h"

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

void HandleSignal(int sig)
{
    printf("Caught signal %d, quitting...\r\n", sig);
    I_Quit();
}

struct sigaction sa;

int main(int argc, char **argv)
{
    // save arguments

    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

    // start doom
    printf("Starting D_DoomMain\r\n");
    D_DoomMain ();

    sa.sa_handler = HandleSignal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);

    return 0;

}
