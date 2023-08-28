#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "recursos.h"
#include "server.h"
#include "fileSystemKernel.h"
#include "memoriaKernel.h"



int crear_conexion(char*, char*);
void send_handshake(int);
void esperar_servidor(int);
void atender_servidor(int*);
// void enviar_instruccion_parseada(instruccion);
void liberar_conexion(int);

#endif /* CLIENTE_H_ */
