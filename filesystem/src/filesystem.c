#include "filesystem.h"

t_config* superBloque;
t_bitarray *bitmap;
char **vectorDePathsPCBs;
int cantidadPaths;
void* memoriaMapeada;
int conexion_memoria = -1;

int main(int argc, char** argv)
{

	if (argc < 2)
	{
		return EXIT_FAILURE;
	}
	logger = iniciar_logger("./filesystem.log", "FileSystem");
	log_info(logger, "logg iniciado");
	config = iniciar_config(argv[1]);
	int socket_servidor = -1;

	char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	
	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("Intento de conexión del filesystem a la memoria", conexion_memoria, MENSAJE);
	enviar_operacion(conexion_memoria, FILESYSTEM);
	
	//Una vez realizada la coneccion a memoria levanto el bitmap de bloques y recorro FCBs
	// Trabajo sobre file System exclyuyendo conexiones

	superBloque=iniciar_config(config_get_string_value(config,"PATH_SUPERBLOQUE"));

	if (superBloque == NULL)
	{
	    log_error(logger, "Error al inicializar la configuración del superbloque");
	    exit(EXIT_FAILURE);
	}

	cantidadPaths = contarArchivosEnCarpeta(config_get_string_value(config,"PATH_FCB"),&vectorDePathsPCBs);

	iniciarArchivoBitmap();
	size_t tamanioBitmap = config_get_int_value(superBloque, "BLOCK_COUNT") / 8;

	// Abre el archivo Bitmap para trabajar con mmap desde la memoria
	int fd_bitmap = open(config_get_string_value(config, "PATH_BITMAP"),O_RDWR);

	if (fd_bitmap == -1)
	{
	    log_error(logger, "Error al abrir el archivo del bitmap");
	    exit(EXIT_FAILURE);
	}


	memoriaMapeada = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd_bitmap, 0);
	if (memoriaMapeada == MAP_FAILED)
	{
	    log_error(logger, "Error al mapear el archivo del bitmap");
	    exit(EXIT_FAILURE);
	}

	bitmap = bitarray_create_with_mode(memoriaMapeada, tamanioBitmap, LSB_FIRST);

	if (bitmap == NULL)
	{
		log_error(logger, "Error al crear el bitmap");
		exit(EXIT_FAILURE);
	}


	
	char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto_escucha);
	esperar_cliente(socket_servidor);

	munmap(memoriaMapeada,tamanioBitmap);
	close(fd_bitmap);
	liberar_conexion(conexion_memoria);

	terminar_programa(logger, config,fd_bitmap);
	log_debug(logger, "ACA TERMINA FILESYSTEM. O SALIO TODO JOYA O TODO PARA EL ORTOOOOOOOOO");
	return EXIT_SUCCESS;
}

void terminar_programa(t_log* logger, t_config* config,int fd_bitmap)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}
	munmap(memoriaMapeada,config_get_int_value(superBloque,"BLOCK_COUNT")/8);
	close(fd_bitmap);
	bitarray_destroy(bitmap);
	if (config != NULL)
	{
		config_destroy(config);
	}
	if (superBloque != NULL)
	{
		config_destroy(superBloque);
	}
}
char* concatenarCadenas(const char* str1, const char* str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* resultado = (char*)malloc((len1 + len2 + 1) * sizeof(char));

    if (resultado == NULL) {
        perror("Error de asignación de memoria");
        return NULL;
    }

    strcpy(resultado, str1);
    strcat(resultado, str2);

    return resultado;
}
int contarArchivosEnCarpeta(const char *carpeta, char ***vectoreRutas) {
    DIR *dir;
    struct dirent *ent;
    int contador = 0;
    int contador2 = 0;
    char *mediaRutaAbsoluta = concatenarCadenas(carpeta,"/");
    char *rutaAbsoluta;
    dir = opendir(carpeta);

    if (dir == NULL)
    {
        log_info(logger, "No se pudo abrir la carpeta");
        return -1; // Retorna -1 en caso de error
    }

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG)
        { // Verifica si es un archivo regular
        	contador++;
        }
    }
    *vectoreRutas = malloc(contador * sizeof(char*));
    closedir(dir);
    dir = opendir(carpeta);
    while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // Verifica si es un archivo regular
            	(*vectoreRutas)[(contador2)]=malloc((strlen(ent->d_name) + 1) * sizeof(char) + (strlen(mediaRutaAbsoluta) + 1) * sizeof(char));
            	rutaAbsoluta = concatenarCadenas(mediaRutaAbsoluta,ent->d_name);
            	strcpy((*vectoreRutas)[(contador2)], rutaAbsoluta);
            	contador2++;
            	free(rutaAbsoluta);
            }
        }
    closedir(dir);
    free (mediaRutaAbsoluta);
    return contador;
}
int abrirArchivo(char *nombre, char **vectorDePaths,int cantidadPaths)
{
	log_info(logger,"Abrir Archivo: %s",nombre);
	int i=0;
	t_config* config_inicial;
	while (i<cantidadPaths)
	{
		config_inicial= iniciar_config(vectorDePaths[i]);
		if(strcmp(nombre,config_get_string_value(config_inicial,"NOMBRE_ARCHIVO")) == 0)
		{
			config_destroy(config_inicial);
			return 1;
		}
		i++;
		config_destroy(config_inicial);
	}
	return 0;
}
int crearArchivo(char *nombre,char *carpeta, char ***vectoreRutas, int *cantidadPaths)
{
	log_info(logger, "Crear Archivo: %s",nombre);
	FILE* archivo;
	t_config* configArchivo;
	char **vectorPruebas;
	char *mediaRutaAbsoluta = concatenarCadenas(carpeta,"/");
	char *rutaArchivo;
	rutaArchivo = concatenarCadenas(mediaRutaAbsoluta, nombre);

	archivo = fopen(rutaArchivo , "w+");
	fclose(archivo);
	configArchivo = iniciar_config(rutaArchivo);
	config_set_value(configArchivo,"NOMBRE_ARCHIVO", nombre);
	config_set_value(configArchivo,"TAMANIO_ARCHIVO", "0");
	config_save(configArchivo);
	vectorPruebas = realloc(*vectoreRutas,(*cantidadPaths + 1) * sizeof(char*));
	if (vectorPruebas != NULL)
	{
		(*vectoreRutas) = vectorPruebas;

		(*vectoreRutas)[*cantidadPaths] = malloc((strlen(rutaArchivo) + 1) * sizeof(char));
		strcpy((*vectoreRutas)[*cantidadPaths],rutaArchivo);
		*cantidadPaths = *cantidadPaths + 1;
		config_destroy(configArchivo);
		free (mediaRutaAbsoluta);
		free(rutaArchivo);
		return 1;
	}
	else
	{
		config_destroy(configArchivo);
		free (mediaRutaAbsoluta);
		free(rutaArchivo);
		return 0;
	}
}

