#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "MS.h"


WorkQueue::WorkQueue()
{
  queue = new List();
  slaveWaiting = 0;
  finished = 0;
  mx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mx, NULL);
  cv = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(cv, NULL);
}

void WorkQueue::addWork(WorkItem * work)
{
  pthread_mutex_lock(mx);
  queue = queue->addElement(work);
  if(slaveWaiting > 0) {
    pthread_cond_broadcast(cv);
  }
  pthread_mutex_unlock(mx);
}


WorkItem * WorkQueue::getWork()
{
  pthread_mutex_lock(mx);
  while(queue->isEmpty() && !finished) {
    slaveWaiting = slaveWaiting + 1;
    pthread_cond_wait(cv, mx);
    slaveWaiting = slaveWaiting - 1;
  }
  if(queue->isEmpty() && finished) {
    pthread_mutex_unlock(mx);
    return NULL;
  }
  else {
    WorkItem * w = (WorkItem*)queue->head();
    queue = queue->tail();
    pthread_mutex_unlock(mx);
    return w;
  }
}

void WorkQueue::setFinished()
{
  pthread_mutex_lock(mx);
  finished = 1;
  pthread_cond_broadcast(cv);
  pthread_mutex_unlock(mx);
}
