#include "../include/utilities.h"

/**
 * Creacion de cola
 * Retorna el id de la misma (puede ser -1 : error)
 */
uint32_t get_cola(char proceso) {

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
uint32_t enviar_a_cola(long id_mensaje, char mensaje[QUEUE_MESAGE_SIZE], char proceso) {
  struct msgbuf mensaje_str;
  mensaje_str.mtype = id_mensaje;
  strcpy(mensaje_str.mtext, mensaje);

  uint32_t qid = get_cola(proceso);

  return msgsnd(qid, &mensaje_str, sizeof mensaje_str.mtext, 0 );
}

/**
 * Recepcion de mensaje de la cola
 */
struct msgbuf recibir_de_cola(long id_mensaje, char proceso) {
  struct msgbuf mensaje_str = {id_mensaje, {0}};

  uint32_t qid = get_cola(proceso);

  if(msgrcv(qid, &mensaje_str, sizeof mensaje_str.mtext, id_mensaje, 0) == -1) {
      perror("Recibiendo mensaje de cola: ");
      exit(1);
  }
  else {
    return mensaje_str;
  }
}
