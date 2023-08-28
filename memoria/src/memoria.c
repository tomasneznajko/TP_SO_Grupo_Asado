#include "memoria.h"

char *ip_memoria;

//archivo_configuracion config_mem;
void* memoria_usuario;
t_list* tabla_segmentos_total;
t_list* huecos;
t_config *config;

// completar: !!!
// posibles cambios: ???


int main(int argc, char **argv)
{
	if (argc < 2)
	{
		return EXIT_FAILURE;
	}

	logger = iniciar_logger("./memoria.log", "Memoria");
	log_info(logger,"Log iniciado");

	// CARGAR CONFIG
	config = iniciar_config(argv[1]);
	//cargar_config(config);

	iniciar_memoria();

	// SERIVIDOR
	int socket_servidor = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));
	esperar_cliente(socket_servidor);



	list_destroy_and_destroy_elements(tabla_segmentos_total, free);
	list_destroy_and_destroy_elements(huecos, free);
		free(memoria_usuario);
		//list_destroy(huecos);
	//terminar_memoria(logger, config, socket_servidor);
	log_trace(logger, "Aquí termina");
	log_debug(logger, "Aquí termina");
	log_info(logger, "Aquí termina");
	log_warning(logger, "Aquí termina");
	log_error(logger, "Aquí termina");
	return EXIT_SUCCESS;
}

//-------------------PROCESOS------------------------------------------------------------------------------------------------------------------------------------------------------------
//Crea los segmentos del proceso. La cantidad de segmentos esta dada por el config.
t_list* iniciar_proceso(unsigned int pid)
{
	t_list* segmentos = list_create();
	segmento* seg;
	t_segmento* t_seg = malloc(sizeof(t_segmento));
	seg = list_get(tabla_segmentos_total, 0);
	t_seg->id_segmento = seg->id;
	t_seg->tam_segmento = seg->tam_segmento;
	t_seg->direccion_base = seg->direccion_base;

	list_add(segmentos, t_seg);

	/*for (int var = 0; var < (config_get_int_value(config, "CANT_SEGMENTOS")-1); ++var)
	{
		anterior = list_get(tabla_segmentos_total, (list_size(tabla_segmentos_total)-1));
		seg->direccion_base = anterior->direccion_base;
		seg->direccion_limite = anterior->direccion_base;
		seg->id = anterior->id + var;
		seg->tam_segmento = 0;
		seg->pid = pid;
		list_add(tabla_segmentos_total, seg);

	}*/

	// segmentos = obtener_segmentos_PID(pid);

	log_info (logger, "Creacion de proceso PID: %u",pid);

	return segmentos;
}

//Elimina los segmentos del proceso. Filtra la lista de segmentos y los elimina de la lista de segmentos.
void finalizar_proceso(t_instruccion* pcb_proceso)
{
	log_debug(logger, "Tamanio segmento 0: %d", ((segmento*)list_get(tabla_segmentos_total, 0))->tam_segmento);
	// t_list* segmentos_proc = list_create();
	t_list* segmentos_proc;
	segmentos_proc	= obtener_segmentos_PID(pcb_proceso->pid); 									//Lista filtrada
	segmento *seg_actual;
	for (int var = 1; var < list_size(segmentos_proc); ++var)											//Recorrre la lista filtrada
		{
			seg_actual = list_get(segmentos_proc, var);														//Toma el segmento actual del for
			eliminar_segmento (pcb_proceso->pid,seg_actual->id);												//Elimina el segmento de la lista de segmentos total.
			// free(seg_actual);
		}
	list_clean_and_destroy_elements(pcb_proceso->tabla_segmentos, free);
	// list_destroy_and_destroy_elements(segmentos_proc, free);
	list_destroy(segmentos_proc);
	log_debug(logger, "Total segmentos: %d", list_size(tabla_segmentos_total));
	log_debug(logger, "Tamanio segmento 0: %d", ((segmento*)list_get(tabla_segmentos_total, 0))->tam_segmento);
	log_info (logger, "Eliminacion de proceso PID: %u",pcb_proceso->pid);
}

