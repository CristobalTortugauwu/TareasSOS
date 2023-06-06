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
//versión antigua


typedef struct job{
  //... defina aca la estructura de Job ...
  JobFun fun; //Para guardar la funcion
  void * param; //Almacena los parametros de la funcion
  void * res; //para guardar la respuesta de la funcion
  int ready;          //Patrón 
  pthread_cond_t c;   //request
  int msg; //1 significa que no va a realizar mas trabajos
} Job;

//Variables globales 
Queue *prioq;  //Cola para las prioridades
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t glob_c = PTHREAD_COND_INITIALIZER; //Condicion para el threadFun
pthread_t *Gpid; //variable global para recordar los pid
int numThreads; //cantidad de threads
int init=0; //para saber si se inicializó el startbatch


//función para consumir los jobs que producirá la funcion submitjob
void* threadFun(void* ptr) {
  Queue* q=ptr;
  for(;;){
    pthread_mutex_lock(&m);
    while(emptyQueue(q) || !init){ //tengo que esperar si la cola está vacía o si se inició el batch
      pthread_cond_wait(&glob_c,&m);
    }   
    //pthread_mutex_unlock(&m);
    //pthread_mutex_lock(&m);
    Job *p_job = peek(q);
    if (p_job == NULL) //según yo es redundante xq si salí del while, ya sé que la cola no está vacia
      return NULL;
    //en esta parte nos encargamos de consumir
    if (p_job != NULL && (p_job->msg!=1)) { 
      p_job->res = (*(p_job->fun))(p_job->param); //utilizamos la funcion
      p_job->ready = 1;
      pthread_cond_signal(&p_job->c);
    }
    //si la cola está vacía, o un thread tiene un trabajo vacio
    if ((p_job->msg==1)){  
      pthread_mutex_unlock(&m);
      return NULL;
    }  
    pthread_mutex_unlock(&m);
  }
}

void startBatch(int n) {
  //necesitamos crear el buffer, e inicializar los threads
  numThreads = n;
  prioq = makeQueue();
  pthread_t pid[n];
  Gpid = pid; //Queremos que apunte al inicio del arreglo 
  for(int k=0; k<numThreads; k++){
    //consumidores
    pthread_create(&Gpid[k], NULL, threadFun, prioq);
  }
  init = 1;
}

void stopBatch() {
  //Creamos un trabajo vacio
  Job null_job = {NULL, NULL, NULL, -1, PTHREAD_COND_INITIALIZER,1}; 
  Job *p_job = malloc(sizeof(Job));
  *p_job = null_job;
  for(int k=0; k<numThreads ;k++) 
    put(prioq,p_job); //Para thread, le enviamos un job vacio, para que no siga trabajando
  
  for(int p=0; p<numThreads; ++p)
    pthread_join(Gpid[p], NULL);

  destroyQueue(prioq);  
}

Job *submitJob(JobFun fun, void *input) {
  //Hay que obtener la info de la función
  //Rellenamos el job con la info necesaria
  pthread_mutex_lock(&m);
  Job job = {fun, input, NULL, 0, PTHREAD_COND_INITIALIZER,0}; 
  Job *p_job = malloc(sizeof(Job));
  *p_job = job;
  put(prioq,p_job); //colocamos el job en la cola
  pthread_cond_signal(&glob_c); //debería despertar al consumidor
  pthread_mutex_unlock(&m);
  return p_job;
}
/*
asdfasf
*/
void *waitJob(Job *job) {
  pthread_mutex_lock(&m);
  //Debe esperar hasta que el consumidor le avise que esta listo
  while (!job->ready){
    pthread_cond_wait(&job->c,&m);
  }
  //liberar el descriptor, y entregar el resultado del job
  Job *p_job = peek(prioq);
  void * res = p_job->res;
  get(prioq); // Sale de la cola
  free(p_job);
  pthread_mutex_unlock(&m);
  return res;
}

