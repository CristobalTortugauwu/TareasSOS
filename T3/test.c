#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
  
#include "pss.h"
#include "batch.h"
int *ap;

typedef struct job{
  //... defina aca la estructura de Job ...
  JobFun *fun; //Para guardar la funcion
  void * param; //Almacena los parametros de la funcion
  void * res; //para guardar la respuesta de la funcion
  int ready;          //Patrón 
  pthread_cond_t c;   //request
  int msg; //1 significa que no va a realizar mas trabajos
} Job;

//Variables globales 
Queue* prioq;  //Cola para las prioridades
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_t *Gpid; //variable global para recordar los pid
int numThreads; //cantidad de threads
int busy;

//función para consumir los jobs que producirá la funcion submitjob
void *threadFun(void* ptr) {
  Queue* q=ptr;
  for(;;){
    Job *p_job = peek(q);
    //si la cola está vacía, o un thread tiene un trabajo vacio
    if (p_job == NULL || (p_job->msg==1))  
      return NULL;
    //en esta parte nos encargamos de consumir
    pthread_mutex_lock(&m);  
    p_job->res = (*(p_job->fun))(p_job->param); //utilizamos la funcion
    p_job->ready = 1;
    pthread_mutex_unlock(&m);
  }
}

void startBatch(int n) {
  //necesitamos crear el buffer, e inicializar los threads
  numThreads = n;
  prioq = makeQueue();
  pthread_t pid[n];
  busy = 0;
  Gpid = pid; //Queremos que apunte al inicio del arreglo 
  for(int k=0; k<numThreads; k++){
    //consumidores
    pthread_create(&pid[k], NULL, threadFun, prioq);
  }
}

int main() {
    int arr[10];
    for(int k=0;k<10;k++)
        arr[k]=1;
    ap = arr;
    for(int k=0;k<10;k++)
        printf("sexo %d\n",ap[k]);   
    startBatch(1);
 
}