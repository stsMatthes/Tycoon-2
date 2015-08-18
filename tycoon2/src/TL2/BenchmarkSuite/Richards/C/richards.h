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
/* richards.h */
/* Richards benchmark in C++, translated from Smalltalk */
/* uh 2/2/89  */


#define RICHARDS

#include "rbase.h" 
#include "tasks.h"

/*
// RBench class definition
*/

class RBench {
    private:
         TaskControlBlock *taskList, *currentTask;
         Identity currentTaskIdent;
         TaskControlBlock *taskTable[NTASKS];
         int layout;
         int holdCount, queuePacketCount;

         /* creation */
         void CreateDevice (Identity id, int prio,  Packet *work,
                           TaskState *state);
         void CreateHandler(Identity id, int prio,  Packet *work,
                           TaskState *state);
         void CreateIdler  (Identity id, int prio,  Packet *work,
                           TaskState *state);
         void CreateWorker (Identity id, int prio,  Packet *work,
                           TaskState *state);
         void EnterTask(Identity id, TaskControlBlock *t);

         /* scheduling */
         void Schedule();

         /* initializing */
         void InitScheduler();
         void InitTrace();
    public:
         /* task management */
         TaskControlBlock *FindTask(Identity id);
         TaskControlBlock *HoldSelf();
         TaskControlBlock *QueuePacket(Packet *p);
         TaskControlBlock *Release(Identity id);
         TaskControlBlock *Wait();
         /* tracing */
         BOOLEAN tracing;
         void Trace(int id, char *s);

         void Start(BOOLEAN askTrace);
};



