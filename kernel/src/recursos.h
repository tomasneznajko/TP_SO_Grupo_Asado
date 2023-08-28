#ifndef RECURSOS_H_
#define RECURSOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/config.h"
#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "proceso.h"

typedef struct {
    char nombre[20];
    int instancias;
    t_queue* procesosBloqueados;
    struct Recurso *siguiente;
} Recurso;

extern t_list* recursos;

t_list* leerRecursos(t_config *config);

void manejo_recursos(pcb*);

void destruir_recursos(t_list*);

void destruir_colas_recursos(void*);

#endif /* RECURSOS_H_ */
