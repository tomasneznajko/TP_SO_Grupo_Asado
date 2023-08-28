#include "cliente.h"

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
            server_info->ai_socktype,
            server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		log_error(logger, "¡No se pudo conectar al servidor!");
		abort();
	}
	freeaddrinfo(server_info);
	send_handshake(socket_cliente);
	log_info(logger, "¡Conectado al servidor!");
	return socket_cliente;
}

void send_handshake(int socket_cliente)
{
	uint32_t handshake = 1;
	uint32_t result;
	send(socket_cliente, &handshake, sizeof(uint32_t), 0);
	if (recv(socket_cliente, &result, sizeof(uint32_t), MSG_WAITALL) == -1 || result == -1) {
		log_error(logger, "¡Protocolo incompatible con el servidor!");
		abort();
	}
}

void esperar_servidor(int conexion){
	pthread_t thread;
	int *socket_servidor = malloc(sizeof(int));
	*socket_servidor = conexion;
	pthread_create(&thread,
	               NULL,
	               (void*) atender_servidor,
	               socket_servidor);
	pthread_detach(thread);
}

void atender_servidor(int* socket_servidor){
	t_list *lista;
	t_instruccion* instruccion;
	while (1) {
		int cod_op = recibir_operacion(*socket_servidor);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(*socket_servidor);
				break;
			case PAQUETE:
				lista = recibir_paquete(*socket_servidor);
				log_info(logger, "Me llegaron los siguientes valores:");
				list_iterate(lista, (void*) iterator);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case MOV_IN:
				lista = recibir_paquete(*socket_servidor);
				instruccion = malloc(sizeof(t_instruccion));
				recibir_instruccion_con_dato(lista, instruccion);
				mov_in(instruccion);
				// destruir_diccionarios();
				interpretar_instrucciones();
				free(instruccion->instruccion);
				free(instruccion->dato);
				list_destroy_and_destroy_elements(instruccion->tabla_segmentos, free);
				list_destroy_and_destroy_elements(lista, free);
				free(instruccion);
				break;
			case OK:
				log_trace(logger, "TRACE: OK");
				// lista = recibir_paquete(*socket_servidor);
				proceso->program_counter++;
				interpretar_instrucciones();
				// list_destroy_and_destroy_elements(lista, free);
				log_trace(logger, "TRACE END: OK");
				break;
			case EXIT:
				error_exit(EXIT);
				// list_destroy_and_destroy_elements(lista, free);
				break;
			case -1:
				log_warning(logger, "El servidor se desconecto. Terminando conexion. Abortando sistema.");
				free(socket_servidor);
				abort();
				return;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

void liberar_conexion(int socket_cliente)
{
	if (socket_cliente != -1) {
	log_warning(logger, "Liberando conexion");
	close(socket_cliente);
	}
}
