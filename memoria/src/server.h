#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <shared.h>
#include "memoria.h"

extern int conexion_kernel;
extern int conexion_cpu;
extern int conexion_filesystem;

int iniciar_servidor(char*);
void recv_handshake(int);
void esperar_cliente(int);
void atender_cliente(int*);
void liberar_servidor(int*);

#endif /* SERVER_H_ */