//Funcion que recorre la lista y retorna los id de los segmentos de un proceso
t_list* obtener_segmentos_PID(unsigned int pid)
{
    t_list* segmentosPorPID = list_create();
    // segmento* seg = malloc(sizeof(segmento));
	segmento* seg;

    for (int i = 0; i < list_size(tabla_segmentos_total); i++)											//Recorre la lista de segmentos
    {
        seg = (segmento*)list_get(tabla_segmentos_total, i);														//Toma un segmento
        if ((seg->pid == 0) || (seg->pid == pid)) {																			//Ve si pertenece al proceso
            list_add(segmentosPorPID,seg);																//Si pertenece, lo agrega a la lista de segmentos del proceso
        }
    }
    // free(seg);
    return segmentosPorPID;
}

//-------------------INICIALIZACION DE MEMORIA------------------------------------------------------------------------------------------------------------------------------------------------------------
//Se le asignan a los elementos de un struct global la informacion del config

/*void cargar_config(t_config *archivo)
{
	log_info (logger, "entro a cargar_config" );
	char* algoritmo = config_get_string_value(config, "ALGORITMO_ASIGNACION");
	char* primera_letra = string_substring_until(algoritmo, 1);
	algoritmo_asignacion alg = cambiar_enum_algoritmo (primera_letra);									//Quiero que el algoritmo de asignacion sea un enum porque uso un switch en crear segmento.

	config_mem.ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	config_mem.puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	config_mem.tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	config_mem.tam_segmento_0 = config_get_int_value(config, "TAM_SEGMENTO_0");
	config_mem.cant_segmentos = config_get_int_value(config, "CANT_SEGMENTOS");
	config_mem.retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	config_mem.retardo_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	config_mem.algoritmo = alg;
}*/

//Cambia el string del algoritmo de asignacion del config a un enum propio

algoritmo_asignacion cambiar_enum_algoritmo (char* letra)
{
	algoritmo_asignacion algoritmo;

    if (string_equals_ignore_case(letra, "F")) {
    	algoritmo = FIRST;
    }
    else if (string_equals_ignore_case(letra, "B")) {
    	algoritmo = BEST;
    }
    else if (string_equals_ignore_case(letra, "W")) {
    	algoritmo = WORST;
    }

    return algoritmo;
}

void iniciar_memoria()
{
	memoria_usuario = malloc(config_get_int_value(config, "TAM_MEMORIA"));
	tabla_segmentos_total = list_create();
	huecos = list_create();

	segmento *segmento_0 = malloc(sizeof(segmento));										//Segmento 0 compartido por todos los procesos
	segmento_0->pid = 0;
	segmento_0->id = 0;
	segmento_0->tam_segmento = config_get_int_value(config, "TAM_SEGMENTO_0");
	segmento_0->direccion_base = 0;
	segmento_0->direccion_limite = (config_get_int_value(config, "TAM_SEGMENTO_0")) - 1;

	list_add(tabla_segmentos_total, segmento_0);
	log_info(logger, "Memoria inicializada.");
}

