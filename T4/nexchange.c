#include "nthread-impl.h"

typedef struct {
  void* msg; //msje que se quiere enviar
  nThread waited; //el nThread que se está esperando
} Args; 

void* nExchange(nThread th, void *msg, int timeout) {
  START_CRITICAL
  nThread this_nth = nSelf();
  //El nThread al que le quiero enviar un mensaje
  //está ocupado
  Args args;
  args.msg = msg; //mensaje que quiero enviar
  args.waited = th; //a quién le quiero enviar un mensaje
  this_nth->ptr = &args; //almaceno estos valores en el thread, para cuando me llamen
  Args * this_ptr = this_nth->ptr;
  if(th->status == WAIT_EXCHANGE){
    Args * th_ptr = th->ptr;
    if(this_nth == th_ptr->waited){ //solo si son iguales lo tiene que despertar con setReady
      void * fin_msg = th_ptr->msg; //rescatamos el msg
      th_ptr->msg=this_ptr->msg; //le entregamos nuestro mensaje
      setReady(th);
      schedule();
      END_CRITICAL
      return fin_msg;
    }
    else //no son el mismo nThread
      suspend(WAIT_EXCHANGE);
  }
  else{ //status run or ready
    suspend(WAIT_EXCHANGE);
  }
  schedule();
  void * fin_msg = this_ptr->msg; //rescatamos el msg
  END_CRITICAL
  return fin_msg;
}
