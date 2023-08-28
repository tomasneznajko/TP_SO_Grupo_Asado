#include"server.h"

int iniciar_servidor(char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;
	// struct addrinfo *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
	                         servinfo->ai_socktype,
	                         servinfo->ai_protocol);

	int reuse = 1;
	if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		log_error(logger, "Error al configurar SO_REUSEADDR");
		abort();
	}

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	if (listen(socket_servidor, SOMAXCONN) == -1) {
		log_error(logger, "¡No se pudo iniciar el servidor!");
		abort();
	}

	freeaddrinfo(servinfo);
	log_info(logger, "Servidor listo para recibir al cliente");

	return socket_servidor;
}

void recv_handshake(int socket_cliente)
{
	uint32_t handshake;
	uint32_t resultOk = 0;
	uint32_t resultError = -1;
	recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);
	if(handshake == 1)
	   send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
	else {
	   send(socket_cliente, &resultError, sizeof(uint32_t), 0);
	}
}

void esperar_cliente(int socket_servidor){
	while (1) {
	   int *socket_cliente = malloc(sizeof(int));
	   *socket_cliente = accept(socket_servidor, NULL, NULL);
	   recv_handshake(*socket_cliente);
	   atender_cliente(socket_cliente);
	}
}

void atender_cliente(int* socket_cliente)
{
	t_list *lista;
	t_instruccion* proceso;
	char *instruccion;
	char *nombreArchivo;
	char **porqueria;
	size_t tamanioAtruncar;
	size_t punteroAArchivo;
	size_t cantidadBytesALeer;
	int direccionFisica;
	void *informacionLeidaOEscrita;
	while (1) 
	{
		int cod_op = recibir_operacion(*socket_cliente);
		switch (cod_op) 
		{
			case MENSAJE:
				recibir_mensaje(*socket_cliente);
				break;
			case PAQUETE:
				lista = recibir_paquete(*socket_cliente);
				log_info(logger, "Me llegaron los siguientes valores:");
				list_iterate(lista, (void*) iterator);
				list_clean_and_destroy_elements(lista, free);
				break;
			case F_OPEN:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				instruccion = proceso->instruccion;

				porqueria=string_split(instruccion, " ");
				nombreArchivo= porqueria[1];


				if(abrirArchivo(nombreArchivo,vectorDePathsPCBs,cantidadPaths))
				{
					enviar_instruccion(*socket_cliente,proceso,F_OPEN_OK);
				}
				else
				{
					enviar_instruccion(*socket_cliente,proceso,EL_ARCHIVO_NO_EXISTE_PAAAAAAA);
				}

				string_array_destroy(porqueria);
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista,free);
				free(proceso);
				break;
			case F_CREATE:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				instruccion = proceso->instruccion;

				porqueria=string_split(instruccion, " ");
				nombreArchivo= porqueria[1];
				if(crearArchivo(nombreArchivo, config_get_string_value(config,"PATH_FCB"), &vectorDePathsPCBs, &cantidadPaths))
				{
					enviar_instruccion(*socket_cliente, proceso ,F_OPEN_OK);
				}
				else
				{
					log_error(logger,"No se pudo crear el archivo pibe. Algo se rompio zarpado");
				}
				 
				string_array_destroy(porqueria);
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista,free);
				free(proceso);
				break;
			case F_TRUNCATE:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				instruccion = proceso->instruccion;
				porqueria=string_split(instruccion, " ");
				nombreArchivo = porqueria[1];
				tamanioAtruncar = atoi(porqueria[2]);
				if(truncarArchivo(nombreArchivo, config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, tamanioAtruncar))
				{
					enviar_instruccion(*socket_cliente, proceso , YA_SE_TERMINO_LA_TRUNCACION);
				}
				else
				{
					log_error(logger,"No se pudo truncar el archivo. CAGAAAAMOSSSSS");
				}
				string_array_destroy(porqueria);
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista,free);
				free(proceso);
				break;
			case F_READ:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				instruccion = proceso->instruccion;
				porqueria=string_split(instruccion, " ");
				nombreArchivo = porqueria[1];
				punteroAArchivo = atoi(porqueria[2]);
				cantidadBytesALeer = atoi(porqueria[3]);
				direccionFisica = atoi(porqueria[4]);
				informacionLeidaOEscrita = leerArchivo(nombreArchivo,punteroAArchivo,cantidadBytesALeer,direccionFisica);
				proceso->dato = informacionLeidaOEscrita;
				proceso->tamanio_dato = cantidadBytesALeer;
				
				// Enviar mensaje a memoria y mandarle la merca
				
				enviar_instruccion_con_dato(conexion_memoria,proceso,F_READ);

				//recibir el ok en memoria
				if (recibir_operacion(conexion_memoria) == OK)
				{
					free(proceso->instruccion);
					free(proceso->dato);
					list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
					list_destroy_and_destroy_elements(lista,free);
					free(proceso);
					lista = recibir_paquete(conexion_memoria);
					proceso = malloc(sizeof(t_instruccion));
					recibir_instruccion(lista,proceso);
				
					//Envio el ok
				
					enviar_instruccion(*socket_cliente, proceso , MEMORIA_DIJO_QUE_PUDO_ESCRIBIR_JOYA);
					string_array_destroy(porqueria);
					free(proceso->instruccion);
					list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
					list_destroy_and_destroy_elements(lista,free);
					free(proceso);
					break;
				}
				log_error(logger,"CORRE PIBE, SE FUE TODO AL CARAJO MEMORIA NO PUDO ESCRIBIR");
				string_array_destroy(porqueria);
				free(proceso->instruccion);
				free(proceso->dato);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista,free);
				free(proceso);
				break;
			case F_WRITE:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				
				//Le pido a memoria que me pase lo que hay en la direccion fisica
				enviar_instruccion(conexion_memoria,proceso,F_WRITE);
				
				if(recibir_operacion(conexion_memoria) == ACA_TENES_LA_INFO_GIIIIIIL)
				{
					free(proceso->instruccion);
					list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
					list_destroy_and_destroy_elements(lista,free);
					free(proceso);
					lista = recibir_paquete(conexion_memoria);
					proceso = malloc(sizeof(t_instruccion));
					recibir_instruccion_con_dato(lista,proceso);
					instruccion = proceso->instruccion;
					porqueria=string_split(instruccion, " ");
					nombreArchivo = porqueria[1];
					punteroAArchivo = atoi(porqueria[2]);
					cantidadBytesALeer = atoi(porqueria[3]);
					direccionFisica = atoi(porqueria[4]);
					informacionLeidaOEscrita = proceso->dato;

					//Lo escribo en el archivo
					if (escribirArchivo(nombreArchivo,punteroAArchivo,cantidadBytesALeer,direccionFisica,informacionLeidaOEscrita))
					{
						enviar_instruccion(*socket_cliente, proceso , SE_PUDO_ESCRIBIR_EL_ARCHIVO);
					}
					else
					{
						log_error(logger,"No se pudo escribir el archivo. CAGAMOS MAL PAAAAAAAAAA");
					}
					string_array_destroy(porqueria);
					free(proceso->instruccion);
					free(proceso->dato);
					list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
					list_destroy_and_destroy_elements(lista,free);
					free(proceso);
					break;
				}
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista,free);
				free(proceso);
				break;
			case -1:
				log_warning(logger, "El cliente se desconecto. Terminando conexion");
				free(socket_cliente);
				return;
			case -2:
				log_warning(logger, "Abortando sistema desde kernel.");
				free(socket_cliente);
				abort();
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

void liberar_servidor(int *socket_servidor)
{
	log_warning(logger, "Liberando servidor");
	close(*socket_servidor);
	free(socket_servidor);
}