//-------------------MANEJO DE SEGMENTOS-----------------------------------------------------------------------------------
//Crea UN segmento segun el algoritmo de asignacion del config si hay espacio en memoria. Si no hay espacio en memoria, solicita una compactacion o le informa al kernel que no hay espacio disponible: int crear_segmento(unsigned int pid, int tamanio_seg, int id_seg)
int crear_segmento(unsigned int pid, int tamanio_seg, int id_seg)
{
	int sumatoria;

	// char* primera_letra = malloc(2* sizeof(char));
	char* primera_letra;
	primera_letra = string_substring_until(config_get_string_value(config, "ALGORITMO_ASIGNACION"), 1);
	algoritmo_asignacion algoritmo = cambiar_enum_algoritmo (primera_letra);
	free(primera_letra);
	int dir_base;


	if (hay_espacio_disponible(tamanio_seg) /*&& hay_segmentos_disponibles(pid)*/)															//Primero se fija si hay espacio disponible en memoria
	{
			// mutex
			switch (algoritmo)
			{
				case FIRST:
					dir_base = first_fit(pid, tamanio_seg, id_seg);
					break;
				case BEST:
					dir_base = best_fit(pid, tamanio_seg, id_seg);
					break;
				case WORST:
					dir_base = worst_fit(pid, tamanio_seg, id_seg);
					break;
			}
		}
	else if (!hay_espacio_disponible(tamanio_seg))																								//No hay espacio disponible en memoria,
	{
		sumatoria = sumatoria_huecos();																			//suma el espacio de los huecos

		if (sumatoria >= tamanio_seg)																	//porque los segmentos no estan compactados (hay espacio pero disperso).
		{
			log_info(logger, "Solicitud de Compactacion");
			// INFORMAR KERNEL COMPACTAR !!!
			return -1;
		}

		else																							//porque no hay mas espacio (si se compactan los segmentos igual no hay espacio).
		{
			log_info(logger, "No hay mas memoria");
			// FALTA DE ESPACIO LIBRE KERNEL !!!
			return -2;
		}
	}
/*	else if(!hay_segmentos_disponibles(pid))
	{
		log_info(logger, "No hay mas memoria");
		// FALTA DE ESPACIO LIBRE KERNEL !!!
		return -3;
	}*/

	return dir_base;
}
int hay_segmentos_disponibles(unsigned int pid)
{
	t_list* listaPID = obtener_segmentos_PID(pid);
	if(list_size(listaPID)<config_get_int_value(config, "CANT_SEGMENTOS")){
		return 1;
	}
	else{
		return 0;
	}

}
//Busca el segmento por su id y lo elimina
void eliminar_segmento(unsigned int pid, int id)
{
	segmento *seg ;
	segmento * hueco = malloc(sizeof(segmento));
	int ady;

	for (int i = 0; i < list_size(tabla_segmentos_total); i++)
	{
		seg = list_get(tabla_segmentos_total, i);
		if(seg->pid == pid){
			if (seg->id == id)																				//Encuentra el segmento.
			{
				//seg = list_get(tabla_segmentos_total, id);
				log_info(logger, "PID: %u - Eliminar Segmento: %d - Base: %d - Tamanio: %d", pid, id, seg->direccion_base, seg->tam_segmento);
				list_remove_and_destroy_element(tabla_segmentos_total, i,free);														//Lo borra de la tabla de segmentos.
				ady = agrupar_huecos(seg->direccion_base, seg->direccion_limite);							//Si tiene huecos aledanios, los agrupa.
				if (!ady)																				//Si no los tiene,
				{
					memcpy(hueco, seg, sizeof(segmento));
					list_add(huecos, hueco);													//crea el hueco.
				}
				break;
			}
		}
	}
			//free(hueco);
			/*break;
		}
	}*/
}

//Si tiene huecos aledanios los agrupa y devuelve 1, sino devuelve 0
int agrupar_huecos(int base, int limite)
{

	segmento *hueco_izquierdo = NULL;
	int index_izq;
	segmento *hueco_derecho = NULL ;
	int index_der;

	segmento *hueco ;
	 segmento *hueco_agrupado;

	/*for (int i = 0; i < list_size(huecos); i++)
	{
		hueco = list_get(huecos, i);printf("1 sf? \n");
		if ((hueco->direccion_limite + 1) == base)
		{
			hueco_izquierdo = hueco;
			index_izq = i;
			break;
		}
	}

	for (int j = 0; j < list_size(huecos); j++)
	{
		printf("%d \n",j);
		hueco = list_get(huecos, j);
		if ((hueco->direccion_base - 1) == limite)
		{
			hueco_derecho = hueco;
			index_der = j;printf("index der: %d \n",index_der);
			break;
		}
	}

	if (hueco_izquierdo != NULL && hueco_derecho != NULL)
	{
		list_remove(huecos, index_izq);
		printf("2 sf? \n");
		list_remove(huecos, index_der);
		printf("3 sf? \n");
		hueco_agrupado = malloc(sizeof(segmento));
		printf("4 sf? \n");
		hueco_agrupado->direccion_base = hueco_derecho->direccion_limite;
		hueco_agrupado->direccion_limite = hueco_izquierdo->direccion_limite;
		printf("5 sf? \n");
		list_add(huecos, hueco_agrupado);
		printf("6 sf? \n");

		return 1;
	}*/
	int i = 0;
    while (i < list_size(huecos))
    {
        hueco = list_get(huecos, i);
        if ((hueco->direccion_limite + 1) == base)
        {
            hueco_izquierdo = hueco;
            index_izq = i;
            break;
        }
        i++;
    }

    int j = 0;
    while (j < list_size(huecos))
    {
        hueco = list_get(huecos, j);
        if ((hueco->direccion_base - 1) == limite)
        {
            hueco_derecho = hueco;
            index_der = j;
            printf("index der: %d\n", index_der);
            break;
        }
        j++;
    }

    if (hueco_izquierdo != NULL && hueco_derecho != NULL)
    {
    	list_remove_and_destroy_element(huecos, index_izq,free);
        if (index_der > index_izq)
        {
            // Restamos 1 al índice si se eliminó un elemento antes
            index_der--;
            hueco_agrupado = malloc(sizeof(segmento));
            hueco_agrupado->direccion_base = hueco_derecho->direccion_limite;
            hueco_agrupado->direccion_limite = hueco_izquierdo->direccion_limite;
            list_add(huecos, hueco_agrupado);
        }
        list_remove_and_destroy_element(huecos, index_der,free);
        return 1;
    }

	return 0;
}


