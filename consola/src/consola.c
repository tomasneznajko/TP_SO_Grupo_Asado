#include "consola.h"

int main(int argc, char** argv)
{
	if (argc < 3) {
        return EXIT_FAILURE;
	}
	int conexion_kernel = -1;

	logger = iniciar_logger("./consola.log", "Consola");
	config = iniciar_config(argv[1]);

	char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
	char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

	// Creamos una conexión hacia el servidor
	conexion_kernel = crear_conexion(ip_kernel, puerto_kernel);

	// Armamos y enviamos el paquete
	paquete_texto(conexion_kernel, argv[2]);

	// Esperando respuesta
	atender_kernel(conexion_kernel);

	// liberar conexion y variables
	liberar_conexion(conexion_kernel);
	terminar_programa(logger, config);
	return EXIT_SUCCESS;
}

/*
void leer_consola(t_log* logger)
{
	char* leido;

	// La primera te la dejo de yapa
	while(1) {
		leido = readline("> ");
	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
		if (leido[0]=='\0') {
			break;
		}
		log_info(logger, "%s", leido);
	// ¡No te olvides de liberar las lineas antes de regresar!
		free(leido);
	}
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete = crear_paquete(PAQUETE);

	// Leemos y esta vez agregamos las lineas al paquete
	while(1) {
		leido = readline("> ");
		if (leido[0]=='\0') {
			break;
		}
		agregar_a_paquete(paquete, leido, strlen(leido)+1);
		free(leido);
	}
	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	eliminar_paquete(paquete);

}
*/

void paquete_texto(int conexion, char* pseudocodigo)
{
	char leido[64];
	char* parsed;
	t_paquete* paquete = crear_paquete(NEW);
	FILE* fptr = fopen(pseudocodigo, "r");
	if (fptr == NULL) {
		printf("¡No se pudo abrir el archivo!\n");
		abort();
	}
	unsigned int pid = process_getpid();
	log_info(logger, "Proceso %u iniciado.", pid);
	agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
	while(NULL !=fgets(leido, 64, fptr)) {
		parsed = strtok(leido, "\n");
		replace_r_with_0(parsed);
		agregar_a_paquete(paquete, parsed, strlen(parsed)+1);
	}
	enviar_paquete(paquete, conexion);
	// ¡No te olvides de cerrar el archivo y liberar paquete antes de regresar!
	fclose(fptr);
	eliminar_paquete(paquete);
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
