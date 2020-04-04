#include "../include/utilities.h"

uint32_t qid;
/**
 * Creacion de cola
 * Retorna el id de la misma (puede ser -1 : error)
 */
uint32_t get_cola() {

  key_t qkey = ftok(QUEUE_FILE_NAME, PROJ_ID);

  if (qkey == -1){
    perror("Obteniendo token: ");
    exit(EXIT_FAILURE);
  }

  qid = msgget(qkey, 0666 | IPC_CREAT);
  return qid;
}

/**
 * Envio de mensaje a cola
 */
uint32_t enviar_a_cola(long id_mensaje, char mensaje[QUEUE_MESAGE_SIZE]) {
  struct msgbuf mensaje_str;
  mensaje_str.mtype = id_mensaje;
  strcpy(mensaje_str.mtext, mensaje);
  return msgsnd(qid, &mensaje_str, sizeof mensaje_str.mtext, 0 );
}

/**
 * Recepcion de mensaje de la cola
 */
char* recibir_de_cola(long id_mensaje) {
  struct msgbuf mensaje_str = {id_mensaje, {0}};
  if(msgrcv(qid, &mensaje_str, sizeof mensaje_str.mtext, id_mensaje, 0) == -1)
    return "n";
  else {
    char* mensaje = malloc(strlen(mensaje_str.mtext));
    strcpy(mensaje, mensaje_str.mtext);
    return mensaje;
  }
}