//Suma los espacios de los huecos
int sumatoria_huecos()
{
	int sumatoria = 0;
	segmento *seg;

	for (int i = 0; i < list_size(huecos); i++)
	{
		seg = list_get(huecos, i);
		sumatoria += seg->tam_segmento;
	}
	return sumatoria;
}

//Si hay espacio disponible devuelve 1, sino devuelve 0
int hay_espacio_disponible(int tam_segmento)
{
	int espacio;
	segmento *segmento_actual;

	if (list_size(tabla_segmentos_total) > 0) {
	        segmento_actual = list_get(tabla_segmentos_total, list_size(tabla_segmentos_total) - 1);
	        espacio = segmento_actual->direccion_limite;
	    }

	if(espacio+tam_segmento<config_get_int_value(config, "TAM_MEMORIA") ){
		return 1;
	}

	return 0;
}

void modificar_hueco (int base, int limite)
{

	segmento* hueco /*= malloc(sizeof(segmento))*/;
	segmento* hueco_modificado = malloc(sizeof(segmento));

	////printf("base %d \n", base);
	////printf("limite %d \n", limite);

	for (int i = 0; i < list_size(huecos); i++)
	{
		hueco = list_get(huecos, i);

		if (hueco->direccion_base == base && hueco->direccion_limite == limite)
		{
			list_remove_and_destroy_element(huecos, i,free);
			break;
		}
		else if (hueco->direccion_base == base && hueco->direccion_limite > limite )
		{
			hueco_modificado->direccion_limite = hueco->direccion_limite;
			hueco_modificado->direccion_base = limite;
			hueco_modificado->pid = 0;
			//list_replace(huecos, i, hueco_modificado);
			list_remove_and_destroy_element(huecos, i,free);
			list_add_in_index(huecos,i, hueco_modificado);

		}
	}
}

void eliminar_hueco(int base, int limite)
{
	segmento* hueco;

	for (int i = 0; i < list_size(huecos); i++)
		{
			hueco = list_get(huecos, i);
			if (hueco->direccion_limite == base && hueco->direccion_limite == limite)
			{
				list_remove_and_destroy_element(huecos, i, free);
				break;
			}
		}
}


