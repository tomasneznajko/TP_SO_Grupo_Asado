#ifndef MEMORIA_KERNEL_H_
#define MEMORIA_KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "proceso.h"



void enviar_segmento(int, char*, t_list*);
// void evaluar_respuesta(int,int,int);
bool actualizo_proceso(pcb*, t_list*);
void actualizar_cola(t_queue*,t_queue*, t_list*);
void actualizar_tablas(t_list*);




#endif /* MEMORIA_KERNEL_H_ */