int truncarArchivo(char *nombre,char *carpeta, char **vectoreRutas, int cantidadPaths, int tamanioNuevo)
{
	log_info(logger, "Truncar Archivo: %s - Tamaño: %d",nombre,tamanioNuevo);
	int i=0;
	t_config* configArchivoActual;
	int tamanioBloques;
	int tamanioOriginal;
	int cantidadBloquesOriginal;
	int cantidadBloquesNueva;
	char *tamanioNuevoChar;

	while (i<cantidadPaths)
	{
		configArchivoActual = iniciar_config(vectoreRutas[i]);
		if(strcmp(nombre,config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO")) == 0)
		{
			log_info(logger,"Truncar archivo dice: ENCONTRE EL ARCHIVO A TRUNCAR");
			break;
		}
		i++;
		config_destroy(configArchivoActual);
	}

	tamanioOriginal = config_get_int_value(configArchivoActual,"TAMANIO_ARCHIVO");
	tamanioBloques = config_get_int_value(superBloque,"BLOCK_SIZE");
	config_set_value(configArchivoActual,"TAMANIO_ARCHIVO",tamanioNuevoChar = string_itoa(tamanioNuevo));
	free(tamanioNuevoChar);
	cantidadBloquesOriginal = dividirRedondeando(tamanioOriginal, tamanioBloques);
	cantidadBloquesNueva = dividirRedondeando(tamanioNuevo, tamanioBloques);
	config_save(configArchivoActual);
	if (tamanioOriginal < tamanioNuevo && cantidadBloquesOriginal != cantidadBloquesNueva)
	{
		log_info(logger,"Se agranda el archivo");
		agregarBloques(cantidadBloquesOriginal,cantidadBloquesNueva,configArchivoActual);
		config_destroy(configArchivoActual);
		return 1;
	}
	else
	{
		if(cantidadBloquesOriginal != cantidadBloquesNueva)
		{
			log_info(logger,"Se achica el archivo");
			sacarBloques(cantidadBloquesOriginal,cantidadBloquesNueva,configArchivoActual, tamanioOriginal);
			config_destroy(configArchivoActual);
			return 1;
		}
	}
	log_info(logger, "No se modificaron la canidad de bloques del archivo");
	config_destroy(configArchivoActual);
	return 1;
}

void sacarBloques(int cantidadBloquesOriginal ,int cantidadBloquesNueva,t_config* configArchivoActual,int tamanioOriginal)
{
	int cantidadBloquesAEliminar =cantidadBloquesOriginal - cantidadBloquesNueva;
	uint32_t punteroIndirecto = config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO");
	uint32_t punteroACadaBloque;
	size_t tamanioBloque = config_get_int_value(superBloque,"BLOCK_SIZE");
	FILE *bloques = fopen(config_get_string_value(config,"PATH_BLOQUES"),"r+");

	if (cantidadBloquesOriginal <= 1)
	{
		//busco un bloque libre para agregar como bloque de punteros
		log_info(logger,"El archivo solo tiene un bloque. No se modifica nada referido a los bloques");
		return;
	}
	if (cantidadBloquesNueva == 1)
	{
		// Me muevo al ultimo puntero del bloque de punteros para eliminar puntero por puntero
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),punteroIndirecto);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		fseek(bloques,punteroIndirecto * tamanioBloque,SEEK_SET);
		fseek(bloques,sizeof(uint32_t)*(cantidadBloquesOriginal - 1), SEEK_CUR);

		for(int i=0;i<cantidadBloquesAEliminar ;i++)
		{
			fseek(bloques,-sizeof(uint32_t), SEEK_CUR);
			fread(&punteroACadaBloque,sizeof(uint32_t),1,bloques);
			limpiarBitmap(bitmap, punteroACadaBloque);
			fseek(bloques,-sizeof(uint32_t), SEEK_CUR);

		}
		limpiarBitmap(bitmap, config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO"));
		config_remove_key(configArchivoActual,"PUNTERO_INDIRECTO");
		config_save(configArchivoActual);
	}
	if (cantidadBloquesNueva > 1)
	{
		// Me muevo al ultimo puntero del bloque de punteros para eliminar puntero por puntero
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),punteroIndirecto);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		fseek(bloques,punteroIndirecto * tamanioBloque,SEEK_SET);
		fseek(bloques,sizeof(uint32_t)*(cantidadBloquesOriginal - 1), SEEK_CUR);

		for(int i=0;i<cantidadBloquesAEliminar ;i++)
		{
			fseek(bloques,-sizeof(uint32_t), SEEK_CUR);
			fread(&punteroACadaBloque,sizeof(uint32_t),1,bloques);
			limpiarBitmap(bitmap, punteroACadaBloque);
			fseek(bloques,-sizeof(uint32_t), SEEK_CUR);

		}
	}
	fclose(bloques);
	return;
}
void agregarBloques(int cantidadBloquesOriginal ,int cantidadBloquesNueva,t_config* configArchivoActual)
{
	int cantidadBloquesAAGregar = cantidadBloquesNueva - cantidadBloquesOriginal;
	FILE *bloques = fopen(config_get_string_value(config,"PATH_BLOQUES"),"r+");
	int punteroABloquePunteros=0;
	uint32_t punteroACadaBloque;
	char *valorIChar;
	if(cantidadBloquesOriginal == 0 && cantidadBloquesNueva == 1)
	{
		//Busco un bloque libre para agregar el primer bloque de datos
		log_info(logger,"Se busca bloque libre para agregar como primer bloque");
		for(int i=0;i<config_get_int_value(superBloque, "BLOCK_COUNT");i++)
		{
			if (accesoBitmap(bitmap, i) == 0)
			{
				//Encontre un bloque vacio lo marco como ocupado
				setearBitmap(bitmap,i);
				config_set_value(configArchivoActual,"PUNTERO_DIRECTO",valorIChar = string_itoa(i));
				free(valorIChar);
				config_save(configArchivoActual);
				sincronizarBitmap();
				break;

			}
		}
	}
	if (cantidadBloquesOriginal == 0 && cantidadBloquesNueva >1)
		{
			//Busco un bloque libre para agregar el primer bloque de datos
			log_info(logger,"Se busca bloque libre para agregar como primer bloque");
			for(int i=0;i<config_get_int_value(superBloque, "BLOCK_COUNT");i++)
			{
				if (accesoBitmap(bitmap, i) == 0)
				{
					//Encontre un bloque vacio lo marco como ocupado
					setearBitmap(bitmap,i);
					config_set_value(configArchivoActual,"PUNTERO_DIRECTO",string_itoa(i));
					config_save(configArchivoActual);
					sincronizarBitmap();
					break;

				}
			}
			//busco un bloque libre para agregar como bloque de punteros
			log_info(logger,"Se busca bloque libre para agregar los punteros a bloques");
			for(int i=0;i<config_get_int_value(superBloque, "BLOCK_COUNT");i++)
			{
				if (accesoBitmap(bitmap, i) == 0)
				{
					//Encontre un bloque vacio lo marco como ocupado
					setearBitmap(bitmap,i);
					config_set_value(configArchivoActual,"PUNTERO_INDIRECTO",string_itoa(i));
					config_save(configArchivoActual);
					punteroABloquePunteros = i;
					sincronizarBitmap();
					break;

				}
			}
			// Me ubico en el bloque de punteros
			log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),punteroABloquePunteros);
			fseek(bloques,punteroABloquePunteros * config_get_int_value(superBloque,"BLOCK_SIZE"),SEEK_SET);
			delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
			log_info(logger,"Busco bloques libres para agregar al archivo");
			//Busco los espacios libres en el bitmap para agregar los bloques
			for (int i=0;i< cantidadBloquesAAGregar -1;i++)
			{
				int j=1;
				int posicion=0;
				while(j != 0)
				{
					j=accesoBitmap(bitmap, posicion);
					posicion++;
				}
				punteroACadaBloque = posicion - 1;
				setearBitmap(bitmap, posicion -1);
				fwrite(&punteroACadaBloque,sizeof(uint32_t),1,bloques);
			}
			fclose(bloques);
			return;
		}
	if(cantidadBloquesOriginal == 1 && cantidadBloquesNueva >1)
	{
		//busco un bloque libre para agregar como bloque de punteros
		log_info(logger,"Se busca bloque libre para agregar los punteros a bloques");
		for(int i=0;i<config_get_int_value(superBloque, "BLOCK_COUNT");i++)
		{
			if (accesoBitmap(bitmap, i) == 0)
			{
				//Encontre un bloque vacio lo marco como ocupado
				setearBitmap(bitmap,i);
				config_set_value(configArchivoActual,"PUNTERO_INDIRECTO",string_itoa(i));
				config_save(configArchivoActual);
				punteroABloquePunteros = i;
				sincronizarBitmap();
				break;

			}
		}
		// Me ubico en el bloque de punteros
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),punteroABloquePunteros);
		fseek(bloques,punteroABloquePunteros * config_get_int_value(superBloque,"BLOCK_SIZE"),SEEK_SET);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		for (int i=0;i< cantidadBloquesAAGregar;i++)
		{
			int j=1;
			int posicion=0;
			while(j != 0)
			{
				j=accesoBitmap(bitmap, posicion);
				posicion++;
			}
			setearBitmap(bitmap, posicion -1);
			punteroACadaBloque = posicion - 1;
			fwrite(&punteroACadaBloque,sizeof(uint32_t),1,bloques);
		}
	}
	if(cantidadBloquesOriginal > 1)
	{
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO"));
		fseek(bloques,config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO") * config_get_int_value(superBloque,"BLOCK_SIZE"),SEEK_SET);
		fseek(bloques,sizeof(uint32_t)*(cantidadBloquesOriginal - 1),SEEK_CUR);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		for (int i=0;i< cantidadBloquesAAGregar;i++)
		{
			int j=1;
			int posicion=0;
			while(j != 0)
			{
				j=accesoBitmap(bitmap, posicion);
						posicion++;
			}
			punteroACadaBloque = posicion - 1;
			fwrite(&punteroACadaBloque,sizeof(uint32_t),1,bloques);
		}
	}
	fclose(bloques);
	return;
}
int dividirRedondeando(int numero1 , int numero2)
{
	if(numero1 % numero2 == 0)
	{
		return (numero1)/(numero2);
	}
	else
	{
		return (numero1)/(numero2) + 1;
	}
}
void iniciarArchivoBitmap()
{
	FILE *archivoBitmap=fopen(config_get_string_value(config,"PATH_BITMAP"),"r+");
	void* punteroABitmapPruebas = malloc(config_get_int_value(superBloque, "BLOCK_COUNT") / 8);
	t_bitarray *bitmapPruebas = bitarray_create_with_mode(punteroABitmapPruebas, config_get_int_value(superBloque, "BLOCK_COUNT") / 8, LSB_FIRST);
	for(int i=0;i<config_get_int_value(superBloque, "BLOCK_COUNT") / 8;i++)
	{
		bitarray_clean_bit(bitmapPruebas, i);
	}
	// Muevo el puntero al final del archivo
	fseek(archivoBitmap, 0, SEEK_END);
	// Obtengo la posición actual del puntero
	 long int size = ftell(archivoBitmap);
	if(size == 0)
	{
		fwrite(bitmapPruebas->bitarray,config_get_int_value(superBloque, "BLOCK_COUNT") / 8,1,archivoBitmap);
	}
	fclose(archivoBitmap);
	bitarray_destroy(bitmapPruebas);
	free(punteroABitmapPruebas);
	return;
}
bool accesoBitmap(t_bitarray* bitmapAAcceder, off_t bit_index)
{
	int posicionIndicada = bit_index;
	log_info(logger,"Acceso a Bitmap - Bloque: %d - Estado: %d",posicionIndicada,bitarray_test_bit(bitmapAAcceder,bit_index));
	return bitarray_test_bit(bitmapAAcceder,bit_index);

}
void setearBitmap(t_bitarray* bitmapAAcceder, off_t bit_index)
{
	int posicionIndicada = bit_index;
	bitarray_set_bit(bitmapAAcceder, bit_index);
	log_info(logger,"Modificacion Bitmap - Bloque: %d - Estado nuevo: %d",posicionIndicada,bitarray_test_bit(bitmapAAcceder,bit_index));
	return;
}
void limpiarBitmap(t_bitarray* bitmapAAcceder, off_t bit_index)
{
	int posicionIndicada = bit_index;
	bitarray_clean_bit(bitmapAAcceder, bit_index);
	log_info(logger,"Modificacion Bitmap - Bloque: %d - Estado nuevo: %d",posicionIndicada,bitarray_test_bit(bitmapAAcceder,bit_index));
}
void sincronizarBitmap()
{
	if (msync(memoriaMapeada, config_get_int_value(superBloque, "BLOCK_COUNT") / 8, MS_SYNC) == 0)
	{
		log_info(logger, "Se escribió correctamente en el bitmap");
	}
	else
	{
		log_warning(logger, "No se pudo escribir correctamente en el bitmap");
	}
}
void revisarBitmap(int hastaDonde)
{
	for(int i=0;i<hastaDonde; i++ )
	{
		accesoBitmap(bitmap, i);
	}
}

