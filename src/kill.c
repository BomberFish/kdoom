// bomberfish 2024
// File: kill.c
// Killall in C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

void kill_processes_by_name(const char *process_name) {
    DIR *dir;
    struct dirent *entry;

    // Open the /proc directory
    if ((dir = opendir("/proc")) == NULL) {
        perror("opendir(/proc)");
        return;
    }

    // Iterate over all entries in /proc
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry name is a number (PID)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            char cmdline_path[256];
            snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);

            FILE *cmdline_file = fopen(cmdline_path, "r");
            if (cmdline_file) {
                char cmdline[256];
                if (fgets(cmdline, sizeof(cmdline), cmdline_file)) {
                    // Check if the process name matches
                    if (strstr(cmdline, process_name) != NULL) {
                        pid_t pid = (pid_t) atoi(entry->d_name);
                        if (kill(pid, SIGKILL) == 0) {
                            printf("Killed process %d (%s)\n", pid, cmdline);
                        } else {
                            perror("kill");
                        }
                    }
                }
                fclose(cmdline_file);
            }
        }
    }

    closedir(dir);
}
