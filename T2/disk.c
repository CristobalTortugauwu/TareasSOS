#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#define MAXTRACK 100

//... defina aca sus variables globales ...
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int last_track; //almacena el último track solicitado
//int turns[MAXTRACK];
int busy;
int arr_tracks[MAXTRACK]; //arreglo que almacena la cantidad de threads que están esperando

//Esta función entrega -1 en caso de que hayan threads esperando
//antes que el mine
//en el arreglo global, caso contrario entrega el índice

int antesEnEspera(int *arr, int n, int last,int mine) {

  for(int i =last;i<n;i++) {
    //Si hay alguien esperando, y cumple con la condicion
    if(arr[i]!=0) {
      //Si es el mismo track, entonces puedo despertar
      if (mine == i) {
        if(busy)
          return 0;
        else{
          return -1;
          }
      }
      //si hay alguien antes que yo, no puedo entrar
      else {
        return 0;
      }
    }
  }
  for(int k=0;k<last;k++){    
    if(arr[k]!=0) {
      if (mine ==k)
        return -1;
      return 0;
    }
  }
  return -1;
}

void requestDisk(int track) {
  //... complete ...
  pthread_mutex_lock(&mutex);
  //si last_track es igual a -1, nadie ha entrado
  if(last_track==(-1)) {
    busy=1;
    last_track=track;
  }
  //en caso contrario deben esperar
  else {  
    arr_tracks[track]++; 
    while (antesEnEspera(arr_tracks,MAXTRACK,last_track,track)!=(-1))
      pthread_cond_wait(&c,&mutex);  
    busy=1;  
    last_track=track;
  }
  pthread_mutex_unlock(&mutex);
  return;
}

void releaseDisk(void) {
  pthread_mutex_lock(&mutex);
  //busy=0; 
  if(arr_tracks[last_track]!=0) {
    busy=0;
    //if (arr_tracks[last_track]>1)
    arr_tracks[last_track]--;
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&mutex);
    return; 
  } 
  busy=0;
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&mutex);
  return;
}
void diskInit(void) {
  last_track = (-1); //Nadie está esperando
  for (int k = 0; k<MAXTRACK;k++){
    arr_tracks[k]=0;
  }  
}

void diskClean(void) {
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&c);  
}