// Busca el primer hueco disponible desde el comienzo de memoria
int first_fit(unsigned int pid_proceso, int tam, int id_seg)
{
	segmento *segmento_actual;
	segmento *segmento_siguiente;
	int espacio_libre;
	segmento *nuevo_segmento= malloc(sizeof(segmento));
	int nueva_dir_base;
	int tamSeg0 = config_get_int_value(config, "TAM_SEGMENTO_0");
		//nueva_dir_base = tamSeg0 + 1;
	int noHayHuecos = -1;


	if(list_size(tabla_segmentos_total)==1)
	{
		nuevo_segmento->direccion_base= tamSeg0 ;
		nuevo_segmento->direccion_limite= nuevo_segmento->direccion_base + tam - 1;
		nuevo_segmento->pid = pid_proceso;
		nuevo_segmento->tam_segmento = tam;
		nuevo_segmento->id= id_seg;
		list_add(tabla_segmentos_total, nuevo_segmento);
		log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
	}

	else
	{
		for (int i = 0; i < list_size(tabla_segmentos_total); i++)
		{
			segmento_actual = list_get(tabla_segmentos_total, i);
	//		//printf ("%u", segmento_actual->pid);// Recorre todos los segmentos

			if (i + 1 < list_size(tabla_segmentos_total))																	// Ve si existe otro segmento
			{
				segmento_siguiente = list_get(tabla_segmentos_total, i + 1);
				////printf("base sig: %d \n", segmento_siguiente->direccion_base);
				////printf("lim actual: %d \n", segmento_actual->direccion_limite);

				espacio_libre = segmento_siguiente->direccion_base - (segmento_actual->direccion_limite); 	// Si existe, calcula el espacio entre ambos
				espacio_libre = espacio_libre - 1;

				////printf("espacio libre: %d \n", espacio_libre);

				if (espacio_libre >= tam)																// Si entra el segmento en el espacio, lo agrega
				{
					//printf ("if (espacio_libre >= tam) \n");

					nuevo_segmento->pid = pid_proceso;
					nuevo_segmento->id = id_seg;
					nuevo_segmento->tam_segmento = tam;
					nuevo_segmento->direccion_base = segmento_actual->direccion_limite + 1;
					nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam - 1 ;
					list_add_in_index(tabla_segmentos_total, i + 1, nuevo_segmento);
					log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
					modificar_hueco(nuevo_segmento->direccion_base, nuevo_segmento->direccion_limite);
					noHayHuecos = 1;
					break;
				}
			}
		}

		if (noHayHuecos == -1){
			//int id_seg_nuevo = list_size(tabla_segmentos_total) - 1;
			//printf ("noHayHuecos == -1 \n");
			nueva_dir_base = (segmento_actual->direccion_limite) + 1;
			//log_info(logger,"limite seg actual: %d",segmento_actual->direccion_limite);
			nuevo_segmento->tam_segmento = tam;
			nuevo_segmento->direccion_base = nueva_dir_base;
			nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam ;
			nuevo_segmento->pid= pid_proceso;

			nuevo_segmento->id = id_seg;

			list_add(tabla_segmentos_total,nuevo_segmento);
			
			log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, segmento_actual->id, nuevo_segmento->direccion_base, tam);
		}
	}
	return nuevo_segmento->direccion_base;

}

// Busca el hueco mas chico donde entre el proceso
int best_fit(unsigned int pid_proceso, int tam, int id_seg)
{
	int segmento_asignado = -1;
	int mejor_ajuste = 2147483647;
	segmento *segmento_actual;
	segmento *segmento_siguiente;
	int espacio_libre;
	segmento *nuevo_segmento = malloc(sizeof(segmento));
	int nueva_dir_base;
	int tamSeg0 = config_get_int_value(config, "TAM_SEGMENTO_0");

	if(list_size(tabla_segmentos_total)==1)
	{
		nuevo_segmento->direccion_base= tamSeg0;
		nuevo_segmento->direccion_limite= nuevo_segmento->direccion_base + tam - 1;
		nuevo_segmento->pid = pid_proceso;
		nuevo_segmento->tam_segmento = tam;
		nuevo_segmento->id = id_seg;

		list_add(tabla_segmentos_total, nuevo_segmento);
		log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
	}

	else
	{
		for (int i = 0; i < list_size(tabla_segmentos_total); i++)
		{

			segmento_actual = list_get(tabla_segmentos_total, i);


			if (i + 1 < list_size(tabla_segmentos_total))
			{
				segmento_siguiente = list_get(tabla_segmentos_total, i + 1);
				espacio_libre = segmento_siguiente->direccion_base - (segmento_actual->direccion_limite + 1);

				if (espacio_libre >= tam && espacio_libre < mejor_ajuste)								//Si entra el segmento y si el espacio es menor al menor espacio, lo agrega.
				{
					segmento_asignado = segmento_actual->id + 1;
					mejor_ajuste = espacio_libre;
					nueva_dir_base = (segmento_actual->direccion_limite) + 1;
					//segmento_asignado = i + 1 ;
				}
			}
		}

		if (segmento_asignado != -1)
		{
			nuevo_segmento->pid = pid_proceso;
			nuevo_segmento->tam_segmento = tam;
			nuevo_segmento->direccion_base = nueva_dir_base;
			nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam - 1;
			nuevo_segmento->id = id_seg;
			// Insertar el nuevo segmento en la lista de memoria después del segmento anterior al segmento asignado
			list_add_in_index(tabla_segmentos_total, segmento_asignado, nuevo_segmento);
			log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
			modificar_hueco (nuevo_segmento->direccion_base,nuevo_segmento->direccion_limite);
			//eliminar_hueco(nuevo_segmento->direccion_base, nuevo_segmento->direccion_limite);
		}
		else
		{
			//si no hay espacio entre segmentos o es el segundo segmento
			nueva_dir_base = (segmento_actual->direccion_limite);
			log_info(logger,"limite seg actual: %d",segmento_actual->direccion_limite);
			nuevo_segmento->pid = pid_proceso;
			nuevo_segmento->tam_segmento = tam;
			nuevo_segmento->direccion_base = nueva_dir_base;
			nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam ;
			//int id_seg_nuevo = list_size(tabla_segmentos_total) - 1;
			nuevo_segmento->id= id_seg;

			list_add(tabla_segmentos_total,nuevo_segmento);
			
			log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
		}

	}
	return nuevo_segmento->direccion_base;
}

