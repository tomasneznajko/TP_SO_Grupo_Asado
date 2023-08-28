#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "server.h"
#include <shared.h>
#include <stdbool.h>

//STRUCTS

typedef enum {
	FIRST,
	BEST,
	WORST
}algoritmo_asignacion;

/*
typedef struct
{
	char* ip_memoria;
	char* puerto_escucha;
	int   tam_memoria;
	int   tam_segmento_0;
	int   cant_segmentos;
	int   retardo_memoria;
	int   retardo_compactacion;
	algoritmo_asignacion algoritmo;
} archivo_configuracion;*/

typedef struct
{
	unsigned int pid;
	int id;
	int tam_segmento; 
	int direccion_base;
	int direccion_limite;

}segmento;

typedef enum {
	M_READ,
	M_WRITE
}inst_mem;


//VARIABLES
extern void* memoria_usuario;
extern t_list* tabla_segmentos_total;
extern t_list* huecos;
extern t_config *config;
//extern archivo_configuracion config_mem;
extern char *ip_memoria;

//FUNCIONES
void eliminar_hueco(int , int );
algoritmo_asignacion cambiar_enum_algoritmo (char* );
t_list* obtener_segmentos_PID(unsigned int );
int hay_segmentos_disponibles(unsigned int );
int crear_segmento(unsigned int , int, int);
void eliminar_segmento(unsigned int,int );
//void iniciar_proceso(unsigned int , int , int );
void finalizar_proceso(t_instruccion* );
int agrupar_huecos(int , int );

void iniciar_memoria ();
//void cargar_config (t_config* );
void iniciar_estructuras(pcb*, int);
int hay_espacio_disponible(int);
int first_fit (unsigned int, int, int);
int best_fit (unsigned int, int, int);
int worst_fit (unsigned int, int, int);
//void manejo_instrucciones (t_list* , int* );
void* leer_memoria(int, size_t);
void escribir_memoria(int, void* nuevo_valor, size_t tamanio);
void terminar_memoria(t_log *logger, t_config *config, int socket);
int comparar_segmentos(void* , void* );
void obtener_huecos_libres ();
int sumatoria_huecos();
t_list* compactar_segmentos();
t_list* iniciar_proceso(unsigned int);

#endif /* MEMORIA_H_ */
