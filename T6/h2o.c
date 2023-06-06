#include <pthread.h>

#include <stdio.h>
#include "pss.h"
#include "spinlocks.h"
#include "h2o.h"

typedef struct {
  int * wait;
  void * elem;
  H2O* res;
} Request;

Queue * oxiq;
Queue * hydroq;
int mutex; //sp para mutex

void initH2O(void) {
  oxiq = makeQueue();
  hydroq = makeQueue();
  mutex = OPEN;
}
void endH2O(void) {
  destroyQueue(oxiq);
  destroyQueue(hydroq);
}

H2O *combineOxy(Oxygen *o){
  spinLock(&mutex);
  if(queueLength(hydroq)>1){
    Request *h1 = get(hydroq);
    Request *h2 = get(hydroq);
    H2O * final = makeH2O(h1->elem,h2->elem,o);
    h1->res = final; h2->res = final;
    spinUnlock(h1->wait); spinUnlock(h2->wait);
    spinUnlock(&mutex);
    return final;
  }
  else{
    int w = CLOSED;
    Request req = {&w,o};
    put(oxiq,&req);
    spinUnlock(&mutex);
    spinLock(req.wait);
    H2O* agua=req.res;
    return agua;
  }
}

H2O *combineHydro(Hydrogen *h){
  spinLock(&mutex);
  if(!emptyQueue(oxiq) && queueLength(hydroq)>0){
    Request *h1 = get(hydroq);
    Request *o = get(oxiq);
    H2O * final = makeH2O(h1->elem,h,o->elem);
    h1->res = final; o->res=final;
    spinUnlock(h1->wait); spinUnlock(o->wait);
    spinUnlock(&mutex);
    return final;
  }
  else{
    int w = CLOSED;
    Request req = {&w,h};
    put(hydroq,&req);
    spinUnlock(&mutex);
    spinLock(req.wait);
    H2O * agua = req.res;
    return agua;
  }
}