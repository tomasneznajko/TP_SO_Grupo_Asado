#include"server.h"

int conexion_kernel = -1;
int conexion_cpu = -1;
int conexion_filesystem = -1;

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
		log_error(logger, "Error al configurar SO_REUSEADDR \n");
		abort();
	}

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	if (listen(socket_servidor, SOMAXCONN) == -1) {
		log_error(logger, "¡No se pudo iniciar el servidor! \n");
		abort();
	}

	freeaddrinfo(servinfo);
	log_info(logger, "Servidor listo para recibir al cliente \n");

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
	   pthread_t thread;
	   int *socket_cliente = malloc(sizeof(int));
	   *socket_cliente = accept(socket_servidor, NULL, NULL);
	   recv_handshake(*socket_cliente);
	   pthread_create(&thread,
	                  NULL,
	                  (void*) atender_cliente,
	                  socket_cliente);
	   pthread_detach(thread);
	}
}

void atender_cliente(int* socket_cliente){
	t_list *lista;
	t_instruccion* proceso;
	void *informacionALeerOEscribir;
	int tamanio_informacion;
	char * instruccion;
	char** parsed;
	int dir_fisica;
	t_paquete* paquete;
	unsigned int pid;
	int id_segmento;
	// int id_seg;
	// int desp;
	// char* valor_mem;
//	char* nuevo_valor;

	while (1) {
		int cod_op = recibir_operacion(*socket_cliente);
		log_debug(logger, "Enum: %d", cod_op);
		switch (cod_op) {
			case KERNEL:
				conexion_kernel = *socket_cliente;
				break;
			case CPU:
				conexion_cpu = *socket_cliente;
				break;
			case FILESYSTEM:
				conexion_filesystem = *socket_cliente;
				break;
			case MENSAJE:
				recibir_mensaje(*socket_cliente);
				break;

			case PAQUETE:
				lista = recibir_paquete(*socket_cliente);

				log_info(logger, "Me llegaron las siguientes instrucciones:");
				list_iterate(lista, (void*) iterator);

				//manejo_instrucciones (lista, *socket_cliente);

				list_clean_and_destroy_elements(lista, free);

				break;

				//kernel
			case CREATE_PROCESS:
				log_trace(logger, "TRACE: CREATE_PROCESS");
				log_trace(logger, "TRACE: DELETE_PROCESS");
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				proceso->tabla_segmentos = iniciar_proceso(proceso->pid);
				log_debug(logger, "Cantidad de segmentos: %d", list_size(proceso->tabla_segmentos));
				log_debug(logger, "tamanio segmento 0: %d", ((t_segmento*)list_get(proceso->tabla_segmentos, 0))->tam_segmento);
				enviar_instruccion(conexion_kernel, proceso, CREATE_PROCESS_OK);
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				log_trace(logger, "TRACE: CREATE_PROCESS_OK enviado");
				break;
			case DELETE_PROCESS:
				log_trace(logger, "TRACE: DELETE_PROCESS");
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				finalizar_proceso(proceso);
				enviar_instruccion(conexion_kernel,proceso,DELETE_PROCESS_OK);
				free(proceso->instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				break;
			case CREATE_SEGMENT:
				lista = recibir_paquete(*socket_cliente);
				pid = *(unsigned int*)list_get(lista, 0);
				id_segmento = *(int*)list_get(lista, 1);
				int tamanio_segmento = *(int*)list_get(lista, 2);
				int resultado = crear_segmento(pid, tamanio_segmento, id_segmento);
				switch (resultado) {
					case -1:
						// COMPACTACION
						// paquete = crear_paquete(EXIT_OUT_OF_MEMORY);
						// agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
						// agregar_a_paquete(paquete, &id_segmento, sizeof(int));
						// enviar_paquete(paquete, conexion_kernel);
						// eliminar_paquete(paquete);
						enviar_operacion(conexion_kernel, COMPACTACION);
						break;
					case -2:
						// OUT_OF_MEMORY
						paquete = crear_paquete(EXIT_OUT_OF_MEMORY);
						agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
						agregar_a_paquete(paquete, &id_segmento, sizeof(int));
						enviar_paquete(paquete, conexion_kernel);
						eliminar_paquete(paquete);
						break;
					default:
						paquete = crear_paquete(CREATE_SEGMENT_OK);
						agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
						agregar_a_paquete(paquete, &id_segmento, sizeof(int));
						agregar_a_paquete(paquete, &tamanio_segmento, sizeof(int));
						agregar_a_paquete(paquete, &resultado, sizeof(int));
						enviar_paquete(paquete, conexion_kernel);
						eliminar_paquete(paquete);

				}
				list_destroy_and_destroy_elements(lista, free);
				break;
			case DELETE_SEGMENT:
				lista = recibir_paquete(*socket_cliente);
				pid = *(unsigned int*)list_get(lista, 0);
				id_segmento = *(int*)list_get(lista, 1);
				int tamanio_tabla_segmentos = *(int*)list_get(lista, 2);
				int post_tamanio_tabla_segmentos = tamanio_tabla_segmentos-1;
				eliminar_segmento(pid, id_segmento);
				for (int i = 0; i < tamanio_tabla_segmentos; i++)
				{
					int j = 3+4*i;
					if ((*(int*)list_get(lista, j))==id_segmento) {
						list_remove_and_destroy_element(lista, j, free);
						list_remove_and_destroy_element(lista, j, free);
						list_remove_and_destroy_element(lista, j, free);
						list_replace_and_destroy_element(lista, 2, &post_tamanio_tabla_segmentos, free);
						break;
					}
				}
				log_trace(logger, "TRACE END");
				paquete = crear_paquete(DELETE_SEGMENT_OK);
				agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
				agregar_a_paquete(paquete, &id_segmento, sizeof(int));
				agregar_a_paquete(paquete, &post_tamanio_tabla_segmentos, sizeof(int));
				for (int i=0; i<post_tamanio_tabla_segmentos; i++) {
					int j = 3+4*i;
					agregar_a_paquete(paquete, list_get(lista, j), sizeof(int));
					agregar_a_paquete(paquete, list_get(lista, j+1), sizeof(int));
					agregar_a_paquete(paquete, list_get(lista, j+2), sizeof(int));
				}
				enviar_paquete(paquete, conexion_kernel);
				eliminar_paquete(paquete);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case F_READ:
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion_con_dato(lista,proceso);
				informacionALeerOEscribir = proceso->dato;
				instruccion = proceso->instruccion;
				parsed = string_split(instruccion," ");
				dir_fisica = atoi(parsed[1]);
				tamanio_informacion = proceso->tamanio_dato;
				escribir_memoria(dir_fisica,informacionALeerOEscribir,tamanio_informacion);
				log_info(logger, "PID: %u - Accion: ESCRIBIR - Direccion fisica: %d - Tamanio: %d - Origen: FS", proceso->pid, dir_fisica, tamanio_informacion);
				//Avisarle a filesystem que se escribio joya
				enviar_instruccion(conexion_filesystem,proceso,OK);
				free(informacionALeerOEscribir);
				string_array_destroy(parsed);
				free(instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				break;
			case F_WRITE:
				log_trace(logger, "F_WRITE");
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);
				// informacionALeerOEscribir = proceso->dato;
				instruccion = proceso->instruccion;
				parsed = string_split(instruccion," ");
				dir_fisica = atoi(parsed[1]);
				tamanio_informacion = atoi(parsed[2]);
				informacionALeerOEscribir = leer_memoria(dir_fisica,tamanio_informacion);
				log_info(logger, "PID: %u - Accion: LEER - Direccion fisica: %d - Tamanio: %d - Origen: FS", proceso->pid, dir_fisica, tamanio_informacion);
				proceso->tamanio_dato=tamanio_informacion;
				proceso->dato=informacionALeerOEscribir;
				enviar_instruccion_con_dato(conexion_filesystem,proceso,ACA_TENES_LA_INFO_GIIIIIIL);
				free(informacionALeerOEscribir);
				string_array_destroy(parsed);
				free(instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				break;
				//cpu
			case MOV_IN: //leer cpu
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista,proceso);

				instruccion = proceso->instruccion;
				parsed = string_split(instruccion," ");
				dir_fisica = atoi(parsed[2]);
				//----------------------------------------------------
				tamanio_informacion = parsed[1][0]=='R' ? 16 : parsed[1][0]=='E' ? 8 : 4;
				informacionALeerOEscribir = leer_memoria(dir_fisica, tamanio_informacion);
				log_info(logger, "PID: %u - Accion: LEER - Direccion fisica: %d - Tamanio: %d - Origen: CPU", proceso->pid, dir_fisica, tamanio_informacion);

				proceso->dato = informacionALeerOEscribir;
				proceso->tamanio_dato = tamanio_informacion;
				enviar_instruccion_con_dato(conexion_cpu, proceso, MOV_IN);
				free(informacionALeerOEscribir);
				string_array_destroy(parsed);
				free(instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				break;
			case MOV_OUT: //escribir
				// parsed [1] -> dir fisica
				// parsed [2] -> registro
				lista = recibir_paquete(*socket_cliente);
				proceso = malloc(sizeof(t_instruccion));
				recibir_instruccion_con_dato(lista,proceso);

				instruccion = proceso->instruccion;
				parsed = string_split(instruccion," ");
				dir_fisica = atoi(parsed[1]);

				informacionALeerOEscribir = proceso->dato;	//warning: assignment to ‘char *’ from incompatible pointer type ‘char **’ [-Wincompatible-pointer-types]
				tamanio_informacion = proceso->tamanio_dato;
				escribir_memoria(dir_fisica, informacionALeerOEscribir, tamanio_informacion);
				log_info(logger, "PID: %u - Accion: ESCRIBIR - Direccion fisica: %d - Tamanio: %d bytes - Origen: CPU", proceso->pid, dir_fisica, tamanio_informacion);

				// enviar_instruccion(conexion_cpu, proceso, OK);
				enviar_operacion(conexion_cpu, OK);

				string_array_destroy(parsed);
				free(informacionALeerOEscribir);
				free(instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(proceso->tabla_segmentos, free);
				free(proceso);
				log_trace(logger, "TRACE END: MOV_OUT");

				break;
			case COMPACTACION:
				// lista = recibir_paquete(*socket_cliente);
				t_list* segmentos_compactados = compactar_segmentos();
				t_list* lista_tablas = list_create();
				for (int i=0; i<list_size(segmentos_compactados); i++) {
					segmento* segmento_actual = (segmento*)list_get(segmentos_compactados, i);
					t_instruccion* tabla = NULL;
					for (int j=0; i<list_size(lista_tablas); i++) {
						t_instruccion* tabla_actual = (t_instruccion*)list_get(lista_tablas, j);
						if (tabla_actual->pid==segmento_actual->pid) {
							tabla = tabla_actual;
							break;
						}
					}
					if (!tabla) {
						tabla = malloc(sizeof(t_instruccion));
						tabla->pid = segmento_actual->pid;
						tabla->tabla_segmentos = list_create();
						list_add(lista_tablas, tabla);
					}
					t_segmento* tsegmento = malloc(sizeof(t_segmento));
					tsegmento->id_segmento = segmento_actual->id;
					tsegmento->tam_segmento = segmento_actual->tam_segmento;
					tsegmento->direccion_base = segmento_actual->direccion_base;
					list_add(tabla->tabla_segmentos, tsegmento);
				}
				enviar_tablas_segmentos(conexion_kernel, lista_tablas, COMPACTACION_OK);
				list_destroy_and_destroy_elements(lista_tablas,free);
				// Falta limpiar
				// list_destroy_and_destroy_elements(lista, free);
				break;
			case -1:
				log_warning(logger, "El cliente se desconecto. Terminando conexion \n");
				free(socket_cliente);
				return;
			default:
				log_warning(logger,"Operacion desconocida \n");
				break;
		}
	}
}

void liberar_servidor(int *socket_servidor)
{
	log_warning(logger, "Liberando servidor \n");
	close(*socket_servidor);
	free(socket_servidor);
}