// Buscar el hueco más grande que pueda contener el nuevo segmento
int worst_fit(unsigned int pid_proceso, int tam, int id_seg)
{
	int segmento_asignado = -1;
	int mejor_ajuste = 0;
	segmento *segmento_actual;
	int espacio_libre;
	segmento *segmento_siguiente;
	int nueva_dir_base;
	segmento *nuevo_segmento = malloc(sizeof(segmento));
	int tamSeg0 = config_get_int_value(config, "TAM_SEGMENTO_0");

	if(list_size(tabla_segmentos_total) == 1)
		{
			nuevo_segmento->direccion_base= tamSeg0 ;
			nuevo_segmento->direccion_limite= nuevo_segmento->direccion_base + tam - 1;
			nuevo_segmento->pid = pid_proceso;
			nuevo_segmento->tam_segmento = tam;
			nuevo_segmento->id = id_seg;

			list_add(tabla_segmentos_total, nuevo_segmento);
			log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
		}

		else
		{
			for (int i = 0; i < list_size(tabla_segmentos_total); i++)
			{
				segmento_actual = list_get(tabla_segmentos_total, i);

				if (i + 1 < list_size(tabla_segmentos_total))
				{
					segmento_siguiente = list_get(tabla_segmentos_total, i + 1);
					espacio_libre = segmento_siguiente->direccion_base - (segmento_actual->direccion_limite + 1);

					if (espacio_libre >= tam && espacio_libre > mejor_ajuste)								//Si entra el segmento y si el espacio es mayor al mayor espacio, lo agrega.
					{

						segmento_asignado =  1;
						mejor_ajuste = espacio_libre;
						nueva_dir_base = 1 + segmento_actual->direccion_limite ;
						//segmento_asignado = i + 1;
					}
				}
			}

			if (segmento_asignado != -1)
			{
				// Crear el nuevo segmento y establecer sus direcciones
				nuevo_segmento->id = id_seg;
				nuevo_segmento->tam_segmento = tam;
				nuevo_segmento->direccion_base = nueva_dir_base; // list_get(memoria_usuario, segmento_asignado - 1)->direccion_limite + 1;
				nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam ;

				// Insertar el nuevo segmento en la lista de memoria después del segmento anterior al segmento asignado
				list_add_in_index(tabla_segmentos_total, segmento_asignado, nuevo_segmento);
				log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
				modificar_hueco(nuevo_segmento->direccion_base, nuevo_segmento->direccion_limite);
			}

			else
			{
				//si no hay espacio entre segmentos o es el segundo segmento
				nueva_dir_base = (segmento_actual->direccion_limite) + 1;
				nuevo_segmento->tam_segmento = tam;
				nuevo_segmento->direccion_base = nueva_dir_base;
				nuevo_segmento->direccion_limite = nuevo_segmento->direccion_base + tam ;
				nuevo_segmento->id=id_seg;
				list_add(tabla_segmentos_total,nuevo_segmento);
				
				log_info(logger, "PID: %u - Crear Segmento: %d - Base: %d - Tamanio: %d", pid_proceso, nuevo_segmento->id, nuevo_segmento->direccion_base, tam);
			}
		}
	return nuevo_segmento->direccion_base;
}


