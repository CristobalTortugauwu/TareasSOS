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
Queue *fifoq;  //Cola para las prioridades
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
    Job *p_job = get(q);
    //En esta parte verificamos los jobs nulos, para terminar la ejecución del thread 
    if (p_job==NULL) {
      pthread_mutex_unlock(&m);
      return NULL;
    }  
    //en esta parte nos encargamos de consumir y que lo realicen en paralelo
    if (p_job != NULL && (p_job->fun!=NULL)) { 
      pthread_mutex_unlock(&m); 
      p_job->res = (*(p_job->fun))(p_job->param); //utilizamos la funcion
      pthread_mutex_lock(&m); 
      p_job->ready = 1;
      pthread_cond_signal(&p_job->c);
    }
    pthread_mutex_unlock(&m);    
  }
}

void startBatch(int n) {
  //necesitamos crear el buffer, e inicializar los threads
  numThreads = n;
  fifoq = makeQueue();
  pthread_t *pid = malloc(n*sizeof(pthread_t)); //Para que sea visible fuera de esta funcion
  Gpid = pid; //Queremos que apunte al inicio del arreglo 
  for(int k=0; k<numThreads; k++){
    //consumidores
    pthread_create(&Gpid[k], NULL, threadFun, fifoq);
  }
  init = 1;
}

void stopBatch() {
  for(int k=0; k<numThreads ;k++) {
    pthread_mutex_lock(&m);
    put(fifoq,NULL); //Para cada thread, le enviamos un job vacio, para que no siga trabajando
    pthread_mutex_unlock(&m);  
  }
  pthread_cond_broadcast(&glob_c);
  for(int p=0; p<numThreads; ++p){
    pthread_join(Gpid[p], NULL);
  }  
  destroyQueue(fifoq);
  free(Gpid);  
}

Job *submitJob(JobFun fun, void *input) {
  //Hay que obtener la info de la función
  //Rellenamos el job con la info necesaria
  Job job = {fun, input, NULL, 0, PTHREAD_COND_INITIALIZER}; 
  Job *p_job = malloc(sizeof(Job));
  *p_job = job;
  pthread_mutex_lock(&m);
  put(fifoq, p_job); //colocamos el job en la cola
  pthread_cond_broadcast(&glob_c); //debería despertar al consumidor
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
  pthread_mutex_unlock(&m);
  //En este punto la información del job ya fue actualizada por el consumidor
  void * res = job->res;
  free(job);
  return res;
}

