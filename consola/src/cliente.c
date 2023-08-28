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

void atender_kernel(int conexion_kernel){
	t_list *lista;
	while (1) {
		int cod_op = recibir_operacion(conexion_kernel);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(conexion_kernel);
				break;
			case PAQUETE:
				lista = recibir_paquete(conexion_kernel);
				log_info(logger, "Me llegaron los siguientes valores:");
				list_iterate(lista, (void*) iterator);
				list_clean_and_destroy_elements(lista, free);
				break;
			case EXIT:
				log_info(logger, "Proceso %d finalizado. Terminando conexion", process_getpid());
				return;
			case -1:
				log_warning(logger, "El servidor se desconecto. Terminando conexion");
				return;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

void liberar_conexion(int conexion_kernel)
{
	if (conexion_kernel != -1) {
	log_warning(logger, "Liberando conexion");
	close(conexion_kernel);
	}
}