int escribirArchivo(char *nombreArchivo,size_t punteroSeek,size_t bytesAEscribir,int direccion,void *memoriaAEscribir)
{
	log_info(logger,"Escribir Archivo: %s - Puntero: %ld - Memoria: %d - Tamaño: %ld",nombreArchivo,punteroSeek,direccion,bytesAEscribir);
	int i=0;
	t_config* configArchivoActual;
	size_t bloqueAEscribir;
	FILE *bloques = fopen(config_get_string_value(config,"PATH_BLOQUES"),"r+");
	size_t tamanioBloque = config_get_int_value(superBloque,"BLOCK_SIZE");
	size_t escritoAnteriormente = 0;

	while (i<cantidadPaths)
	{
		configArchivoActual = iniciar_config(vectorDePathsPCBs[i]);
		if(strcmp(nombreArchivo,config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO")) == 0)
		{
			log_info(logger,"Escribir  dice: ENCONTRE EL ARCHIVO A escribir");
			break;
		}
		i++;
		config_destroy(configArchivoActual);
	}
	int cantidadBloquesAEscribir = cantidadDeBloquesAAcceder(configArchivoActual,punteroSeek,bytesAEscribir);
	bloqueAEscribir = punteroSeek /tamanioBloque;
	log_info(logger,"El bloque del archivo a escribir es el bloque %ld",bloqueAEscribir);
	if(bloqueAEscribir == 0)
	{
		moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAEscribir);
		//Me fijo si todo lo que voy a leer esta en un solo bloque
		if(cantidadBloquesAEscribir)
		{
			//Escribo todo lo que puedo en el primer bloque del archivo y luego paso al segundo
			log_info(logger,"La informacion a escribir NO entra en un solo bloque");
			fseek(bloques,punteroSeek,SEEK_CUR);
			fwrite(memoriaAEscribir,tamanioBloque-punteroSeek,1,bloques);
			
			escritoAnteriormente = tamanioBloque-(punteroSeek-bloqueAEscribir * tamanioBloque);
			for(int i = 1;i<cantidadBloquesAEscribir;i++)
			{
				moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAEscribir + i);
				fwrite(memoriaAEscribir + escritoAnteriormente,(bytesAEscribir - escritoAnteriormente)-((cantidadBloquesAEscribir - (i+1)) * tamanioBloque),1,bloques);
				escritoAnteriormente = (size_t)memoriaAEscribir + (bytesAEscribir - escritoAnteriormente)-((cantidadBloquesAEscribir - (i+1)) * tamanioBloque);	
			}
			fclose(bloques);
			config_destroy(configArchivoActual);
			return 1;
		}
		//escribo todo en el bloque 0
		else
		{
			log_info(logger,"La informacion a escribir entra en un solo bloque");
			fseek(bloques,punteroSeek,SEEK_CUR);
			fwrite(memoriaAEscribir,bytesAEscribir,1,bloques);
			fclose(bloques);
			config_destroy(configArchivoActual);
			return 1;
		}
	}
	//no tengo que escribir el bloque del puntero directo. Paso a buscar el primer bloque
	else
	{
		//Hay mas de un bloque para leer
		if(cantidadBloquesAEscribir)
		{
			log_info(logger,"La informacion a escribir NO entra en un solo bloque");
			moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAEscribir);
			//Escribo todo lo que puedo del primer bloque
			fseek(bloques,punteroSeek-(tamanioBloque * bloqueAEscribir),SEEK_CUR);
			fwrite(memoriaAEscribir,tamanioBloque-(punteroSeek-bloqueAEscribir * tamanioBloque),1,bloques);
			escritoAnteriormente = tamanioBloque-(punteroSeek-bloqueAEscribir * tamanioBloque);
			for(int i = 1;i<cantidadBloquesAEscribir;i++)
			{
				moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAEscribir + i);
				fwrite(memoriaAEscribir + escritoAnteriormente,(bytesAEscribir - escritoAnteriormente)-((cantidadBloquesAEscribir - (i+1)) * tamanioBloque),1,bloques);
				escritoAnteriormente = (size_t)memoriaAEscribir + (bytesAEscribir - escritoAnteriormente)-((cantidadBloquesAEscribir - (i+1)) * tamanioBloque);

				
			}
			fclose(bloques);
			config_destroy(configArchivoActual);
			return 1;
		}
		//Hay solo un bloque que leer
		else
		{
			log_info(logger,"La informacion a escribir entra en un solo bloque");
			moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAEscribir);
			fseek(bloques,punteroSeek-(tamanioBloque * bloqueAEscribir),SEEK_CUR);
			fwrite(memoriaAEscribir,bytesAEscribir,1,bloques);
			fclose(bloques);
			config_destroy(configArchivoActual);
			return 1;
		}
	}
	fclose(bloques);
	config_destroy(configArchivoActual);
	return 0;

}

