#include "nthread-impl.h"

// Use los estados predefinidos WAIT_EXCHANGE y/o WAIT_EXCHANGE_TIMEOUT
// El descriptor de thread incluye el campo ptr para que Ud. lo use
// a su antojo.

typedef struct {
  void* msg; //msje que se quiere enviar
  nThread waited; //el nThread que se está esperando
  int ready; //valor para saber si en nThread está listo
} Args; 

void* nExchange(nThread th, void *msg, int timeout) {
  START_CRITICAL
  nThread this_nth = nSelf();
  //El nThread al que le quiero enviar un mensaje
  //está ocupado
  Args args;
  args.msg = msg; //mensaje que quiero enviar
  args.waited = th; //a quién le quiero enviar un mensaje
  args.ready =0;
  this_nth->ptr = &args; //almaceno estos valores en el thread, para cuando me llamen
  Args * this_ptr = this_nth->ptr;
  //Hay considerar el nuevo estado.
  if(th->status == WAIT_EXCHANGE || th->status == WAIT_EXCHANGE_TIMEOUT){
    Args * th_ptr = th->ptr; 
    if(this_nth == th_ptr->waited) { //solo si son iguales lo tiene que despertar con setReady
      if(th->status == WAIT_EXCHANGE_TIMEOUT){
        //Lo coloqué después de wait_exchange_timeout, ya que desconozco el funcionamiento del
        //nThread cuando se despierta pasado el tiempo que está durmiendo
        th_ptr->ready = 1; //marcamos con 1, para decir que está listo
        nth_cancelThread(th); 
      }  
      th_ptr->ready = 1; //marcamos con 1, para decir que está listo  
      void * fin_msg = th_ptr->msg; //rescatamos el msg
      th_ptr->msg=this_ptr->msg; //le entregamos nuestro mensaje
      setReady(th);
      schedule(); 
      END_CRITICAL 
      return fin_msg; 
    } 
    else //no son el mismo nThread
          //preguntamos sobre el valor de timeout
      if (timeout<0)
        suspend(WAIT_EXCHANGE);
      else{  // caso timeout>0
        suspend(WAIT_EXCHANGE_TIMEOUT);
        nth_programTimer((long long)timeout * 1000000, NULL);
      }
  }
  else{ //status run or ready
    //preguntamos sobre el valor de timeout
    if (timeout<0)
      suspend(WAIT_EXCHANGE);
    else{  // caso timeout>=0
      suspend(WAIT_EXCHANGE_TIMEOUT);
      nth_programTimer((long long)timeout * 1000000, NULL);
    }
  }
  schedule();
  if(this_ptr->ready == 0 ){ //Si la variable ready es igual a 0, entonces 
                             //no era su turno de despertar
    END_CRITICAL
    return NULL;
  }
  void * fin_msg = this_ptr->msg; //rescatamos el msg
  END_CRITICAL
  return fin_msg;
}
