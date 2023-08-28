#ifndef LOGCONFIG_H_
#define LOGCONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/error.h>

extern t_log* logger;
extern t_config* config;

t_log* iniciar_logger(char*, char*);
t_config* iniciar_config(char*);
void iterator(char*);

#endif /* LOGCONFIG_H_ */
