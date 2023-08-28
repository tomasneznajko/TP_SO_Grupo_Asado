#include "proceso.h"

int conexion_kernel;
int conexion_memoria;
pcb *proceso;
t_dictionary *instrucciones;
t_dictionary *registros;

void iniciar_diccionario_instrucciones(void)
{
	instrucciones = dictionary_create();
	dictionary_put(instrucciones, "SET", (void *)(intptr_t)I_SET);
	dictionary_put(instrucciones, "MOV_IN", (void *)(intptr_t)I_MOV_IN);
	dictionary_put(instrucciones, "MOV_OUT", (void *)(intptr_t)I_MOV_OUT);
	dictionary_put(instrucciones, "I/O", (void *)(intptr_t)I_IO);
	dictionary_put(instrucciones, "F_OPEN", (void *)(intptr_t)I_F_OPEN);
	dictionary_put(instrucciones, "F_CLOSE", (void *)(intptr_t)I_F_CLOSE);
	dictionary_put(instrucciones, "F_SEEK", (void *)(intptr_t)I_F_SEEK);
	dictionary_put(instrucciones, "F_READ", (void *)(intptr_t)I_F_READ);
	dictionary_put(instrucciones, "F_WRITE", (void *)(intptr_t)I_F_WRITE);
	dictionary_put(instrucciones, "F_TRUNCATE", (void *)(intptr_t)I_F_TRUNCATE);
	dictionary_put(instrucciones, "WAIT", (void *)(intptr_t)I_WAIT);
	dictionary_put(instrucciones, "SIGNAL", (void *)(intptr_t)I_SIGNAL);
	dictionary_put(instrucciones, "CREATE_SEGMENT", (void *)(intptr_t)I_CREATE_SEGMENT);
	dictionary_put(instrucciones, "DELETE_SEGMENT", (void *)(intptr_t)I_DELETE_SEGMENT);
	dictionary_put(instrucciones, "YIELD", (void *)(intptr_t)I_YIELD);
	dictionary_put(instrucciones, "EXIT", (void *)(intptr_t)I_EXIT);
}

void iniciar_diccionario_registros(registros_cpu *registro)
{
	registros = dictionary_create();
	dictionary_put(registros, "AX", (void *)registro->AX);
	dictionary_put(registros, "BX", (void *)registro->BX);
	dictionary_put(registros, "CX", (void *)registro->CX);
	dictionary_put(registros, "DX", (void *)registro->DX);
	dictionary_put(registros, "EAX", (void *)registro->EAX);
	dictionary_put(registros, "EBX", (void *)registro->EBX);
	dictionary_put(registros, "ECX", (void *)registro->ECX);
	dictionary_put(registros, "EDX", (void *)registro->EDX);
	dictionary_put(registros, "RAX", (void *)registro->RAX);
	dictionary_put(registros, "RBX", (void *)registro->RBX);
	dictionary_put(registros, "RCX", (void *)registro->RCX);
	dictionary_put(registros, "RDX", (void *)registro->RDX);
}

