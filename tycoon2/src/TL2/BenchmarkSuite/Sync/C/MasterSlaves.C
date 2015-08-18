#include <stdio.h>
#include <stdlib.h>
#include "MS.h"


void * startMaster(Master * m)
{
  return m->run(NULL);
}

void * startSlave(Slave * s)
{
  return s->run(NULL);
}


class MasterSlaves {
public:
  int m;
  int s;
  int wpm;
  int debug;
	
  MasterSlaves(int mm, int ss, int wwpm, int d)
    {
      m = mm;
      s = ss;
      wpm = wwpm;
      debug = d;
    }

  void go()
    {
      int i;
      pthread_t * mid = (pthread_t*)malloc(m * sizeof(pthread_t));
      pthread_t * sid = (pthread_t*)malloc(s * sizeof(pthread_t));
      Master ** masters = (Master**)malloc(m * sizeof(Master));
      WorkQueue * queue = new WorkQueue();
      /* start masters */
      for(i = 0; i < m; i++) {
    	Master * ma = new Master(queue, wpm);
    	masters[i] = ma;
	pthread_create(&mid[i], NULL, (void *(*)(void*))startMaster, ma);
      }
      if(debug != 0) { printf("masters started\n"); }
      /* start slaves */
      for(i = 0; i < s; i++) {
    	Slave * sl = new Slave(queue);
    	pthread_create(&sid[i], NULL, (void *(*)(void*))startSlave, sl); 
      }
      if(debug != 0) { printf("slaves started\n"); }
      /* join masters */
      for(i = 0; i < m; i++) {
    	pthread_join(mid[i], NULL);
      }
      if(debug != 0) { printf("masters joined\n"); }
      /* stop queue */
      queue->setFinished();
      if(debug != 0) { printf("queue state reset\n"); }
      /* join slaves */
      for(i = 0; i < s; i++) {
    	pthread_join(sid[i], NULL);
      }
      if(debug != 0) { printf("slaves joined\n"); }
      /* output result */
      if(debug != 0) {
	for(i = 0; i < m; i++) {
	  WorkItem ** it = masters[i]->workList;
	  int j = 0;
	  for(j = 0; j < wpm; j++) {
	    printf("%d ",(it[j])->result);
	    fflush(stdout);
	  }
	}
      }
    }
};

extern int stoi(char **str);

int main(int argc, char * argv[])
{
  int i, loops = atol(argv[1]);
  int m = atol(argv[2]);
  int s = atol(argv[3]);
  int w = atol(argv[4]);
  int d = atol(argv[5]);
  printf("L:%s M:%s S:%s I:%s D:%s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
  printf("L:%d M:%d S:%d I:%d D:%d\n", loops, m, s, w, d);
  for(; loops > 0; loops--) {
    MasterSlaves * ms = new MasterSlaves(m, s, w, d);
    ms->go();
  }
}

