#include <pthread.h>
#include <stdio.h>
#include "suma.h"
#define P 8 

Set buscarThread(int a[], int n, int ini, int end) {
  for (Set k= ini; k<=end; k++) {
    // k es el mapa de bits para el subconjunto { a[i] | bit ki de k es 1 }
    long long sum= 0;
    for (int i= 0; i<n; i++) {
      if ( k & ((Set)1<<i) ) // si bit ki de k es 1
        sum+= a[i];
    }
    if (sum==0) {  // exito: el subconjunto suma 0
      return k;    // y el mapa de bits para el subconjunto es k
  } }
  return 0;        // no existe subconjunto que sume 0
}


// Defina aca las estructuras que necesite

typedef struct {
    Set str, end, n, res;
    int *arr;
} Args;

// Defina aca la funcion que ejecutaran los threads

void *thread (void *a) {
    Args * args = (Args*)a;
    Set ini = args->str;
    Set end = args->end;
    int n = args->n;
    int *arr = args->arr;
    args->res = buscarThread(arr, n, ini, end);
    return NULL;
}

Set minor_set(Set *arr, int size) {
  Set minor = 0;
  for(int k =0;k<size;k++) {
      if (arr[k]!= 0)
          minor = arr[k];
  }
  return minor;
}

// Reprograme aca la funcion buscar
Set buscar(int a[], int n) {
    pthread_t pid[P]; //Identificadores
    Args args[P]; //Argumentos para cada thread
    Set comb= (1<<(n-1)<<1)-1; // 2n-1: nro. de combinaciones
    Set intervalo = comb/8; //para segmentar el intervalo
    for(int i =0;i<P;i++) {
        args[i].str =  1+ intervalo*i;
        args[i].end =  intervalo*(1+i);
        printf("el intervalo es [%lld,%lld]\n", args[i].str, args[i].end);
        args[i].arr = a;
        args[i].n = n;
        pthread_create(&pid[i],NULL , thread ,&args[i]);
    }

    Set minor =0;
    for(int i = 0; i<P ;i++) {
        pthread_join(pid[i],NULL);
        if(args[i].res!=0) {
            if (minor == 0 ) {
              minor = args[i].res;
            }
            else {
                if (minor>args[i].res) {
                    minor=args[i].res;
                }    
            }
        }    
    }
    return minor;

}

