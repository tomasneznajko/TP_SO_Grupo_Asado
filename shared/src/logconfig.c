#include "logconfig.h"

t_log* logger;
t_config* config;

t_log* iniciar_logger(char* log_file, char* process_name)
{
	t_log* nuevo_logger;
	if ((nuevo_logger = log_create(log_file, process_name, 1, LOG_LEVEL_INFO)) == NULL) {
	    error_show("¡No se pudo crear el logger!");
	    exit(1);
	}
	return nuevo_logger;
}

t_config* iniciar_config(char* config_file)
{
	t_config* nuevo_config;
	if ((nuevo_config = config_create(config_file)) == NULL) {
	    error_show("¡No se pudo crear el config!");
	    exit(1);
	}
	return nuevo_config;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
