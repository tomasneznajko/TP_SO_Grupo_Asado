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
	               (void*) socket_servidor);
	pthread_detach(thread);
}

void atender_servidor(int* socket_servidor){
	t_list *lista;
	pthread_t thread;
	char* instruccion;
	char* numero; //No se me ocurre otra forma de hacerlo
	// char* numero_din;
	char** parsed;
	char* instruccionQueMando;
	t_instruccion* laCosaQueMando;
	Archivo *archivoQueUso;
	t_segmento* segmento;
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
			case READY:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				exec_a_ready();
				list_destroy_and_destroy_elements(lista, free);
				break;
			case IO_BLOCK:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				pthread_create(&thread, NULL, (void*) io_block, NULL);
				pthread_detach(thread);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case WAIT:
				log_trace(logger, "TRACE: WAIT");
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				// instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				manejo_recursos(((pcb*)queue_peek(qexec)));
				// pthread_create(&thread, NULL, (void*) manejo_recursos, queue_peek(qexec));
				// pthread_detach(thread);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case SIGNAL:
				log_trace(logger, "TRACE: SIGNAL");
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				// instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				manejo_recursos(((pcb*)queue_peek(qexec)));
				list_destroy_and_destroy_elements(lista, free);
				break;
			case F_OPEN:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				if(abrirArchivoKernel(((pcb*)queue_peek(qexec)), instruccion))
				{

					//PUEDO HACER ESTO?????
					laCosaQueMando = generar_instruccion(queue_peek(qexec), instruccion);
					// laCosaQueMando->pid=((pcb*)queue_peek(qexec))->pid;
					// laCosaQueMando->instruccion=instruccion;
					enviar_instruccion(conexion_filesystem,laCosaQueMando,F_OPEN);
					free(laCosaQueMando);
					// enviar_pcb(conexion_cpu, (pcb*)queue_peek(qexec), EXEC);
				}
				list_destroy_and_destroy_elements(lista, free);
				log_trace(logger, "TRACE: F_OPEN");
				break;
			case F_OPEN_OK:
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				enviar_pcb(conexion_cpu, (pcb*)queue_peek(qexec), EXEC);
				
				free(laCosaQueMando->instruccion);
				list_destroy_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);
				log_trace(logger, "TRACE: OK");
				break;
			case EL_ARCHIVO_NO_EXISTE_PAAAAAAA:
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				//PUEDO HACER ESTO????? X2
				enviar_instruccion(conexion_filesystem,laCosaQueMando,F_CREATE);

				free(laCosaQueMando->instruccion);
				list_destroy_and_destroy_elements(lista, free);
				list_destroy_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				free(laCosaQueMando);
				log_trace(logger, "TRACE: F_CREATE");
				break;
			case F_CLOSE:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				cerrarArchivoKernel(((pcb*)queue_peek(qexec)), instruccion);
				enviar_pcb(conexion_cpu, (pcb*)queue_peek(qexec), EXEC);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case F_SEEK:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				buscarEnArchivo(((pcb*)queue_peek(qexec)), instruccion);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case F_TRUNCATE:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);

				//PUEDO HACER ESTO????? X2
				laCosaQueMando = generar_instruccion(queue_peek(qexec), instruccion);
				// laCosaQueMando->pid=((pcb*)queue_peek(qexec))->pid;
				// laCosaQueMando->instruccion=malloc(strlen(instruccion)+1);
				// strcpy(laCosaQueMando->instruccion,instruccion);
				parsed = string_split(instruccion, " ");
				log_info(logger, "PID: %d - Archivo: %s - Tamaño: %s", laCosaQueMando->pid, parsed[1], parsed[2]);
				exec_a_block();
				enviar_instruccion(conexion_filesystem,laCosaQueMando,F_TRUNCATE);
				free(laCosaQueMando);
				string_array_destroy(parsed);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case YA_SE_TERMINO_LA_TRUNCACION:
				log_trace(logger, "TRACE: YA_SE_TERMINO_LA_TRUNCACION");
				char* cola_block_str = queue_iterator(qblock);
				log_debug(logger, "Cola block: [%s]", cola_block_str);
				free(cola_block_str);
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				log_debug(logger, "YA_SE_TERMINO_LA_TRUNCACION: PID %u", laCosaQueMando->pid);
				block_a_ready(queue_seek(qblock, laCosaQueMando->pid));
				free(laCosaQueMando->instruccion);
				list_destroy_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);
				break;
			case F_READ:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				parsed = string_split(instruccion, " ");

				//esto deberia devolver el archivo que voy a usar
				archivoQueUso = estoDevuelveUnArchivo(((pcb*)queue_peek(qexec)), instruccion);

				numero = string_itoa(archivoQueUso->puntero);
				// numero_din = malloc(strlen(numero)+1);
				// strcpy(numero_din, numero);
				//le meto el numero (como string) a la instruccion para mandarselo a file system
				// strcat(instruccion," ");
				// strcat(instruccion, numero_din);
				instruccionQueMando = string_from_format("%s %s", instruccion, numero);
				log_debug(logger, "DEBUG: F_READ - Instruccion %s - Numero %s", instruccionQueMando, numero);

				//PUEDO HACER ESTO????? X2
				laCosaQueMando = generar_instruccion(queue_peek(qexec), instruccionQueMando);
				// laCosaQueMando->pid=((pcb*)queue_peek(qexec))->pid;
				// laCosaQueMando->instruccion=malloc(strlen(instruccion)+1);
				// strcpy(laCosaQueMando->instruccion,instruccion);
				enviar_instruccion(conexion_filesystem,laCosaQueMando,F_READ);

				sem_wait(sem_escrituraLectura);
				contadorDeEscrituraOLectura ++;
				sem_post(sem_escrituraLectura);

				log_info(logger, "PID: %u - Leer Archivo: %s - Puntero: %s - Direccion Memoria %s - Tamanio %s", ((pcb*)queue_peek(qexec))->pid, parsed[1], numero, parsed[2], parsed[3]);
				


				exec_a_block();
				string_array_destroy(parsed);
				free(numero);
				free(instruccionQueMando);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);

				break;
			case MEMORIA_DIJO_QUE_PUDO_ESCRIBIR_JOYA:
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				block_a_ready(queue_seek(qblock, laCosaQueMando->pid));
				sem_wait(sem_escrituraLectura);
				contadorDeEscrituraOLectura --;
				sem_post(sem_escrituraLectura);
				free(laCosaQueMando->instruccion);
				list_destroy_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);
				break;
			case F_WRITE:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				parsed = string_split(instruccion, " ");

				//esto deberia devolver el archivo que voy a usar
				archivoQueUso = estoDevuelveUnArchivo(((pcb*)queue_peek(qexec)), instruccion);

				char* numero = string_itoa(archivoQueUso->puntero);

				//le meto el numero (como string) a la instruccion para mandarselo a file system
				instruccionQueMando = string_from_format("%s %s", instruccion, numero);
				log_debug(logger, "DEBUG: F_WRITE - Instruccion %s - Numero %s", instruccionQueMando, numero);
				// log_debug(logger, "Instrucción de la lista: %s", (char*)list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1));
				//PUEDO HACER ESTO????? X2
				laCosaQueMando = generar_instruccion(queue_peek(qexec), instruccionQueMando);
				// laCosaQueMando->pid=((pcb*)queue_peek(qexec))->pid;
				// laCosaQueMando->instruccion=malloc(strlen(instruccion)+1);
				// strcpy(laCosaQueMando->instruccion,instruccion);
				enviar_instruccion(conexion_filesystem,laCosaQueMando,F_WRITE);

				sem_wait(sem_escrituraLectura);
				contadorDeEscrituraOLectura ++;
				sem_post(sem_escrituraLectura);

				log_info(logger, "PID: %u - Escribir Archivo: %s - Puntero: %s - Direccion Memoria %s - Tamaño %s", ((pcb*)queue_peek(qexec))->pid, parsed[1], numero, parsed[2], parsed[3]);
				exec_a_block();

				string_array_destroy(parsed);
				free(numero);
				free(instruccionQueMando);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);

				break;
			case SE_PUDO_ESCRIBIR_EL_ARCHIVO:
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				block_a_ready(queue_seek(qblock, laCosaQueMando->pid));

				sem_wait(sem_escrituraLectura);
				contadorDeEscrituraOLectura --;
				sem_post(sem_escrituraLectura);

				free(laCosaQueMando->instruccion);
				list_destroy_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				list_destroy_and_destroy_elements(lista, free);
				free(laCosaQueMando);
				break;
			case CREATE_SEGMENT:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				enviar_segmento(((pcb*)queue_peek(qexec))->pid,instruccion, ((pcb*)queue_peek(qexec))->tabla_segmentos);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case CREATE_SEGMENT_OK:
				lista = recibir_paquete(*socket_servidor); //Debería enviar la base/id_segmento + el tipo de resultado que se obtuvo: 0-> Todo bien, 1->No hay espacio, 2->Requiere compactacion
				segmento = malloc(sizeof(t_segmento));
				memcpy(&(segmento->id_segmento), list_get(lista,1), sizeof(int));
				memcpy(&(segmento->tam_segmento), list_get(lista,2), sizeof(int));
				memcpy(&(segmento->direccion_base), list_get(lista,3), sizeof(int));
				list_add(((pcb*)queue_peek(qexec))->tabla_segmentos, segmento);
				log_info(logger, "Se ha creado exitosamente para PID: %u - Segmento - Id: %d con la base %d en memoria", ((pcb*)queue_peek(qexec))->pid,*((int*)list_get(lista,1)),*((int*)list_get(lista,3)));
				enviar_pcb(conexion_cpu, (pcb*)queue_peek(qexec), EXEC);
				list_destroy_and_destroy_elements(lista,free);
				break;
			case DELETE_SEGMENT:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, queue_peek(qexec));
				instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				enviar_segmento(((pcb*)queue_peek(qexec))->pid,instruccion, ((pcb*)queue_peek(qexec))->tabla_segmentos);
				list_destroy_and_destroy_elements(lista, free);
				break;
			case DELETE_SEGMENT_OK:
				lista = recibir_paquete(*socket_servidor);
				list_clean_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				int tamanio_tabla_segmentos = *(int*)list_get(lista, 2);
				for (int i=0; i<tamanio_tabla_segmentos; i++) {
					int j = 3+4*i;
					t_segmento* segmento_actual = malloc(sizeof(t_segmento));
					segmento_actual->id_segmento = *(int*)list_get(lista, j);
					segmento_actual->tam_segmento = *(int*)list_get(lista, j+1);
					segmento_actual->direccion_base = *(int*)list_get(lista, j+2);
					list_add(((pcb*)queue_peek(qexec))->tabla_segmentos, segmento_actual);
				}

				log_info(logger, "Se ha creado exitosamente para PID: %u - Segmento - Id: %d",((pcb*)queue_peek(qexec))->pid,*(int*)list_get(lista, 1));

				enviar_pcb(conexion_cpu,((pcb*)queue_peek(qexec)),EXEC);
				list_destroy_and_destroy_elements(lista,free);
				break;
			case COMPACTACION:
				log_info(logger,"Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>");
				while (1) {
					sem_wait(sem_escrituraLectura);
					if (contadorDeEscrituraOLectura == 0) {
						break;
					}
					sem_post(sem_escrituraLectura);
				}
				enviar_operacion(conexion_memoria,COMPACTACION);
				break;
			case COMPACTACION_OK:
				lista = recibir_paquete(*socket_servidor);
				t_list* lista_tablas = recibir_tablas_segmentos(lista);
				actualizar_tablas(lista_tablas);
				char* instruccion = list_get(((pcb*)queue_peek(qexec))->instrucciones, ((pcb*)queue_peek(qexec))->program_counter-1);
				log_info(logger,"Se finalizó el proceso de compactación, por lo que realizamos nuevamente la solicitud de creación del segmento");
				enviar_segmento(((pcb*)queue_peek(qexec))->pid,instruccion,((pcb*)queue_peek(qexec))->tabla_segmentos); //Volvemos a solicitar la creacion del segmento
				list_destroy_and_destroy_elements(lista,free);
				list_destroy_and_destroy_elements(lista_tablas,free);
				break;
			case CREATE_PROCESS_OK:
				lista = recibir_paquete(*socket_servidor);
				log_trace(logger, "TRACE: CREATE_PROCESS_OK");
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				// Esto rompia?
				list_destroy(((pcb*)queue_peek(qnew))->tabla_segmentos);
				((pcb*)queue_peek(qnew))->tabla_segmentos = list_duplicate(laCosaQueMando->tabla_segmentos); //Actualiza la tabla de segmentos
				log_debug(logger, "Tamanio de segmento: %d", ((t_segmento*)list_get(((pcb*)queue_peek(qnew))->tabla_segmentos, 0))->tam_segmento);
				pthread_create(&thread,
	                  NULL,
	                  (void*) new_a_ready,
	                  NULL); //Memoria dice que el proceso está listo
				pthread_detach(thread);
				free(laCosaQueMando->instruccion);
				list_destroy(laCosaQueMando->tabla_segmentos);
				list_destroy_and_destroy_elements(lista,free);
				// list_clean_and_destroy_elements(laCosaQueMando->tabla_segmentos,free);
				free(laCosaQueMando);
				break;
			case EXIT:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, (pcb*)queue_peek(qexec));
				exec_a_exit("SUCCESS");
				list_destroy_and_destroy_elements(lista, free);
				break;
			case EXIT_SEG_FAULT:
				lista = recibir_paquete(*socket_servidor);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->instrucciones, free);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexec))->tabla_segmentos, free);
				recibir_pcb(lista, (pcb*)queue_peek(qexec));
				exec_a_exit("SEG_FAULT");
				list_destroy_and_destroy_elements(lista,free);
				break;
			case EXIT_OUT_OF_MEMORY:
				lista = recibir_paquete(*socket_servidor); //Debería enviar la base/id_segmento + el tipo de resultado que se obtuvo: 0-> Todo bien, 1->No hay espacio, 2->Requiere compactacion
				log_error(logger,"Out of memory: No se encontró espacio para el segmento id %d en memoria, por lo que se finaliza el proceso en ejecucion", *((int*)list_get(lista,1)));
            	exec_a_exit("OUT_OF_MEMORY");
				list_destroy_and_destroy_elements(lista,free);
				break;
			case DELETE_PROCESS_OK:
				lista = recibir_paquete(*socket_servidor);
				laCosaQueMando = malloc(sizeof(t_instruccion));
				recibir_instruccion(lista, laCosaQueMando);
				list_destroy_and_destroy_elements(((pcb*)queue_peek(qexit))->tabla_segmentos, free);
				((pcb*)queue_peek(qexit))->tabla_segmentos = list_duplicate(laCosaQueMando->tabla_segmentos); //Actualiza la tabla de segmentos
				finalizar_proceso(laCosaQueMando->instruccion); //Memoria dice que el proceso fue liberado
				free(laCosaQueMando->instruccion);
				list_destroy(laCosaQueMando->tabla_segmentos);
				list_destroy_and_destroy_elements(lista,free);
				free(laCosaQueMando);
				break;
			case -1:
				log_warning(logger, "El servidor se desconecto. Terminando conexion. Abortando sistema.");
				enviar_operacion(conexion_cpu, -2);
				enviar_operacion(conexion_filesystem, -2);
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
