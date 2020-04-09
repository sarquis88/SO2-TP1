#include "../include/utilities.h"

/**
 * Creacion de cola
 * Retorna el id de la misma (puede ser -1 : error)
 */
int32_t get_cola(char proceso) {

  key_t qkey;
  if( proceso == 'p' )
    qkey = ftok(PRIMARY_QUEUE_FILE_NAME, PROJ_ID);
  else if( proceso == 'f')
    qkey = ftok(FILE_QUEUE_FILE_NAME, PROJ_ID);
  else if( proceso == 'a')
    qkey = ftok(AUTH_QUEUE_FILE_NAME, PROJ_ID);
  else {
    printf("Error obteniendo token: proceso desconocido\n");
    exit(1);
  }

  if (qkey == -1) {
    perror("Obteniendo token: ");
    exit(1);
  }

  return msgget(qkey, 0666 | IPC_CREAT);
}

/**
 * Envio de mensaje a cola
 */
int32_t enviar_a_cola(long id_mensaje, char mensaje[QUEUE_MESAGE_SIZE], char proceso) {

  if(strlen(mensaje) > QUEUE_MESAGE_SIZE) {
    perror("Mensaje muy grande\n");
    exit(1);
  }
  struct msgbuf mensaje_str;
  mensaje_str.mtype = id_mensaje;
  strcpy(mensaje_str.mtext, mensaje);

  int32_t qid = get_cola(proceso);

  return msgsnd(qid, &mensaje_str, sizeof mensaje_str.mtext, 0 );
}

/**
 * Recepcion de mensaje de la cola
 */
struct msgbuf recibir_de_cola(long id_mensaje, char proceso) {
  struct msgbuf mensaje_str = {id_mensaje, {0}};

  int32_t qid = get_cola(proceso);

  if(msgrcv(qid, &mensaje_str, sizeof mensaje_str.mtext, id_mensaje, 0) == -1) {
      perror("Recibiendo mensaje de cola: ");
      exit(1);
  }
  else {
    return mensaje_str;
  }
}

/**
 * Retona el hash md5 del archivo ingresado
 */
char* get_md5(char* file_path) {

		FILE* file = fopen(file_path, "rb");	// rb para archivos de no-texto;
    char* buffer[BUFFER_SIZE];
    int32_t n;

    unsigned char c[MD5_DIGEST_LENGTH];
		MD5_CTX mdContext;

    MD5_Init (&mdContext);
		while (( n = fread (buffer, sizeof(char), sizeof(buffer), file) ) != 0)
        MD5_Update (&mdContext, buffer, n);
    MD5_Final (c,&mdContext);

		fclose(file);

    char* md5string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    for (int32_t i = 0; i < MD5_DIGEST_LENGTH; ++i)
      sprintf(&md5string[i * 2], "%02x", (unsigned int)c[i]);

    md5string[strlen(md5string)] = '\0';

		return md5string;
}
