#ifndef PROCESO_H_
#define PROCESO_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/process.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <shared.h>

extern int conexion_cpu;
extern int conexion_memoria;
extern int conexion_filesystem;
extern t_queue* qnew;
extern t_queue* qready;
extern t_queue* qexec;
extern t_queue* qblock;
extern t_queue* qexit;
extern sem_t* sem_largo_plazo;
extern sem_t* sem_cpu;
extern sem_t* sem_new;
extern sem_t* sem_ready;
extern sem_t* sem_exec;
extern sem_t* sem_block;
extern sem_t* sem_exit;
extern sem_t* sem_new_ready;
extern sem_t* sem_exec_exit;
extern sem_t* sem_escrituraLectura;
extern t_temporal* tiempo_en_cpu;
extern t_dictionary* conexiones;


void iniciar_colas(void);
void destruir_colas(void);
void iniciar_semaforos(void);
void destruir_semaforos(void);
sem_t* iniciar_semaforo(int, unsigned int);
void destruir_semaforo(sem_t*);
char* queue_iterator(t_queue*);
pcb* queue_seek(t_queue*, unsigned int);
void queue_extract(t_queue*, pcb*);
void generar_proceso(t_list*, int*);
void gestionar_multiprogramación(void);
void new_a_ready(void);
void ready_a_exec(void);
void exec_a_ready(void);
pcb* exec_a_block(void);
void block_a_ready(pcb*);
void exec_a_exit(char*);
void finalizar_proceso(char*);
void planificador(t_queue*);
void calcular_estimacion(pcb*, int64_t);
bool HRRN_comparator(void*, void*);
double HRRN_R(pcb*);
int seconds_from_string_time(char*);
int month_days(int);
void io_block(void);

#endif /* PROCESO_H_ */