void destruir_diccionarios(void)
{
	// dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}

void interpretar_instrucciones(void)
{
	// iniciar_diccionario_registros(&proceso->registros);
	while (proceso->program_counter < list_size(proceso->instrucciones))
	{
		// char* instruccion = list_get(proceso->instrucciones, proceso->program_counter);
		// log_debug(logger, "Instrucción: %s0", instruccion);
		char **parsed = string_split((char *)list_get(proceso->instrucciones, proceso->program_counter), " "); // Partes de la instruccion actual
		int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parsed[0]);
		switch (instruccion_enum)
		{
		case I_SET:
			instruccion_set(parsed);
			string_array_destroy(parsed);
			break;
		case I_MOV_IN:
			instruccion_mov_in(parsed);
			string_array_destroy(parsed);
			return;
			// break;
		case I_MOV_OUT:
			instruccion_mov_out(parsed);
			string_array_destroy(parsed);
			return;
			// break;
		case I_IO:
			instruccion_i_o(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		case I_F_OPEN:
			instruccion_f_open(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_F_CLOSE:
			instruccion_f_close(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_F_SEEK:
			instruccion_f_seek(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_F_READ:
			instruccion_f_read(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_F_WRITE:
			instruccion_f_write(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_F_TRUNCATE:
			instruccion_f_truncate(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_WAIT:
			instruccion_wait(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		case I_SIGNAL:
			instruccion_signal(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		case I_CREATE_SEGMENT:
			instruccion_create_segment(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_DELETE_SEGMENT:
			instruccion_delete_segment(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
			// break;
		case I_YIELD:
			instruccion_yield(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		case I_EXIT:
			instruccion_exit(parsed);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		case -1:
			log_warning(logger, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", proceso->pid);
			error_exit(EXIT);
			// destruir_diccionarios();
			string_array_destroy(parsed);
			return;
		}
	}
	log_warning(logger, "PID: %d - Advertencia: Sin instrucciones por ejecutar - Ejecutando: EXIT", proceso->pid);
	error_exit(EXIT);
	// destruir_diccionarios();
	return;
}

void instruccion_set(char **parsed)
{
	iniciar_diccionario_registros(&proceso->registros);
	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	memcpy(dictionary_get(registros, parsed[1]), parsed[2], strlen(parsed[2]));
	delay(config_get_int_value(config, "RETARDO_INSTRUCCION"));
	proceso->program_counter++;
	dictionary_destroy(registros);
}

void instruccion_mov_in(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado, descomentar el return en el case correspondiente, y comentar en donde se modifica el program_counter
	iniciar_diccionario_registros(&proceso->registros);
	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	int tamanio_dato = parsed[1][0] == 'R' ? 16 : parsed[1][0] == 'E' ? 8 : 4;
	char *dir_fisica = traducir_dir_logica(parsed[2], tamanio_dato);
	if (strcmp(dir_fisica, "SEG_FAULT") == 0)
	{
		// log_error(logger, "PID:  - Error SEG_FAULT - Segmento:  - Offset:  - Tamanio: ");
		error_exit(EXIT_SEG_FAULT);
		return;
	}
	char *instruccion = string_from_format("%s %s %s", parsed[0], parsed[1], dir_fisica);
	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion, free);
	log_trace(logger, "PID: %d - Instruccion traducida: %s", proceso->pid, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	t_instruccion *instruccion_proceso = generar_instruccion(proceso, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	enviar_instruccion(conexion_memoria, instruccion_proceso, MOV_IN);
	free(dir_fisica);
	free(instruccion_proceso);
	// proceso->program_counter++;
}

void mov_in(t_instruccion *instruccion_proceso)
{
	char **parsed = string_split(instruccion_proceso->instruccion, " ");
	memcpy(dictionary_get(registros, parsed[1]), instruccion_proceso->dato, instruccion_proceso->tamanio_dato);
	int id_segmento = floor((double)atoi(parsed[1]) / config_get_int_value(config, "TAM_MAX_SEGMENTO"));
	char* valor = string_substring_until(instruccion_proceso->dato, instruccion_proceso->tamanio_dato);
	log_info(logger, "PID: %u  Acción: LEER - Segmento: %d - Dirección Física: %s - Valor: %s", proceso->pid, id_segmento, parsed[2], valor);
	free(valor);
	proceso->program_counter++;
	dictionary_destroy(registros);
	string_array_destroy(parsed);
}

void instruccion_mov_out(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado, descomentar el return en el case correspondiente, y comentar en donde se modifica el program_counter
	iniciar_diccionario_registros(&proceso->registros);
	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	int tamanio_dato = parsed[1][0] == 'R' ? 16 : parsed[1][0] == 'E' ? 8
																	  : 4;
	char *dir_fisica = traducir_dir_logica(parsed[1], tamanio_dato);
	if (strcmp(dir_fisica, "SEG_FAULT") == 0)
	{
		// log_error(logger, "Error: segmentation fault");
		error_exit(EXIT_SEG_FAULT);
		return;
	}
	char *instruccion = string_from_format("%s %s %s", parsed[0], dir_fisica, parsed[2]);
	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion, free);
	log_trace(logger, "PID: %d - Instruccion traducida: %s", proceso->pid, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	t_instruccion *instruccion_proceso = generar_instruccion(proceso, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	// log_debug(logger, "1. Registro AX: %s", string_substring_until(proceso->registros.AX, 4));
	// log_debug(logger, "2. Registro AX: %s", string_substring_until(dictionary_get(registros, parsed[2]), 4));
	instruccion_proceso->tamanio_dato = tamanio_dato;
	instruccion_proceso->dato = dictionary_get(registros, parsed[2]);
	enviar_instruccion_con_dato(conexion_memoria, instruccion_proceso, MOV_OUT);
	int id_segmento = floor((double)atoi(parsed[1]) / config_get_int_value(config, "TAM_MAX_SEGMENTO"));
	char* valor = string_substring_until((char *)dictionary_get(registros, parsed[2]), tamanio_dato);
	log_info(logger, "PID: %u  Acción: ESCRIBIR - Segmento: %d - Dirección Física: %s - Valor: %s", proceso->pid, id_segmento, dir_fisica, valor);
	free(valor);
	free(dir_fisica);
	free(instruccion_proceso);
	dictionary_destroy(registros);
	// proceso->program_counter++;
}

void instruccion_i_o(char **parsed)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, IO_BLOCK);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_open(char **parsed)
{

	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_OPEN);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_close(char **parsed)
{

	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_CLOSE);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_seek(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_SEEK);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_read(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s %s %s", proceso->pid, parsed[0], parsed[1], parsed[2], parsed[3]);
	char *dir_fisica = traducir_dir_logica(parsed[2], atoi(parsed[3]));
	if (strcmp(dir_fisica, "SEG_FAULT") == 0)
	{
		// log_error(logger, "Error: segmentation fault");
		error_exit(EXIT_SEG_FAULT);
		return;
	}
	char *instruccion = string_from_format("%s %s %s %s", parsed[0], parsed[1], dir_fisica, parsed[3]);
	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion, free);
	log_trace(logger, "PID: %d - Instruccion traducida: %s", proceso->pid, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_READ);
	free(dir_fisica);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_write(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s %s %s", proceso->pid, parsed[0], parsed[1], parsed[2], parsed[3]);
	char *dir_fisica = traducir_dir_logica(parsed[2], atoi(parsed[3]));
	if (strcmp(dir_fisica, "SEG_FAULT") == 0)
	{
		// log_error(logger, "Error: segmentation fault");
		error_exit(EXIT_SEG_FAULT);
		return;
	}
	char *instruccion = string_from_format("%s %s %s %s", parsed[0], parsed[1], dir_fisica, parsed[3]);
	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion, free);
	log_trace(logger, "PID: %d - Instruccion traducida: %s", proceso->pid, (char *)list_get(proceso->instrucciones, proceso->program_counter));
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_WRITE);
	free(dir_fisica);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_f_truncate(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, F_TRUNCATE);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_wait(char **parsed)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, WAIT);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_signal(char **parsed)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, SIGNAL);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_create_segment(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parsed[0], parsed[1], parsed[2]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, CREATE_SEGMENT);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_delete_segment(char **parsed)
{
	// Para probar la función hay que descomentar lo comentado y descomentar el return en el case correspondiente

	log_info(logger, "PID: %d - Ejecutando: %s - %s", proceso->pid, parsed[0], parsed[1]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, DELETE_SEGMENT);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_yield(char **parsed)
{
	log_info(logger, "PID: %d - Ejecutando: %s", proceso->pid, parsed[0]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, READY);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void instruccion_exit(char **parsed)
{
	log_info(logger, "PID: %d - Ejecutando: %s", proceso->pid, parsed[0]);
	proceso->program_counter++;
	enviar_pcb(conexion_kernel, proceso, EXIT);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

void error_exit(op_code codigo)
{
	enviar_pcb(conexion_kernel, proceso, codigo);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
	// free(proceso);
}

char *traducir_dir_logica(char *direccion_logica, int tamanio_dato)
{
	int tam_max_segmento = config_get_int_value(config, "TAM_MAX_SEGMENTO");
	int num_segmento = floor((double)atoi(direccion_logica) / tam_max_segmento);
	int desplazamiento_segmento = atoi(direccion_logica) % tam_max_segmento;
	// int segmento = obtener_segmento(num_segmento);
	int base_segmento;
	int tamanio_segmento;
	log_debug(logger, "Numero de segmento: %d - Desplazamiento: %d", num_segmento, desplazamiento_segmento);
	t_segmento *segmento_actual;
	for (int i = 0; i < list_size(proceso->tabla_segmentos); i++)
	{
		segmento_actual = list_get(proceso->tabla_segmentos, i);
		if (segmento_actual->id_segmento == num_segmento)
		{
			base_segmento = segmento_actual->direccion_base;
			tamanio_segmento = segmento_actual->tam_segmento;
			break;
		}
	}
	log_debug(logger, "Base: %d - Tamanio: %d", base_segmento, tamanio_segmento);
	if (desplazamiento_segmento + tamanio_dato >= tamanio_segmento)
	{
		log_error(logger, "PID: %u - Error SEG_FAULT - Segmento: %d  - Offset: %d - Tamanio: %d", proceso->pid, segmento_actual->id_segmento, desplazamiento_segmento, segmento_actual->tam_segmento);
		return "SEG_FAULT";
	}
	int dir_fisica = base_segmento + desplazamiento_segmento;
	return string_from_format("%d", dir_fisica);
}
