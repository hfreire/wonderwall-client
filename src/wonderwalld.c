/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <jansson.h>

#include "../include/wonderwalld.h"
#include "amqp.h"
#include "download.h"
#include "display.h"

void handle_shutdown(int signal) {
    fprintf(stdout, "Received signal %i\n", signal);

    fprintf(stdout, "Disconnecting from AMQP server\n");
    disconnect_amqp();

    fprintf(stdout, "Closing display\n");
    destroy_display();

    exit(0);
}

json_t *parse_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "error: jansson failed to parse JSON on line %d: %s\n", error.line, error.text);

        return NULL;
    }
}

void handle_message(char *message) {
    json_t *json_message;
    json_t *json_url;
    json_t *json_timeout;
    const char *url = NULL;
    unsigned int timeout = DEFAULT_DISPLAY_TIMEOUT;

    fprintf(stdout, "Handling message %s\n", message);

    json_message = parse_json(message);
    if (!json_message) {
        fprintf(stderr, "Failed to parse URL because of malformed JSON message\n");

        return;
    }

    json_url = json_object_get(json_message, "url");
    if (!json_url) {
        fprintf(stderr, "Failed to parse URL because of invalid JSON message\n");

        json_decref(json_message);

        return;
    }

    url = json_string_value(json_url);
    if (!url || strlen(url) == 0) {
        fprintf(stderr, "Failed to parse URL because of invalid JSON type\n");

        json_decref(json_message);

        return;
    }

    json_timeout = json_object_get(json_message, "timeout");
    if (json_timeout) {
        timeout = (unsigned int) json_integer_value(json_timeout);
        if (!timeout) {
            fprintf(stderr, "Failed to parse timeout because of invalid JSON type\n");

            json_decref(json_message);

            return;
        }
    }

    if (download_image(url, filename) < 0) {
        fprintf(stderr, "Failed to download URL: %s\n", url);

        json_decref(json_message);

        remove(filename);

        return;
    }

    show_image(filename, timeout);

    json_decref(json_message);

    remove(filename);
}

int main(int argc, char **argv) {
    struct sigaction action;
    int width = DISPLAY_MAX_WIDTH;
    int height = DISPLAY_MAX_HEIGHT;
    char const *hostname;
    char const *username;
    char const *password;
    char const *queue;
    int port;
    char const *exchange;
    char const *bindingkey;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = handle_shutdown;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    if (argc < 6) {
        fprintf(stderr, "Usage: wonderwalld host port username password queue [width] [height]\n");
        return 1;
    }

    hostname = argv[1];
    port = atoi(argv[2]);
    username = argv[3];
    password = argv[4];
    queue = argv[5];
    exchange = "amq.direct";
    bindingkey = "test queue";

    if (argv[6] != NULL && argv[7] != NULL) {
        width = atoi(argv[6]);
        height = atoi(argv[7]);
    }

    init_display(width, height);

    connect_amqp(hostname, port, username, password, queue, exchange, bindingkey, handle_message);

    return 0;
}
