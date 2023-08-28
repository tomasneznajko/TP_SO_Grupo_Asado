#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include "cliente.h"
#include "server.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/error.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <ftw.h>

extern char **vectorDePathsPCBs;
extern t_config* superBloque;
extern t_bitarray *bitmap;
extern int cantidadPaths;
extern void* memoriaMapeada;
extern int conexion_memoria;

void terminar_programa(t_log* logger, t_config* config,int fd_bitmap);
int recorrerFCBs();
char* concatenarCadenas(const char* str1, const char* str2);
int contarArchivosEnCarpeta(const char *carpeta, char ***vectoreRutas);
int abrirArchivo(char *nombre, char **vectorDePaths,int cantidadPaths);
int crearArchivo(char *nombre,char *carpeta, char ***vectoreRutas, int *cantidadPaths);
int truncarArchivo(char *nombre,char *carpeta, char **vectoreRutas, int cantidadPaths, int tamanioNuevo);
int dividirRedondeando(int numero1 , int numero2);
void sacarBloques(int cantidadBloquesOriginal ,int cantidadBloquesNueva,t_config* configArchivoActual,int tamanioOriginal);
void agregarBloques(int cantidadBloquesOriginal ,int cantidadBloquesNueva,t_config* configArchivoActual);
void iniciarArchivoBitmap();
bool accesoBitmap(t_bitarray* bitmapAAcceder, off_t bit_index);
void setearBitmap(t_bitarray* bitmapAAcceder, off_t bit_index);
void sincronizarBitmap();
void revisarBitmap(int hastaDonde);
void limpiarBitmap(t_bitarray* bitmapAAcceder, off_t bit_index);
void* concatPunteros(void* ptr1, void* ptr2, size_t size1, size_t size2);
void moverPunteroAbloqueDelArchivo(FILE* bloques, t_config* configArchivoActual,int bloqueBuscado);
void moverPunteroABloquePunteros (FILE* bloques, t_config* configArchivoActual);
void *leerArchivo(char *nombreArchivo,size_t punteroSeek,size_t bytesALeer, int direccion);
int escribirArchivo(char *nombreArchivo,size_t punteroSeek,size_t bytesAEscribir,int direccion,void *memoriaAEscribir);
int cantidadDeBloquesAAcceder(t_config *archivoActual,size_t punteroAInformacion,size_t bytesAOperar);
#endif /* FILESYSTEM_H_ */
