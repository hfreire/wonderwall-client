/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdlib.h>
#include <stdio.h>

#include <amqp_tcp_socket.h>

#include <stdarg.h>
#include <stdbool.h>

#include <jansson.h>

#include <curl/curl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

const int DEFAULT_DISPLAY_TIMEOUT = 10;
char filename[FILENAME_MAX] = "/tmp/wonderwall.data";

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void die_on_error(int x, char const *context) {
    if (x < 0) {
        fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x));
        exit(1);
    }
}

void die_on_amqp_error(amqp_rpc_reply_t x, char const *context) {
    switch (x.reply_type) {
        case AMQP_RESPONSE_NORMAL:
            return;

        case AMQP_RESPONSE_NONE:
            fprintf(stderr, "%s: missing RPC reply type!\n", context);
            break;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
            break;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            switch (x.reply.id) {
                case AMQP_CONNECTION_CLOSE_METHOD: {
                    amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
                    fprintf(stderr, "%s: server connection error %uh, message: %.*s\n",
                            context,
                            m->reply_code,
                            (int) m->reply_text.len, (char *) m->reply_text.bytes);
                    break;
                }
                case AMQP_CHANNEL_CLOSE_METHOD: {
                    amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
                    fprintf(stderr, "%s: server channel error %uh, message: %.*s\n",
                            context,
                            m->reply_code,
                            (int) m->reply_text.len, (char *) m->reply_text.bytes);
                    break;
                }
                default:
                    fprintf(stderr, "%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
                    break;
            }
            break;
    }

    exit(1);
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

int download_image(const char *url) {
    FILE *file;
    CURL *curl;
    CURLcode code;

    if (!(file = fopen(filename, "wb"))) {
        fprintf(stderr, "error: could not open temporary file\n");

        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "error: curl failed to initialize\n");

        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    code = curl_easy_perform(curl);
    if (code) {
        fprintf(stderr, "error: curl failed to download URL with error code %d\n", code);

        curl_easy_cleanup(curl);
        fclose(file);

        return -1;
    }

    curl_easy_cleanup(curl);
    fclose(file);

    return 0;
}

void draw_image(const unsigned int timeout) {
    pid_t child_pid;

    child_pid = fork();

    if (child_pid == 0) {
        char *arg[] = {"fbi", "--noverbose", "-a", filename};

        if (execvp(arg[0], arg) < 0) {
            fprintf(stderr, "Failed to show image\n");
        }

        exit(0);
    } else {
        sleep(timeout);

        kill(child_pid, SIGKILL);
    }
}

void display_image(const char *url, const unsigned int timeout) {
    fprintf(stdout, "Displaying %s for %u sec\n", url, timeout);

    if (download_image(url) < 0) {
        fprintf(stderr, "Failed to download URL: %s\n", url);
        return;
    }

    draw_image(timeout);
}

void handle_message(amqp_message_t message) {
    char *body = NULL;
    json_t *json_body;
    json_t *json_url;
    json_t *json_timeout;
    const char *url = NULL;
    unsigned int timeout = DEFAULT_DISPLAY_TIMEOUT;

    if (message.body.len <= 0) {
        return;
    }

    body = (char *) malloc((message.body.len + 1) * sizeof(char *));
    if (body == NULL) {
        fprintf(stderr, "Failed to allocate memory for message body\n");
    }
    memset(body, '\0', (message.body.len + 1) * sizeof(char *));
    memcpy(body, message.body.bytes, message.body.len);
    fprintf(stdout, "Handling message %s\n", body);

    json_body = parse_json(body);
    if (!json_body) {
        fprintf(stderr, "Failed to parse URL because of malformed JSON message\n");

        return;
    }

    json_url = json_object_get(json_body, "url");
    if (!json_url) {
        fprintf(stderr, "Failed to parse URL because of invalid JSON message\n");

        json_decref(json_body);

        return;
    }

    url = json_string_value(json_url);
    if (!url || strlen(url) == 0) {
        fprintf(stderr, "Failed to parse URL because of invalid JSON type\n");

        json_decref(json_body);

        return;
    }

    json_timeout = json_object_get(json_body, "timeout");
    if (json_timeout) {
        timeout = (unsigned int) json_integer_value(json_timeout);
        if (!timeout) {
            fprintf(stderr, "Failed to parse timeout because of invalid JSON type\n");

            json_decref(json_body);

            return;
        }
    }

    display_image(url, timeout);

    json_decref(json_body);

    free(body);

    remove(filename);
}

static void run(amqp_connection_state_t conn) {
    amqp_frame_t frame;

    while (true) {
        amqp_rpc_reply_t ret;
        amqp_envelope_t envelope;
        amqp_maybe_release_buffers(conn);
        ret = amqp_consume_message(conn, &envelope, NULL, 0);

        if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
            if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
                AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
                if (AMQP_STATUS_OK != amqp_simple_wait_frame(conn, &frame)) {
                    return;
                }

                if (AMQP_FRAME_METHOD == frame.frame_type) {
                    switch (frame.payload.method.id) {
                        case AMQP_BASIC_ACK_METHOD:
                            /* if we've turned publisher confirms on, and we've published a message
                             * here is a message being confirmed
                             */

                            break;
                        case AMQP_BASIC_RETURN_METHOD:
                            /* if a published message couldn't be routed and the mandatory flag was set
                             * this is what would be returned. The message then needs to be read.
                             */
                        {
                            amqp_message_t message;
                            ret = amqp_read_message(conn, frame.channel, &message, 0);
                            if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                                return;
                            }

                            amqp_destroy_message(&message);
                        }

                            break;

                        case AMQP_CHANNEL_CLOSE_METHOD:
                            /* a channel.close method happens when a channel exception occurs, this
                             * can happen by publishing to an exchange that doesn't exist for example
                             *
                             * In this case you would need to open another channel redeclare any queues
                             * that were declared auto-delete, and restart any consumers that were attached
                             * to the previous channel
                             */
                            return;

                        case AMQP_CONNECTION_CLOSE_METHOD:
                            /* a connection.close method happens when a connection exception occurs,
                             * this can happen by trying to use a channel that isn't open for example.
                             *
                             * In this case the whole connection must be restarted.
                             */
                            return;

                        default:
                            fprintf(stderr, "An unexpected method was received %u\n", frame.payload.method.id);
                            return;
                    }
                }
            }

        } else {
            handle_message(envelope.message);

            memset(envelope.message.body.bytes, '\0', envelope.message.body.len);

            amqp_destroy_envelope(&envelope);
        }
    }
}

int main(int argc, char const *const *argv) {
    char const *hostname;
    char const *username;
    char const *password;
    int port, status;
    char const *exchange;
    char const *bindingkey;
    amqp_socket_t *socket = NULL;
    amqp_connection_state_t conn;

    amqp_bytes_t queue;

    if (argc < 6) {
        fprintf(stderr, "Usage: wonderwalld host port username password\n");
        return 1;
    }

    hostname = argv[1];
    port = atoi(argv[2]);
    username = argv[3];
    password = argv[4];
    queue = amqp_cstring_bytes(argv[5]);
    exchange = "amq.direct"; /* argv[3]; */
    bindingkey = "test queue"; /* argv[4]; */

    conn = amqp_new_connection();

    socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        die("creating TCP socket");
    }

    status = amqp_socket_open(socket, hostname, port);
    if (status) {
        die("opening TCP socket");
    }

    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, username, password), "Logging in");
    amqp_channel_open(conn, 1);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    amqp_queue_bind(conn, 1, queue, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey), amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

    amqp_basic_consume(conn, 1, queue, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    run(conn);

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");

    return 0;
}