/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <stdlib.h>

void print_usage() {
    printf("Usage: wonderwallc <-b broker hostname>\n");
}

int main(int argc, char **argv) {
    char *broker = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i] == NULL || argv[i][0] != '-') break;
        switch (argv[i][1]) {
            case 'h':
                broker = argv[++i];
                break;
            default:
                fprintf(stderr, "%s: %c: uknown option\n", argv[0], argv[i][1]);
                print_usage();

                exit(0);
        }
    }

    if (broker == NULL) {
        print_usage();
        exit(0);
    }
}