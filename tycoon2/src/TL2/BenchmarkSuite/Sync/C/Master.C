#include <stdio.h>
#include <stdlib.h>
#include "MS.h"

Master::Master(WorkQueue * q, int i)
{
  workQueue = q;
  items = i;
  workList = (WorkItem**)malloc(i * sizeof(WorkItem));
}

void * Master::run(void * p)
{
  int i = 0;
  for(i = 0; i < items; i++) {
    WorkItem * w = new WorkItem(i);
    workQueue->addWork(w);
    workList[i] = w;
  }
  return NULL;
}