void *leerArchivo(char *nombreArchivo,size_t punteroSeek,size_t bytesALeer, int direccion)
{
	log_info(logger,"Leer Archivo: %s - Puntero: %ld - Memoria: %d - Tamaño: %ld",nombreArchivo,punteroSeek,direccion,bytesALeer);
	int i=0;
	t_config* configArchivoActual;
	size_t bloqueAleer;
	void *infoDelArchivo;
	FILE *bloques = fopen(config_get_string_value(config,"PATH_BLOQUES"),"r+");
	void *punteroFinal;
	size_t tamanioBloque = config_get_int_value(superBloque,"BLOCK_SIZE");
	size_t leidoAnteriormente = 0;
	while (i<cantidadPaths)
	{
		configArchivoActual = iniciar_config(vectorDePathsPCBs[i]);
		if(strcmp(nombreArchivo,config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO")) == 0)
		{
			log_info(logger,"Leer  dice: ENCONTRE EL ARCHIVO A LEER");
			break;
		}
		i++;
		config_destroy(configArchivoActual);
	}
	bloqueAleer = punteroSeek /tamanioBloque;
	int cantidadBloquesALeer = cantidadDeBloquesAAcceder(configArchivoActual,punteroSeek,bytesALeer);
	log_info(logger,"El bloque del archivo a leer es el bloque %ld",bloqueAleer);
	//Busco el bloque desde donde voy a leer usando los punteros del archivo
	if(bloqueAleer == 0)
	{
		moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAleer);
		//Me fijo si todo lo que voy a leer esta en un solo bloque
		if(cantidadBloquesALeer)
		{
			log_info(logger,"La informacion a leer NO esta en un solo bloque");
			infoDelArchivo= malloc(bytesALeer);
			fseek(bloques,punteroSeek,SEEK_CUR);
			fread(infoDelArchivo,tamanioBloque-punteroSeek,1,bloques);
			
			//Ahora me voy al bloque siguiente
			leidoAnteriormente = tamanioBloque-(punteroSeek-bloqueAleer * tamanioBloque);
			for(int i = 1;i<cantidadBloquesALeer;i++)
			{
				moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAleer + i);
				fread(infoDelArchivo + leidoAnteriormente,(bytesALeer - leidoAnteriormente)-((cantidadBloquesALeer - (i+1)) * tamanioBloque),1,bloques);
				leidoAnteriormente = (size_t)infoDelArchivo + (bytesALeer - leidoAnteriormente)-((cantidadBloquesALeer - (i+1)) * tamanioBloque);
			}
			fclose(bloques);
			config_destroy(configArchivoActual);
			return infoDelArchivo;
		}
		//leo todo del primer bloque
		else
		{
			log_info(logger,"La informacion a leer esta en un solo bloque");
			infoDelArchivo= malloc(bytesALeer);
			fseek(bloques,punteroSeek,SEEK_CUR);
			fread(infoDelArchivo,bytesALeer,1,bloques);
			fclose(bloques);
			config_destroy(configArchivoActual);
			return infoDelArchivo;

		}
	}
	//no tengo que leer el bloque del puntero directo. Paso a buscar los demas bloques
	else
	{
		moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAleer);

		//Hay mas de un bloque para leer
		if(cantidadBloquesALeer)
		{
			//Leo lo que puedo del primer bloque
			infoDelArchivo= malloc(bytesALeer);			
			fseek(bloques,punteroSeek-(tamanioBloque * bloqueAleer),SEEK_CUR);
			fread(infoDelArchivo,tamanioBloque-(punteroSeek-bloqueAleer * tamanioBloque),1,bloques);
			
			leidoAnteriormente = tamanioBloque-(punteroSeek-bloqueAleer * tamanioBloque);
			for(int i = 1;i<cantidadBloquesALeer;i++)
			{
				moverPunteroAbloqueDelArchivo(bloques,configArchivoActual,bloqueAleer + i);
				fread(infoDelArchivo + leidoAnteriormente,(bytesALeer - leidoAnteriormente)-((cantidadBloquesALeer - (i+1)) * tamanioBloque),1,bloques);
				leidoAnteriormente = (size_t)infoDelArchivo + (bytesALeer - leidoAnteriormente)-((cantidadBloquesALeer - (i+1)) * tamanioBloque);
			}
			fclose(bloques);
			config_destroy(configArchivoActual);
			return infoDelArchivo;
		}
		//Hay solo un bloque que leer
		else
		{
			infoDelArchivo = malloc(bytesALeer);
			fseek(bloques,punteroSeek-(tamanioBloque * bloqueAleer),SEEK_CUR);
			fread(infoDelArchivo,bytesALeer,1,bloques);
			fclose(bloques);
			config_destroy(configArchivoActual);
			return infoDelArchivo;
		}
	}
	config_destroy(configArchivoActual);
	fclose(bloques);
	return punteroFinal;
}
void* concatPunteros(void* ptr1, void* ptr2, size_t size1, size_t size2) 
{
    char* resultado = malloc(size1 + size2); // Reserva memoria para el resultado

    memcpy(resultado, ptr1, size1); // Copia los bytes de ptr1 al resultado
    memcpy(resultado + size1, ptr2, size2); // Copia los bytes de ptr2 al resultado

    return resultado;
}

