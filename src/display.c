//
// Created by Hugo Freire on 2016-12-12.
//

#include "display.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void display_image(const char *filename, const unsigned int timeout) {
    pid_t child_pid;

    child_pid = fork();

    if (child_pid == 0) {
        char *arg[] = {"fbi", "--noverbose", "-a", (char *) filename};

        if (execvp(arg[0], arg) < 0) {
            fprintf(stderr, "Failed to show image\n");
        }

        exit(0);
    } else {
        sleep(timeout);

        kill(child_pid, SIGKILL);
    }
}