#include "shared.h"

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void enviar_operacion(int socket_cliente, op_code codigo)
{
	// t_paquete *paquete = malloc(sizeof(t_paquete));
	int bytes = sizeof(int);

	void *a_enviar = malloc(bytes);

	memcpy(a_enviar, &codigo, bytes);
	// paquete->codigo_operacion = codigo;

	// paquete->buffer = malloc(sizeof(t_buffer));
	// paquete->buffer->size = 0;
	// paquete->buffer->stream = malloc(paquete->buffer->size);

	// int bytes = paquete->buffer->size + 2 * sizeof(int);

	// void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	// eliminar_paquete(paquete);
}

void enviar_mensaje(char *mensaje, int socket_cliente, op_code codigo)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_paquete *crear_paquete(op_code codigo)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);
	
	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_list *recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void delay(int milliseconds)
{
	t_temporal *clock = temporal_create();
	while (temporal_gettime(clock) < milliseconds)
		;
	temporal_destroy(clock);
}

void enviar_pcb(int conexion, pcb *proceso, op_code codigo)
{
	t_paquete *paquete = crear_paquete(codigo);

	agregar_a_paquete(paquete, &(proceso->pid), sizeof(unsigned int));

	int cantidad_instrucciones = list_size(proceso->instrucciones);
	agregar_a_paquete(paquete, &cantidad_instrucciones, sizeof(int));

	for (int i = 0; i < cantidad_instrucciones; i++)
	{
		char *instruccion = list_get(proceso->instrucciones, i);
		agregar_a_paquete(paquete, instruccion, strlen(instruccion) + 1);
	}

	agregar_a_paquete(paquete, &(proceso->program_counter), sizeof(int));

	agregar_a_paquete(paquete, proceso->registros.AX, 4);
	agregar_a_paquete(paquete, proceso->registros.BX, 4);
	agregar_a_paquete(paquete, proceso->registros.CX, 4);
	agregar_a_paquete(paquete, proceso->registros.DX, 4);
	agregar_a_paquete(paquete, proceso->registros.EAX, 8);
	agregar_a_paquete(paquete, proceso->registros.EBX, 8);
	agregar_a_paquete(paquete, proceso->registros.ECX, 8);
	agregar_a_paquete(paquete, proceso->registros.EDX, 8);
	agregar_a_paquete(paquete, proceso->registros.RAX, 16);
	agregar_a_paquete(paquete, proceso->registros.RBX, 16);
	agregar_a_paquete(paquete, proceso->registros.RCX, 16);
	agregar_a_paquete(paquete, proceso->registros.RDX, 16);
	
	int cantidad_segmentos = list_size(proceso->tabla_segmentos);
	agregar_a_paquete(paquete, &(cantidad_segmentos), sizeof(int));
	for (int i=0; i<cantidad_segmentos; i++) {
		t_segmento* segmento_actual = list_get(proceso->tabla_segmentos, i);
		agregar_a_paquete(paquete, &(segmento_actual->id_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->tam_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->direccion_base), sizeof(int));
	}

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void recibir_pcb(t_list *lista, pcb *proceso)
{
	int i = 0;
	memcpy(&(proceso->pid), list_get(lista, i++), sizeof(unsigned int));
	int cantidad_instrucciones;
	memcpy(&(cantidad_instrucciones), list_get(lista, i++), sizeof(int));
	
	proceso->instrucciones = list_slice_and_remove(lista, i, cantidad_instrucciones); //Aca hay una perdida grosa, de segmentation fault
	memcpy(&(proceso->program_counter), list_get(lista, i++), sizeof(int));
	memcpy(proceso->registros.AX, list_get(lista, i++), 4);
	memcpy(proceso->registros.BX, list_get(lista, i++), 4);
	memcpy(proceso->registros.CX, list_get(lista, i++), 4);
	memcpy(proceso->registros.DX, list_get(lista, i++), 4);
	memcpy(proceso->registros.EAX, list_get(lista, i++), 8);
	memcpy(proceso->registros.EBX, list_get(lista, i++), 8);
	memcpy(proceso->registros.ECX, list_get(lista, i++), 8);
	memcpy(proceso->registros.EDX, list_get(lista, i++), 8);
	memcpy(proceso->registros.RAX, list_get(lista, i++), 16);
	memcpy(proceso->registros.RBX, list_get(lista, i++), 16);
	memcpy(proceso->registros.RCX, list_get(lista, i++), 16);
	memcpy(proceso->registros.RDX, list_get(lista, i++), 16);

	int cantidad_segmentos;
	memcpy(&(cantidad_segmentos), list_get(lista, i++), sizeof(int));
	proceso->tabla_segmentos = list_create();
	for (int j=0; j<cantidad_segmentos; j++) {
		t_segmento* segmento_actual = malloc(sizeof(t_segmento));
		memcpy(&(segmento_actual->id_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->tam_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->direccion_base), list_get(lista, i++), sizeof(int));
		list_add(proceso->tabla_segmentos, segmento_actual);
	}

}

void enviar_instruccion(int conexion, t_instruccion* proceso, op_code codigo) {
	t_paquete *paquete = crear_paquete(codigo);

	agregar_a_paquete(paquete, &(proceso->pid), sizeof(unsigned int));
	agregar_a_paquete(paquete, proceso->instruccion, strlen(proceso->instruccion)+1);

	int cantidad_segmentos = list_size(proceso->tabla_segmentos);
	agregar_a_paquete(paquete, &(cantidad_segmentos), sizeof(int));
	for (int i=0; i<cantidad_segmentos; i++) {
		t_segmento* segmento_actual = list_get(proceso->tabla_segmentos, i);
		agregar_a_paquete(paquete, &(segmento_actual->id_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->tam_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->direccion_base), sizeof(int));
	}

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
	log_trace(logger, "TRACE: %d enviada", codigo);
}

void recibir_instruccion(t_list* lista, t_instruccion* proceso) {

	int i=0;

	memcpy(&(proceso->pid), list_get(lista, i++), sizeof(unsigned int));
	proceso->instruccion = (char*)list_remove(lista, i);

	int cantidad_segmentos;
	memcpy(&(cantidad_segmentos), list_get(lista, i++), sizeof(int));
	proceso->tabla_segmentos = list_create();
	for (int j=0; j<cantidad_segmentos; j++) {
		t_segmento* segmento_actual = malloc(sizeof(t_segmento));
		memcpy(&(segmento_actual->id_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->tam_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->direccion_base), list_get(lista, i++), sizeof(int));
		list_add(proceso->tabla_segmentos, segmento_actual);
	}
	log_trace(logger, "TRACE: Instrucción recibida - %s", proceso->instruccion);
}

void enviar_instruccion_con_dato(int conexion, t_instruccion* proceso, op_code codigo) {
	t_paquete *paquete = crear_paquete(codigo);

	agregar_a_paquete(paquete, &(proceso->pid), sizeof(unsigned int));
	agregar_a_paquete(paquete, proceso->instruccion, strlen(proceso->instruccion)+1);
	int cantidad_segmentos = list_size(proceso->tabla_segmentos);
	agregar_a_paquete(paquete, &(cantidad_segmentos), sizeof(int));
	for (int i=0; i<cantidad_segmentos; i++) {
		t_segmento* segmento_actual = list_get(proceso->tabla_segmentos, i);
		agregar_a_paquete(paquete, &(segmento_actual->id_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->tam_segmento), sizeof(int));
		agregar_a_paquete(paquete, &(segmento_actual->direccion_base), sizeof(int));
	}

	agregar_a_paquete(paquete, &(proceso->tamanio_dato), sizeof(int));
	agregar_a_paquete(paquete, proceso->dato, proceso->tamanio_dato);

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
	log_trace(logger, "TRACE: %d enviada", codigo);
}

void recibir_instruccion_con_dato(t_list* lista, t_instruccion* proceso) {

	int i=0;

	memcpy(&(proceso->pid), list_get(lista, i++), sizeof(unsigned int));
	proceso->instruccion = (char*)list_remove(lista, i);
	
	int cantidad_segmentos;
	memcpy(&(cantidad_segmentos), list_get(lista, i++), sizeof(int));
	proceso->tabla_segmentos = list_create();
	for (int j=0; j<cantidad_segmentos; j++) {
		t_segmento* segmento_actual = malloc(sizeof(t_segmento));
		memcpy(&(segmento_actual->id_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->tam_segmento), list_get(lista, i++), sizeof(int));
		memcpy(&(segmento_actual->direccion_base), list_get(lista, i++), sizeof(int));
		list_add(proceso->tabla_segmentos, segmento_actual);
	}

	memcpy(&(proceso->tamanio_dato), list_get(lista, i++), sizeof(int));
	proceso->dato = list_remove(lista, i);
	log_trace(logger, "TRACE: Instrucción recibida - %s", proceso->instruccion);
}

t_instruccion* generar_instruccion(pcb* proceso, char* instruccion) {
	t_instruccion* instruccion_proceso = malloc(sizeof(t_instruccion));
	instruccion_proceso->pid = proceso->pid;
	instruccion_proceso->instruccion = instruccion;
	instruccion_proceso->tabla_segmentos = proceso->tabla_segmentos;
	// instruccion_proceso->dato = NULL;
	return instruccion_proceso;
}

void enviar_tablas_segmentos(int conexion, t_list* tablas_segmentos, op_code codigo) {
	t_paquete *paquete = crear_paquete(codigo);

	int cantidad_tablas = list_size(tablas_segmentos);
	agregar_a_paquete(paquete, &cantidad_tablas, sizeof(int));

	t_list* cantidades = list_create();
	for (int i=0; i<cantidad_tablas; i++) {
		t_instruccion* tabla_actual = (t_instruccion*)list_get(tablas_segmentos, i);
		agregar_a_paquete(paquete, &(tabla_actual->pid), sizeof(unsigned int));
		int* cantidad_segmentos = malloc(sizeof(int));
		*cantidad_segmentos = list_size(tabla_actual->tabla_segmentos);
		list_add(cantidades, cantidad_segmentos);
		agregar_a_paquete(paquete, cantidad_segmentos, sizeof(int));
		for (int j=0; j<*cantidad_segmentos; j++) {
			t_segmento* segmento_actual = (t_segmento*)list_get(tabla_actual->tabla_segmentos, j);
			agregar_a_paquete(paquete, &(segmento_actual->id_segmento), sizeof(int));
			agregar_a_paquete(paquete, &(segmento_actual->tam_segmento), sizeof(int));
			agregar_a_paquete(paquete, &(segmento_actual->direccion_base), sizeof(int));
		}
	}
	
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
	list_destroy_and_destroy_elements(cantidades, free);
	log_trace(logger, "TRACE: %d enviada", codigo);
}

t_list* recibir_tablas_segmentos(t_list* lista) {

	int i=0;
	t_list* lista_tablas = list_create();

	int cantidad_tablas;
	memcpy(&(cantidad_tablas), list_get(lista, i++), sizeof(int));

	for (int j=0; j<cantidad_tablas; j++) {
		t_instruccion* tabla_actual = malloc(sizeof(t_instruccion));
		memcpy(&(tabla_actual->pid), list_get(lista, i++), sizeof(unsigned int));
		tabla_actual->tabla_segmentos = list_create();
		int cantidad_segmentos;
		memcpy(&(cantidad_segmentos), list_get(lista, i++), sizeof(int));
		for (int k=0; k<cantidad_segmentos; k++) {
			t_segmento* segmento_actual = malloc(sizeof(t_segmento));
			memcpy(&(segmento_actual->id_segmento), list_get(lista, i++), sizeof(int));
			memcpy(&(segmento_actual->tam_segmento), list_get(lista, i++), sizeof(int));
			memcpy(&(segmento_actual->direccion_base), list_get(lista, i++), sizeof(int));
			list_add(tabla_actual->tabla_segmentos, segmento_actual);
		}
		list_add(lista_tablas, tabla_actual);
	}
	return lista_tablas;

}

void replace_r_with_0(char *line)
{
	char *pos = strchr(line, '\r');
	if (pos)
	{
		*pos = '\0';
	}
}
