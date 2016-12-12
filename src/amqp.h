/*
 * Copyright (c) 2016, Hugo Freire <hugo@exec.sh>.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef WONDERWALLD_AMQP_H
#define WONDERWALLD_AMQP_H

#include <amqp_tcp_socket.h>

void die(const char *fmt, ...);

void die_on_error(int x, char const *context);

void die_on_amqp_error(amqp_rpc_reply_t x, char const *context);

void connect(const char *hostname,
             const int port,
             const char *username,
             const char *password,
             const char *queue,
             const char *exchange,
             const char *bindingkey,
             void (*handle_message)(char *)
);

static void listen(amqp_connection_state_t conn, void (*handle_message)(char *));

#endif //WONDERWALLD_AMQP_H
