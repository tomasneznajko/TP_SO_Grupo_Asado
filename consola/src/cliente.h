#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/process.h>
#include <shared.h>

int crear_conexion(char*, char*);
void send_handshake(int);
void atender_kernel(int);
void liberar_conexion(int);

#endif /* CLIENTE_H_ */
