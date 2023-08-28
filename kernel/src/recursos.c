#include "recursos.h"

t_list* recursos;

t_list* leerRecursos(t_config *config) {
    t_list* listaRecursos = list_create();

    char** recursosConfig =  config_get_array_value(config, "RECURSOS");
    char** instanciasConfig = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    int numRecursos = 0;
    while (recursosConfig[numRecursos]) {
        numRecursos++;
    }

    for(int i = 0; i<numRecursos ;i++) {
		Recurso* nuevoRecurso = malloc(sizeof(Recurso));
		strcpy(nuevoRecurso->nombre, recursosConfig[i]);
		nuevoRecurso->instancias = atoi(instanciasConfig[i]);
		nuevoRecurso->siguiente = NULL;
		nuevoRecurso->procesosBloqueados = queue_create();
		list_add(listaRecursos, nuevoRecurso);
	}

    string_array_destroy(recursosConfig);
    string_array_destroy(instanciasConfig);

	return listaRecursos;
}



void manejo_recursos(pcb* proceso) {

    char* instruccion = list_get(proceso->instrucciones, proceso->program_counter-1);

    char** parsed = string_split(instruccion, " "); //Partes de la instruccion actual

    char* operacion = parsed[0];
    char* recursoSolicitado = parsed[1];

    bool recursoExiste = false;

    Recurso* recursoActual = NULL;
    for (int i = 0; i < list_size(recursos); i++) {
        recursoActual = list_get(recursos, i);
        if (strcmp(recursoActual->nombre, recursoSolicitado) == 0) {
            recursoExiste = true;
            break;
        }
    }

    if (!recursoExiste) {
        log_error(logger, "PID: %d - %s de recurso no existente. Finalizando proceso", proceso->pid, operacion);
    	exec_a_exit("SUCCESS");
    } else {
        if (strcmp(operacion, "WAIT") == 0) {
            // Procesar operación WAIT
            recursoActual->instancias--;
            if (recursoActual->instancias < 0) {
            	queue_push(recursoActual->procesosBloqueados, proceso);
            	exec_a_block();
                log_info(logger, "PID: %d - Bloqueado por: %s", proceso->pid, recursoActual->nombre);
            }
            else{
                enviar_pcb(conexion_cpu, proceso, EXEC);
            }
            log_info(logger, "PID: %d - Wait: %s - Instancias: %d", proceso->pid, recursoActual->nombre, recursoActual->instancias);
        } else if (strcmp(operacion, "SIGNAL") == 0) {
            // Procesar operación SIGNAL
            recursoActual->instancias++;
            if (recursoActual->instancias <= 0) {
                block_a_ready(queue_peek(recursoActual->procesosBloqueados));
                queue_pop(recursoActual->procesosBloqueados);
                // Desbloquear primer proceso en la cola de bloqueados del recurso (si corresponde)
            }
            enviar_pcb(conexion_cpu, proceso, EXEC);
            log_info(logger, "PID: %d - Signal: %s - Instancias: %d", proceso->pid, recursoActual->nombre, recursoActual->instancias);
        }
    }
    string_array_destroy(parsed);
}

void destruir_recursos(t_list* recursos) {
    list_iterate(recursos, destruir_colas_recursos);
	list_destroy_and_destroy_elements(recursos, free);
}

void destruir_colas_recursos(void* recurso) {
    queue_destroy_and_destroy_elements(((Recurso*)recurso)->procesosBloqueados,free);
}