void* leer_memoria(int direccion, size_t tamanio)
{
	void* valorLeido = malloc(tamanio);
	delay (config_get_int_value(config, "RETARDO_MEMORIA"));
	memcpy(valorLeido, memoria_usuario + direccion, tamanio);
	return valorLeido;
	//segmento *seg= list_get(tabla_segmentos_total, id_buscado);
	//void* valorLeido = NULL;
	/*if (seg != NULL) {
		log_info(logger,"pid del segmento %u", seg->pid);
		direccion = seg->direccion_base + desp;
		log_info(logger,"direccion %d", direccion);
		valorLeido = malloc(tamanio);
		delay (config_get_int_value(config, "RETARDO_COMPACTACION"));
		memcpy(valorLeido, memoria_usuario + direccion, tamanio);*/

	/*
	for (int i = 0; i < list_size(tabla_segmentos_total); i++)
	{
		seg = list_get(tabla_segmentos_total, i);

		if (seg->pid == pid_buscado){
			if (seg->id == id_buscado){
				direccion = seg->direccion_base + desp;
				valorLeido = malloc(tamanio);
				delay (config_get_int_value(config, "RETARDO_COMPACTACION"));
				memcpy(valorLeido, memoria_usuario + direccion, tamanio);
				return valorLeido;
			}
		}
	}*/
	//free(seg);
	//}
	/*else {
		*log_info(logger,"No existe en memoria");
	}*/
	return valorLeido;
}

void escribir_memoria(int direccion, void* nuevo_valor, size_t tamanio)
{

	//segmento* seg = list_get(tabla_segmentos_total, id_buscado);

	delay (config_get_int_value(config, "RETARDO_MEMORIA"));
	memcpy(memoria_usuario + direccion, nuevo_valor, tamanio);

	/*if (seg != NULL) {
		log_info(logger,"pid del segmento buscado: %u", seg->pid);
		direccion = seg->direccion_base + desp;
		delay (config_get_int_value(config, "RETARDO_COMPACTACION"));
		memcpy(memoria_usuario + direccion, nuevo_valor, tamanio);*/


/*	for (int i = 0; i < list_size(tabla_segmentos_total); i++)
		{
			seg = list_get(tabla_segmentos_total, i);

			if(seg->pid == pid_buscado){
				if (seg->id == id_buscado) {
					direccion = seg->direccion_base + desp;
					delay (config_get_int_value(config, "RETARDO_COMPACTACION"));
					memcpy(memoria_usuario + direccion, nuevo_valor, tamanio);
				}
			}
//		free(seg);
//				free(seg);		//	}
			}
	else{
		log_info(logger,"Segmento no encontrado.");
	}*/
}

t_list* compactar_segmentos() {
    t_list* segmentos_compactados = list_create();
    segmento* segm = malloc(sizeof(segmento));
    // segmento* seg;
    int direccion_base_actual = 0;
    int tam_segmento = 0;
    int antigua_direccion_base = 0;

    for (int i = 0; i < list_size(tabla_segmentos_total); i++) {
        segmento* seg = list_get(tabla_segmentos_total, i);
        memcpy(segm, seg, sizeof(segmento));
        antigua_direccion_base = seg->direccion_base;

        //Cambios segmento
        tam_segmento = seg->direccion_limite - seg->direccion_base + 1;
        seg->direccion_base = direccion_base_actual;
        seg->direccion_limite = seg->direccion_base + tam_segmento - 1;
        list_add(segmentos_compactados, segm);
        list_remove_and_destroy_element(tabla_segmentos_total, i,free);

        //Cambios en memoria usuario
        int desplazamiento = direccion_base_actual - antigua_direccion_base;
        if(desplazamiento != 0){
        	memmove(memoria_usuario + direccion_base_actual, memoria_usuario + antigua_direccion_base, tam_segmento);
        }
		log_info(logger, "PID: %u - Segmento: %d - Base: %d - Tamanio: %d", segm->pid, segm->id, segm->direccion_base, segm->tam_segmento);
        direccion_base_actual = seg->direccion_limite + 1;
    }

   list_destroy_and_destroy_elements(tabla_segmentos_total, free);

    // Actualizar la tabla de segmentos total con los segmentos compactados
    tabla_segmentos_total = segmentos_compactados;
    return tabla_segmentos_total;
}


