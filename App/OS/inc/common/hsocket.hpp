#ifndef HSOCKET_HPP
#define HSOCKET_HPP

#include "hcore.hpp"
#include <stdint.h>

namespace OS {

extern bool socket_connected;

enum SocketMode { CLIENT, SERVER };

void socket_set_core(HCore::HCore *core);

void socket_init(SocketMode sm);

int socket_connect(const char *server, uint16_t port);

int socket_listen(uint16_t port);
int socket_accept();

int socket_receive(void *data, unsigned int size);
int socket_send(void *data, unsigned int size);

void socket_close();

} // namespace OS

#endif
