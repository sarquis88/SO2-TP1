#include "../include/files_service.h"

// definicion de variables
int32_t newsockfd, qid, sockfd, pid, puerto, qid;
ssize_t n;
uint32_t clilen;
struct sockaddr_in serv_addr, cli_addr;
Archivo* archivos[CANT_ARCHIVOS];
char buffer[BUFFER_SIZE], impresion[BUFFER_SIZE], direccion[16];

/**
 * Funcion main
 */
int32_t main( int32_t argc, char *argv[] ) {

	// chequeo de argumentos
	if ( argc < 3 ) {
		sprintf(impresion, "Uso: %s <direccion ip> <puerto>\n", argv[0]);
    imprimir(1);
		exit(1);
	}

	// definicion de puerto y direccion
	sprintf(direccion, argv[1]);
	puerto = atoi( argv[2] ) + 1;

	// configuracion de socket
	configurar_socket();

	// levantar base de datos
	if(conectar()) {
		sprintf(impresion, "error leyendo archivos\n");
		imprimir(1);
		exit(1);
	}

	// creacion de cola
	qid = get_cola('f');
	if(qid == -1) {
		sprintf(impresion, "error creando cola\n");
		imprimir(1);
		exit(1);
	}
	sprintf(impresion, "cola = %d\n", qid);
	imprimir(0);

	// empezar a escuchar mensajes en cola y por socket
	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );
  while(1) {

		// recibir cualquier tipo de mensajes
		struct msgbuf mensaje_str = recibir_de_cola((long) 0, 'f');

    // handler para archivos request
		if(mensaje_str.mtype == ARCHIVOS_REQUEST) {
			sprintf(impresion, "archivos request\n");
			imprimir(0);

      char* primero = "<Indice> - <Nombre> - <Formato> - <Tamaño [MB]> - <Hash>\n";
      size_t size = strlen(primero);
      char* salto = "\n";
      char* guion = " - ";
      char index[2];
			char size_archivo[ARCHIVO_NOMBRE_SIZE];
      for(int32_t i = 0; i < CANT_ARCHIVOS; i++) {
				sprintf(size_archivo, "%ld", archivos[i]->size);

        size = size + strlen(index) + strlen(guion) * 4 +
        strlen(archivos[i]->nombre) + strlen(archivos[i]->formato) +
				strlen(archivos[i]->hash) + strlen(size_archivo);
				if(i < CANT_ARCHIVOS - 1)
        	size = size + strlen(salto);
			}

      char files[size];
      sprintf(files, primero);
      for(int32_t i = 0; i < CANT_ARCHIVOS; i++) {

				char tmp[strlen(files)];
				sprintf(tmp, files);
        sprintf(files, "%s%d%s%s%s%s%s%ld%s%s", 	tmp,
																								archivos[i]->index,
																								guion,
																								archivos[i]->nombre,
																								guion,
																								archivos[i]->formato,
																								guion,
																								archivos[i]->size,
																								guion,
																								archivos[i]->hash);
				if(i < CANT_ARCHIVOS - 1)
        	strcat(files, salto);
      }

			enviar_a_cola_local((long) ARCHIVOS_RESPONSE, files, 'p');
			sprintf(impresion, "archivos response\n");
			imprimir(0);
		}

		// handler para descarga request
		if(mensaje_str.mtype == DESCARGA_REQUEST) {
			sprintf(impresion, "descarga request\n");
			imprimir(0);

			int32_t index_descarga = atoi(mensaje_str.mtext);
			int32_t flag = 0;
			for(int32_t i = 0; i < CANT_ARCHIVOS; i++) {
				if(archivos[i]->index == index_descarga) {
					flag = 1;
					break;
				}
			}

			sprintf(impresion, "descarga response\n");
			imprimir(0);

			if(flag == 0)
				enviar_a_cola_local((long) DESCARGA_RESPONSE, "descarga_no", 'p');
			else {
				enviar_a_cola_local((long) DESCARGA_RESPONSE, "descarga_si", 'p');

				// empezar a escuchar
				newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
				enviar_archivo(index_descarga);
				close(newsockfd);
			}
		}
	}
	exit(0);
}

/**
 * Levantamiento de socket
 */
void configurar_socket() {
	sockfd = socket( AF_INET, SOCK_STREAM, 0);
	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(direccion);
	serv_addr.sin_port = htons( (uint16_t) puerto );
	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		sprintf(impresion, "error conectando socket\n");
		imprimir(1);
		exit(1);
	}
	else {
		sprintf(impresion, "iniciando\n");
		imprimir(0);
		sprintf(impresion, "proceso: %d - puerto: %d\n", getpid(), ntohs(serv_addr.sin_port));
		imprimir(0);
	}
}

