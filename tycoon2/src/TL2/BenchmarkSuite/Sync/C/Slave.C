#include <stdio.h>
#include <stdlib.h>
#include "MS.h"

Slave::Slave(WorkQueue * queue)
{
  workQueue = queue;
}
	
void * Slave::run(void * p)
{
  int loop = 1;
  while(loop) {
    WorkItem * w = workQueue->getWork();
    if(w == NULL) {
      loop = 0;
    }
    else {
      w->eval();
    }
  }
  return NULL;
}