void terminar_memoria(t_log *logger, t_config *config, int socket)
{
	list_destroy_and_destroy_elements(tabla_segmentos_total, free);
	list_destroy_and_destroy_elements(huecos, free);
	free(memoria_usuario);

	log_info(logger, "Finalizando memoria");
	if (logger != NULL)
	{
		log_destroy(logger);
	}
	if (config != NULL)
	{
		config_destroy(config);
	}

	liberar_servidor(&socket);
}

/*
  ,-.       _,---._ __  / \
 /  )    .-'       `./ /   \
(  (   ,'            `/    /|
 \  `-"             \'\   / |
  `.              ,  \ \ /  |
   /`.          ,'-`----Y   |
  (            ;        |   '
  |  ,-.    ,-'         |  /
  |  | (   |        hjw | /
  )  |  \  `.___________|/
  `--'   `--'

    /\_____/\
   /  o   o  \
  ( ==  ^  == )
   )         (
  (           )
 ( (  )   (  ) )
(__(__)___(__)__)

("`-''-/").___..--''"`-._ 
 `6_ 6  )   `-.  (     ).`-.__.`) 
 (_Y_.)'  ._   )  `._ `. ``-..-' 
   _..`--'_..-_/  /--'_.'
  ((((.-''  ((((.'  (((.-' 


                      /^--^\     /^--^\     /^--^\
                      \____/     \____/     \____/
                     /      \   /      \   /      \
KAT                 |        | |        | |        |
                     \__  __/   \__  __/   \__  __/
|^|^|^|^|^|^|^|^|^|^|^|^\ \^|^|^|^/ /^|^|^|^|^\ \^|^|^|^|^|^|^|^|^|^|^|^|
| | | | | | | | | | | | |\ \| | |/ /| | | | | | \ \ | | | | | | | | | | |
########################/ /######\ \###########/ /#######################
| | | | | | | | | | | | \/| | | | \/| | | | | |\/ | | | | | | | | | | | |
|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|


     db         db
    dpqb       dp8b
    8b qb_____dp_88
    88/ .        `p
    q'.            \
   .'.  .-.    ,-.  `--.
   |.  / 0 \  / 0 \ |   \
   |.  `.__   ___.' | \\/
   |.       "       | (
    \.    `-'-'    ,' |
   _/`------------'. .|
  /.  \\::(::[];)||.. \
 /.  ' \.`:;;;;'''/`. .|
|.   |/ `;--._.__/  |..|
|.  _/_,'''',,`.    `:.'
|.     ` / ,  ',`.   |/     "Yotsuya no Neko"
 \.   -'/\/     ',\  |\         gst38min
  /\__-' /\ /     ,. |.\       1995.08.31
 /. .|  '  /-.    ,: |..\
:.  .|    /| | ,  ,||. ..:
|.  .`     | '//` ,:|.  .|
|..  .\      //\/ ,|.  ..|
 \.   .\     <./  ,'. ../
  \_ ,..`.__    _,..,._/
    `\|||/  `--'\|||/'


                                               .--.
                                               `.  \
                                                 \  \
                                                  .  \
                                                  :   .
                                                  |    .
                                                  |    :
                                                  |    |
  ..._  ___                                       |    |
 `."".`''''""--..___                              |    |
 ,-\  \             ""-...__         _____________/    |
 / ` " '                    `""""""""                  .
 \                                                      L
 (>                                                      \
/                                                         \
\_    ___..---.                                            L
  `--'         '.                                           \
                 .                                           \_
                _/`.                                           `.._
             .'     -.                                             `.
            /     __.-Y     /''''''-...___,...--------.._            |
           /   _."    |    /                ' .      \   '---..._    |
          /   /      /    /                _,. '    ,/           |   |
          \_,'     _.'   /              /''     _,-'            _|   |
                  '     /               `-----''               /     |
                  `...-'     dp                                `...-'


*/
