/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "amqp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>

amqp_connection_state_t conn;

void die(const char *fmt, ...) {
    fprintf(stderr, "%s\n", fmt);
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

void listen_amqp(amqp_connection_state_t conn, void (*handle_message)(char *)) {
    amqp_frame_t frame;
    char *message = NULL;

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
                            amqp_message_t m;
                            ret = amqp_read_message(conn, frame.channel, &m, 0);
                            if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                                return;
                            }

                            amqp_destroy_message(&m);
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
            if (envelope.message.body.len <= 0) {
                amqp_destroy_envelope(&envelope);

                return;
            }

            message = (char *) malloc((envelope.message.body.len + 1) * sizeof(char *));
            if (message == NULL) {
                fprintf(stderr, "Failed to allocate memory for message body\n");

                amqp_destroy_envelope(&envelope);

                return;
            }

            memset(message, '\0', (envelope.message.body.len + 1) * sizeof(char *));
            memcpy(message, envelope.message.body.bytes, envelope.message.body.len);

            handle_message(message);

            free(message);

            amqp_destroy_envelope(&envelope);
        }
    }
}

void connect_amqp(const char *hostname,
                  const int port,
                  const char *username,
                  const char *password,
                  const char *queue,
                  const char *exchange,
                  const char *bindingkey,
                  void (*handle_message)(char *)
) {
    int status;
    amqp_socket_t *socket = NULL;

    fprintf(stdout, "Connecting to AMQP server %s:%i\n", hostname, port);

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

    amqp_queue_bind(conn, 1, amqp_cstring_bytes(queue), amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey),
                    amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue), amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    listen_amqp(conn, handle_message);
}

void disconnect_amqp() {
    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");
}