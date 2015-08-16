/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
#include <stdio.h>
#include <pthread.h>

class List {
 public:

  void * h;
  List * t;

  List()
    {
      h = NULL;
      t = NULL;
    }

  List(void * e, List * l)
    {
      h = e;
      t = l;
    }

  List * addElement(void * e)
    {
      List * n = new List(e, this);
      return n;
    }

  int isEmpty()
    {
      if(h == NULL && t == NULL)
	return 1;
      else
	return 0;
    }

  void * head()
    {
      return h;
    }

  List * tail()
    {
      return t;
    }
};

class WorkItem {
 public:
  int in;
  int result;
  
  WorkItem(int i);
  void eval();
};


class WorkQueue {
 public:
  List * queue;
  int slaveWaiting;
  int finished;
  pthread_mutex_t * mx;
  pthread_cond_t * cv;
  
  WorkQueue();
  void addWork(WorkItem * work);
  WorkItem * getWork();
  void setFinished();
};

class Master{
  
 public:
  WorkItem ** workList;
  WorkQueue * workQueue;
  int items;
  
  Master(WorkQueue * q, int i);
  void * run(void * p);
};


class Work {
 public:
  int * arr;
  int elem;

  Work();
  void doWork();
};


class Slave {
 public:
  WorkQueue * workQueue;
	
  Slave(WorkQueue * queue);
  void * run(void *);
};
