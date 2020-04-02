#include "base_de_datos.h"

char lineas[CANT_USUARIOS * 3][LINE_SIZE];

Usuario* usuarios[CANT_USUARIOS];

uint32_t conectar() {

	FILE *file;
	file = fopen("usuarios", "r");

	if ( file != NULL ) {
      char line[LINE_SIZE];

			uint32_t index = 0;
      while ( fgets( line, LINE_SIZE, file ) != NULL ) {
				strcpy(lineas[index], line);
				index++;
      }
      fclose ( file );
  }
	else
		return 1;

	for(uint32_t i = 0; i < CANT_USUARIOS * 3;) {
		uint32_t usuario_index = i / CANT_USUARIOS;
		usuarios[usuario_index] = malloc(sizeof(Usuario));

		strcpy(usuarios[usuario_index]->nombre, lineas[i]);
		i++;
		strcpy(usuarios[usuario_index]->clave, lineas[i]);
		i++;
		strcpy(usuarios[usuario_index]->bloqueado, lineas[i]);
		i++;
	}

	return 0;
}

/**
 * Retorna un puntero char con los nombres de todos los usuarios
 * Formato: nombre1 + \n + nombre2 + \n nombre3 + \n
 */
char* get_nombres() {

	uint32_t size = 0;
	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		size = size + strlen(usuarios[i]->nombre);

	char* nombres = malloc(	size );

	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		strcat(nombres, usuarios[i]->nombre);

	return nombres;
}

/**
 * Retorna un puntero char con las claves de todos los usuarios
 * Formato: clave1 + \n + clave2 + \n clave3 + \n
 */
char* get_claves() {

	uint32_t size = 0;
	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		size = size + strlen(usuarios[i]->clave);

	char* claves = malloc( size );

	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		strcat(claves, usuarios[i]->clave);

	return claves;
}

/**
 * Retorna un puntero char con los estados de todos los usuarios
 * Formato: bloqueado1 + \n + bloqueado2 + \n bloqueado3 + \n
 */
char* get_bloqueados() {

	uint32_t size = 0;
	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		size = size + strlen(usuarios[i]->bloqueado);

	char* bloqueados = malloc(	size );

	for(uint32_t i = 0; i < CANT_USUARIOS; i++)
		strcat(bloqueados, usuarios[i]->bloqueado);

	return bloqueados;
}

uint32_t bloquear(uint32_t usuario_index) {

	usuarios[usuario_index]->bloqueado[0] = '1';

	FILE *file;
	file = fopen("usuarios", "w+");

	if ( file != NULL ) {

			for(uint32_t i = 0; i < CANT_USUARIOS; i++) {
				fputs(usuarios[i]->nombre, file);
				fputs(usuarios[i]->clave, file);
				fputs(usuarios[i]->bloqueado, file);
			}
      fclose ( file );
			return 0;
  }
	else
		return 1;
}