/**
 * Conexion a base de datos
 * Creacion de usuarios
 */
int32_t conectar() {

  DIR *dir;
  struct dirent *ent;
  int32_t index = 0;
  if ((dir = opendir (FILES_DIR_NAME)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      if( ent->d_name[0] != '.' ) {
				sprintf(impresion, "configurando archivo nro %d...\n", index);
				imprimir(0);

        archivos[index] = malloc(sizeof(Archivo));

				char path[strlen(FILES_DIR_NAME) + strlen(ent->d_name) + 1];
				sprintf(path, "%s%s", FILES_DIR_NAME, ent->d_name);
				path[strlen(path)] = '\0';

        archivos[index]->index = index;
    		sprintf(archivos[index]->nombre, strtok(ent->d_name, "."));
    		archivos[index]->nombre[strlen(archivos[index]->nombre)] = '\0';
    		sprintf(archivos[index]->formato, strtok(NULL, "."));
    		archivos[index]->formato[strlen(archivos[index]->formato)] = '\0';

				FILE* file = fopen(path, "rb");
				if(file != NULL) {

					fseek(file, 0L, SEEK_END);
					ssize_t size = ftell(file);
					if(size < 0)
						size = INT_MAX;
					else
						archivos[index]->size = size / 1048576;

					fclose(file);
				}

				sprintf(archivos[index]->hash, get_md5(path, 0));
        index++;

				sprintf(impresion, "configuracion terminada\n");
				imprimir(0);
      }
    }
    closedir (dir);
  }
  else {
		sprintf(impresion, "error creando archivos\n");
		imprimir(1);
    return EXIT_FAILURE;
  }
	return 0;
}

/**
 * Enviar mensaje a cola de mensaje
 */
void enviar_a_cola_local(long id, char* mensaje, char proceso) {
	if(enviar_a_cola(id, mensaje, proceso) == -1) {
		sprintf(impresion, "error enviando mensaje\n");
		imprimir(1);
		exit(1);
	}
}

/**
 * Imprimir en consola
 */
void imprimir(int32_t error) {
		printf("\33[1;32m");
		if(error)
			fprintf(stderr, "FILES_SERVICE: %s", impresion );
		else
			printf("FILES_SERVICE: %s", impresion);
		fflush(stdout);
		printf("\033[0m");
}

/**
 * Recibe datos y los guarda en buffer
 */
void recepcion() {
	memset( buffer, 0, BUFFER_SIZE );
	n = recv( newsockfd, buffer, BUFFER_SIZE, 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error leyendo de socket\n");
	  imprimir(1);
	  exit(1);
	}
}

/**
 * Envia datos por el socket
 */
void enviar_a_cliente(char* mensaje) {
	n = send( newsockfd, mensaje, strlen(mensaje), 0 );
	if ( n < 0 ) {
		sprintf(impresion, "error enviando a cliente\n");
	  imprimir(1);
	  exit( 1 );
	}
}

/**
 * Enviar archivo a cliente
 */
void enviar_archivo(int32_t index_descarga) {

	char* punto = ".";
	char nombre_archivo[strlen(archivos[index_descarga]->nombre) + strlen(archivos[index_descarga]->formato)
		+ strlen(punto)];
	sprintf(nombre_archivo, "%s%s%s", archivos[index_descarga]->nombre,
																		punto,
																		archivos[index_descarga]->formato);
	nombre_archivo[strlen(nombre_archivo)] = '\0';

	enviar_a_cliente(nombre_archivo);

	char* path_archivo = malloc(strlen(FILES_DIR_NAME) + strlen(nombre_archivo));
	sprintf(path_archivo, "%s%s", FILES_DIR_NAME, nombre_archivo);
	path_archivo[strlen(path_archivo)] = '\0';

	sprintf(impresion, "enviando %s\n", nombre_archivo);
	imprimir(0);

	FILE* file = fopen(path_archivo, "rb");	// rb para archivos de no-texto;
	if ( file != NULL ) {
		while( fread(buffer, sizeof(char), sizeof(buffer), file) > 0 )
			send(newsockfd, buffer, sizeof(buffer), 0);

		sprintf(impresion, "enviado %s\n", nombre_archivo);
		imprimir(0);
		fclose(file);
	}
	else {
		sprintf(impresion, "error abriendo %s\n", nombre_archivo);
		imprimir(1);
	}
	free(path_archivo);
}
