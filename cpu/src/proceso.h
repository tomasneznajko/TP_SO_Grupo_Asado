#ifndef PROCESO_H_
#define PROCESO_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <shared.h>

extern int conexion_kernel;
extern int conexion_memoria;
extern pcb* proceso;
extern t_dictionary* instrucciones;
extern t_dictionary* registros;

typedef enum
{
	I_SET,
	I_MOV_IN,
	I_MOV_OUT,
	I_IO,
	I_F_OPEN,
	I_F_CLOSE,
	I_F_SEEK,
	I_F_READ,
	I_F_WRITE,
	I_F_TRUNCATE,
	I_WAIT,
	I_SIGNAL,
	I_CREATE_SEGMENT,
	I_DELETE_SEGMENT,
	I_YIELD,
	I_EXIT
}enum_instrucciones;

void iniciar_diccionario_instrucciones(void);
void iniciar_diccionario_registros(registros_cpu*);
void destruir_diccionarios(void);
void interpretar_instrucciones();
void instruccion_set(char**);
void instruccion_mov_in(char**);
void mov_in(t_instruccion*);
void instruccion_mov_out(char**);
void instruccion_i_o(char**);
void instruccion_f_open(char**);
void instruccion_f_close(char**);
void instruccion_f_seek(char**);
void instruccion_f_read(char**);
void instruccion_f_write(char**);
void instruccion_f_truncate(char**);
void instruccion_wait(char**);
void instruccion_signal(char**);
void instruccion_create_segment(char**);
void instruccion_delete_segment(char**);
void instruccion_yield(char**);
void instruccion_exit(char**);
void error_exit(op_code);
char* traducir_dir_logica(char*, int);

#endif /* PROCESO_H_ */
