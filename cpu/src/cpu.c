#include "cpu.h"

int main(int argc, char** argv) {

	if (argc < 2) {
		return EXIT_FAILURE;
	}
	conexion_kernel = -1;
	conexion_memoria = -1;
	int socket_servidor = -1;

	logger = iniciar_logger("./cpu.log", "CPU");
	config = iniciar_config(argv[1]);

	char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

	iniciar_diccionario_instrucciones();
	proceso = malloc(sizeof(pcb));
	proceso->instrucciones = list_create();
	proceso->tabla_segmentos = list_create();

	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	enviar_mensaje("Intento de conexión del cpu a la memoria", conexion_memoria, MENSAJE);
	esperar_servidor(conexion_memoria);
	enviar_operacion(conexion_memoria, CPU);
	
	socket_servidor = iniciar_servidor(puerto_escucha);
	esperar_cliente(socket_servidor);

	liberar_conexion(conexion_memoria);
	terminar_programa(logger, config);
	return EXIT_SUCCESS;
}

void terminar_programa(t_log* logger, t_config* config)
{
	if (logger != NULL) {
		log_destroy(logger);
	}
	if (config != NULL) {
		config_destroy(config);
	}
}