void moverPunteroABloquePunteros (FILE* bloques, t_config* configArchivoActual)
{
	log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: bloque de punteros - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO"));
	delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
	fseek(bloques,config_get_int_value(configArchivoActual,"PUNTERO_INDIRECTO")* config_get_int_value(superBloque,"BLOCK_SIZE"),SEEK_SET);
}
void moverPunteroAbloqueDelArchivo(FILE* bloques, t_config* configArchivoActual,int bloqueBuscado)
{
	uint32_t punteroAbloqueBuscado;
	
	if(bloqueBuscado == 0)
	{
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),bloqueBuscado,	config_get_int_value(configArchivoActual,"PUNTERO_DIRECTO"));
		fseek(bloques,config_get_int_value(configArchivoActual,"PUNTERO_DIRECTO") * config_get_int_value(superBloque,"BLOCK_SIZE") ,SEEK_SET);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		return;
	}
	else
	{
		moverPunteroABloquePunteros(bloques,configArchivoActual);
		fseek(bloques,sizeof(uint32_t) * (bloqueBuscado - 1),SEEK_CUR);
		//leo la posicion del bloque buscado bloque buscado
		fread(&punteroAbloqueBuscado,sizeof(uint32_t),1,bloques);
		//Me muevo a ese bloque
		log_info(logger,"Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d",config_get_string_value(configArchivoActual,"NOMBRE_ARCHIVO"),bloqueBuscado,	punteroAbloqueBuscado);
		fseek(bloques,punteroAbloqueBuscado * config_get_int_value(superBloque,"BLOCK_SIZE"),SEEK_SET);
		delay(config_get_int_value(config,"RETARDO_ACCESO_BLOQUE"));
		return;
	}
	

}
int cantidadDeBloquesAAcceder(t_config *archivoActual,size_t punteroAInformacion,size_t bytesAOperar)
{
	size_t tamanioBloque = config_get_int_value(superBloque,"BLOCK_SIZE");
	int bloquesAleer = (punteroAInformacion + bytesAOperar)/tamanioBloque;
	return bloquesAleer;

}
void pruebaArchivos()
{
	// PRUEBAS CON UN ARCHIVO GENERICO

	
	if(abrirArchivo("archivoPruebas2",vectorDePathsPCBs,cantidadPaths))
	{
		log_info(logger,"Abrir archivo retorna OK");
	}
	else
	{
		log_info(logger,"Abrir archivo retorna EL_ARCHIVO_NO_EXISTE_PAAAAAAA");
	}
	//////////////////////////////////////
	if(crearArchivo("archivoPruebas2", config_get_string_value(config,"PATH_FCB"), &vectorDePathsPCBs, &cantidadPaths))
	{
		log_info(logger,"Se creo joya el archivo");
	}
	else
	{
		log_error(logger,"No se pudo crear el archivo pibe. Algo se rompio zarpado");
	}
	//////////////////////////////////////
	if(abrirArchivo("archivoPruebas2",vectorDePathsPCBs,cantidadPaths))
	{
		log_info(logger,"Abrir archivo retorna OK");
	}
	else
	{
		log_info(logger,"Abrir archivo retorna EL_ARCHIVO_NO_EXISTE_PAAAAAAA");
	}
	//////////////////////////////////////
	if(truncarArchivo("archivoPruebas2", config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, 1024))
	{
		log_info(logger,"En teoria el archivo deberia estar truncado");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo truncar");
	}


	// Pruebas genericas PARTE 2
	if(truncarArchivo("archivoPruebas2", config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, 320))
	{
		log_info(logger,"En teoria el archivo deberia estar truncado");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo truncar");
	}
	
	
	if(crearArchivo("elQueMireElArchivoEsGay", config_get_string_value(config,"PATH_FCB"), &vectorDePathsPCBs, &cantidadPaths))
	{
		log_info(logger,"Se creo joya el archivo");
	}
	else
	{
		log_error(logger,"No se pudo crear el archivo pibe. Algo se rompio zarpado");
	}

	if(crearArchivo("toyotaSupra", config_get_string_value(config,"PATH_FCB"), &vectorDePathsPCBs, &cantidadPaths))
	{
		log_info(logger,"Se creo joya el archivo");
	}
	else
	{
		log_error(logger,"No se pudo crear el archivo pibe. Algo se rompio zarpado");
	}

	if(crearArchivo("tomate", config_get_string_value(config,"PATH_FCB"), &vectorDePathsPCBs, &cantidadPaths))
	{
		log_info(logger,"Se creo joya el archivo");
	}
	else
	{
		log_error(logger,"No se pudo crear el archivo pibe. Algo se rompio zarpado");
	}
	
	
	if(truncarArchivo("elQueMireElArchivoEsGay", config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, 100))
	{
		log_info(logger,"En teoria el archivo deberia estar truncado");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo truncar");
	}

	if(truncarArchivo("toyotaSupra", config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, 150))
	{
		log_info(logger,"En teoria el archivo deberia estar truncado");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo truncar");
	}

	if(truncarArchivo("tomate", config_get_string_value(config,"PATH_FCB"), vectorDePathsPCBs, cantidadPaths, 60))
	{
		log_info(logger,"En teoria el archivo deberia estar truncado");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo truncar");
	}

	// Escribo algo en el archivo para ver que lee

	//Opcion 1
	char* infoPrueba = malloc((strlen("Hola estoy escrito en un archivo") + 1) * sizeof(char));
	strcpy(infoPrueba,"Hola estoy escrito en un archivo");

	//Opcion 2
	//infoPrueba = string_from_format("Hola estoy escrito en un archivo");
	
	
	if(escribirArchivo("archivoPruebas2",123,(strlen("Hola estoy escrito en un archivo") + 1) * sizeof(char),120,infoPrueba))
	{
		log_info(logger,"En teoria se deberia haber escrito el archivo");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo escribir");
	}
	
	char *AlgoALeer = leerArchivo("archivoPruebas2",123,(strlen("Hola estoy escrito en un archivo") + 1) * sizeof(char),0);

	log_info(logger,"Lo leido del archivo es %s", AlgoALeer);
	
	char *infoPrueba3 = string_from_format("Hola soy un toyota supra sututu");
	if(escribirArchivo("toyotaSupra",123,(strlen("Hola soy un toyota supra sututu") + 1) * sizeof(char),120,infoPrueba3))
	{
		log_info(logger,"En teoria se deberia haber escrito el archivo");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo escribir");
	}
	char *infoPrueba2 = string_from_format("Hola soy un tomate");
	if(escribirArchivo("tomate",5,(strlen("Hola soy un tomate") + 1) * sizeof(char),120,infoPrueba2))
	{
		log_info(logger,"En teoria se deberia haber escrito el archivo");
	}
	else
	{
		log_warning(logger,"El archivo no se pudo escribir");
	}
}

