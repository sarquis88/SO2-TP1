#include "../include/auth_service.h"

uint32_t intentos, qid;

char lineas[CANT_USUARIOS * 3][LINE_SIZE];

char* nombres[USUARIO_NOMBRE_SIZE];
char* claves[USUARIO_CLAVE_SIZE];
char* bloqueados[USUARIO_BLOQUEADO_SIZE];

Usuario* usuarios[CANT_USUARIOS];

uint32_t main( uint32_t argc, char *argv[] ) {

	int sockfd, newsockfd, servlen, clilen, n, buf, pid;
  struct sockaddr_un  cli_addr, serv_addr;

	/* Se toma el nombre del socket de la línea de comandos */
  if( argc != 2 ) {
    printf( "Uso: %s <nombre_de_socket>\n", argv[0] );
    exit( 1 );
  }

	if ( ( sockfd = socket( AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    perror( "creación de  socket");
    exit(1);
  }

	/* Remover el nombre de archivo si existe */
	unlink ( argv[1] );

	memset( &serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sun_family = AF_UNIX;
  strcpy( serv_addr.sun_path, argv[1] );
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  if( bind( sockfd,(struct sockaddr *)&serv_addr,servlen )<0 ) {
    perror( "auth_service: ligadura" );
    exit(1);
  }
	else {
		printf("auth_service: Iniciando\n");
		printf("auth_service: Proceso: %d - Socket: %s\n", getpid(), serv_addr.sun_path);
	}

	if(conectar()) {
		perror("auth_service: Error al leer base de datos");
		exit(1);
	}
	else {
		configurar_nombres();
		configurar_claves();
		configurar_bloqueados();
	}

	qid = get_cola('a');
	if(qid == -1) {
		perror("auth_service: creando cola: ");
		exit(1);
	}
	printf("auth_service: cola = %d\n", qid);

  listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

  while(1) {

		struct msgbuf mensaje_str = recibir_de_cola((long) 0, 'a');	// recibir cualquier tipo de mensajes

		// handler para login request
		if(mensaje_str.mtype == LOGIN_REQUEST) {
			uint32_t log = login(mensaje_str.mtext);

			if(log == 0)
				enviar_a_cola((long) LOGIN_RESPONSE, "0", 'p');
			else if(log == 1)
				enviar_a_cola((long) LOGIN_RESPONSE, "1", 'p');
			else if(log == 9)
				enviar_a_cola((long) LOGIN_RESPONSE, "9", 'p');
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/**
 * Conexion a base de datos
 * Creacion de usuarios
 */
uint32_t conectar() {

	FILE *file;
	file = fopen(DATA_FILE_NAME, "r");

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

/**
 * Bloquea a un usuario, escribiendo un 1 en la base de datos
 */
uint32_t bloquear(uint32_t usuario_index) {

	usuarios[usuario_index]->bloqueado[0] = '1';

	FILE *file;
	file = fopen(DATA_FILE_NAME, "w+");

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

/**
 * Formatea los nombres en array
 */
void configurar_nombres() {
	char* auxiliar = strtok(get_nombres(), "\n");
	for(uint32_t i = 0; i < CANT_USUARIOS; i++) {
		nombres[i] = malloc(strlen(auxiliar));
		strcpy(nombres[i], auxiliar);
		auxiliar = strtok(NULL, "\n");
	}
}

/**
 * Formatea las claves en array
 */
void configurar_claves() {
	char* auxiliar = strtok(get_claves(), "\n");
	for(uint32_t i = 0; i < CANT_USUARIOS; i++) {
		claves[i] = malloc(strlen(auxiliar));
		strcpy(claves[i], auxiliar);
		auxiliar = strtok(NULL, "\n");
	}
}

/**
 * Formatea los bloqueados en array
 */
void configurar_bloqueados() {
	char* auxiliar = strtok(get_bloqueados(), "\n");
	for(uint32_t i = 0; i < CANT_USUARIOS; i++) {
		bloqueados[i] = malloc(strlen(auxiliar));
		strcpy(bloqueados[i], auxiliar);
		auxiliar = strtok(NULL, "\n");
	}
}

/**
 *
 */
void bloquear_usuario(char* nombre_usuario) {
	uint32_t usuario_index = 0;
	for(uint32_t i = 0; i < CANT_USUARIOS; i++) {
		if( strcmp(nombre_usuario, nombres[i]) == 0 ) {
			bloquear(i);
			strcpy(bloqueados[i], "1");
			return;
		}
	}
}

uint32_t login(char* credenciales) {

	char* login;

	login = strtok(credenciales, "-");
	char usuario[strlen(login)];
	strcpy(usuario, login);

	login = strtok(NULL, "-");
	char clave[strlen(login)];
	strcpy(clave, login);

	for(uint32_t i = 0; i < CANT_USUARIOS	; i++) {
		if( strcmp(nombres[i], usuario) == 0 ) {
			if( strcmp(claves[i], clave) == 0 ) {
				if( bloqueados[i][0] == '0' )
					return 1;
				else
					return 9;
			}
		}
	}
	return 0;
}
