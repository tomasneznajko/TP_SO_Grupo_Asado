#include "memoriaKernel.h"

void enviar_segmento(int pid, char* instruccion, t_list* tabla_segmentos){
	char** parsed = string_split(instruccion, " "); //Partes de la instruccion actual
	char* operacion = parsed[0];
    int id_segmento = atoi(parsed[1]);

    t_paquete *paquete = crear_paquete((strcmp(operacion, "CREATE_SEGMENT")==0) ? CREATE_SEGMENT : DELETE_SEGMENT);
    agregar_a_paquete(paquete,&(pid),sizeof(pid));
    agregar_a_paquete(paquete,&(id_segmento),sizeof(id_segmento));
    if(strcmp(operacion, "CREATE_SEGMENT") == 0){

        int tamanio_solicitado = atoi(parsed[2]); //El tamanio tambien se recibe de parametro
        agregar_a_paquete(paquete,&tamanio_solicitado,sizeof(tamanio_solicitado));
        enviar_paquete(paquete,conexion_memoria);
        log_info(logger,"Se solicita para PID: %u - Crear Segmento - Id: %d - Tamaño: %d",pid,id_segmento,tamanio_solicitado);
    }
    else{
        int tabla_segmentos_size = list_size(tabla_segmentos);
        agregar_a_paquete(paquete, &tabla_segmentos_size, sizeof(int));
        for (int i=0; i<tabla_segmentos_size; i++) {
            t_segmento* segmento_actual = (t_segmento*)list_get(tabla_segmentos, i);
            agregar_a_paquete(paquete, &(segmento_actual->id_segmento), sizeof(int));
            agregar_a_paquete(paquete, &(segmento_actual->tam_segmento), sizeof(int));
            agregar_a_paquete(paquete, &(segmento_actual->direccion_base), sizeof(int));
        }
        log_info(logger,"Se solicita para PID: %u - Eliminar Segmento - Id: %d",pid,id_segmento);
        enviar_paquete(paquete,conexion_memoria);
    }
    
    string_array_destroy(parsed);
    eliminar_paquete(paquete);
}


bool actualizo_proceso(pcb* proceso, t_list* lista) {
    t_list_iterator* buscador = list_iterator_create(lista);

    while (list_iterator_has_next(buscador)) {
        t_instruccion* proceso_actualizado = list_iterator_next(buscador);

        if (proceso_actualizado->pid == proceso->pid) {
            proceso->tabla_segmentos = proceso_actualizado->tabla_segmentos;
            list_iterator_destroy(buscador);
            return true;
        }
    }
    list_iterator_destroy(buscador);
    return false;
}

void actualizar_cola(t_queue* cola, t_queue* cola_copia, t_list* tablas_actualizadas) {
    while (!queue_is_empty(cola)) {
        if (actualizo_proceso((pcb*)queue_peek(cola), tablas_actualizadas)) {
            log_info(logger, "El proceso PID=%d fue actualizado correctamente", ((pcb*)queue_peek(cola))->pid);
        } else {
            log_error(logger, "El proceso PID=%d no fue encontrado en la lista con las tablas actualizadas", ((pcb*)queue_peek(cola))->pid);
        }
        queue_push(cola_copia, ((pcb*)queue_peek(cola)));
        queue_pop(cola);
    }
    cola = cola_copia;
    queue_destroy(cola_copia);
}


void actualizar_tablas(t_list* tablas_actualizadas) {
    t_queue* qready_copia = queue_create();
    t_queue* qblock_copia = queue_create();
    t_queue* qexec_copia = queue_create();

    actualizar_cola(qready, qready_copia, tablas_actualizadas);
    actualizar_cola(qblock, qblock_copia, tablas_actualizadas);
    actualizar_cola(qexec, qexec_copia, tablas_actualizadas);
}