/*
      __                                                      
     /  l                                                     
   .'   :               __.....__..._  ____                   
  /  /   \          _.-"        "-.  ""    "-.                
 (`-: .---:    .--.'          _....J.         "-.             
  """y     \,.'    \  __..--""       `+""--.     `.           
    :     .'/    .-"""-. _.            `.   "-.    `._.._     
    ;  _.'.'  .-j       `.               \     "-.   "-._`.   
    :    / .-" :          \  `-.          `-      "-.      \  
     ;  /.'    ;          :;               ."        \      `,
     :_:/      ::\        ;:     (        /   .-"   .')      ;
       ;-"      ; "-.    /  ;           .^. .'    .' /    .-" 
      /     .-  :    `. '.  : .- / __.-j.'.'   .-"  /.---'    
     /  /      `,\.  .'   "":'  /-"   .'       \__.'          
    :  :         ,\""       ; .'    .'      .-""              
   _J  ;         ; `.      /.'    _/    \.-"                  
  /  "-:        /"--.b-..-'     .'       ;                    
 /     /  ""-..'            .--'.-'/  ,  :                    
:`.   :     / : bug         `-i" ,',_:  _ \                   
:  \  '._  :__;             .'.-"; ; ; j `.l                  
 \  \          "-._         `"  :_/ :_/                       
  `.;\             "-._                                       
    :_"-._             "-.                                    
      `.  l "-.     )     `.                                  
        ""^--""^-. :        \                                 
                  ";         \                                
                  :           `._                             
                  ; /    \ `._   ""---.                       
                 / /   _      `.--.__.'                       
                : :   / ;  :".  \                             
                ; ;  :  :  ;  `. `.                           
               /  ;  :   ; :    `. `.                         
              /  /:  ;   :  ;     "-'                         
             :_.' ;  ;    ; :                                 
                 /  /     :_l                                 
                 `-'

    la golfeta
		|
		|
		|
	   \/                                     
                    /\\      _____      
     ,-----,       /  \\____/__|__\_   
  ,--'---:---`--, /  |  _     |     `|
 ==(o)-----(o)==J    `(o)-------(o)=    
``````````````````````````````````````
*/

/*





*/