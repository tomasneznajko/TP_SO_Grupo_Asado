#ifndef FILESYSTEMKERNEL_H_
#define FILESYSTEMKERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "proceso.h"

typedef struct
{
	char *nombreDeArchivo;
	int puntero;
	t_queue* procesosBloqueados;
	struct Archivo *siguiente;
}Archivo;

extern t_list* archivosAbiertos;

extern int contadorDeEscrituraOLectura;

int abrirArchivoKernel(pcb*, char*);
void cerrarArchivoKernel(pcb*, char*);
void buscarEnArchivo(pcb*, char*);
void truncarArchivo(pcb*, char*);
Archivo *estoDevuelveUnArchivo(pcb*, char*);
bool condicionParaBorrar(t_list*, char*);


#endif /* FILESYSTEMKERNEL_H_ */
