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
/* tasks.h - definitions of TaskState, TaskControlBlock and xxxTCB 
 * The subclasses of TaskControlBlock differ only in their
 * implementation of the action function 				  */

#ifndef TASKS

#define TASKS

#include "types.h"

/*
// TaskState
*/

class TaskState {
    protected:
        BOOLEAN packetPending, taskWaiting, taskHolding;
	
    public:
        /* initializing */
        TaskState() {packetPending = TRUE; taskWaiting = taskHolding = FALSE;}
	void PacketPending() {packetPending = TRUE; taskHolding = taskWaiting = FALSE;}
	void Waiting() {packetPending = taskHolding = FALSE; taskWaiting = TRUE;}
	void Running() {packetPending = taskWaiting = taskHolding = FALSE;}
	void WaitingWithPacket() {packetPending = taskWaiting = TRUE; taskHolding = FALSE;}
	/* accessing */
	BOOLEAN IsPacketPending() {return packetPending;}
	BOOLEAN IsTaskWaiting() {return taskWaiting;}
	BOOLEAN IsTaskHolding() {return taskHolding;}
	void SetTaskHolding(BOOLEAN state) {taskHolding = state;}
	void SetTaskWaiting(BOOLEAN state) {taskWaiting = state;}
	/* testing */
	BOOLEAN IsRunning() {return !packetPending && !taskWaiting && !taskHolding;}
	BOOLEAN IsTaskHoldingOrWaiting() {return taskHolding || !packetPending && taskWaiting;}
	BOOLEAN IsWaiting() {return !packetPending && taskWaiting && !taskHolding;}
	BOOLEAN IsWaitingWithPacket() {return packetPending && taskWaiting && !taskHolding;}
};

/*
// TaskControlBlock
*/

class TaskControlBlock : public TaskState {

    protected:
        TaskControlBlock *link;
	Identity ident;
	int priority;
	Packet *input;
	void *handle;
	
    public:
        /* initializing */
        TaskControlBlock(TaskControlBlock *l, Identity id, int prio,
                         Packet *initialWork, TaskState *initialState,
                         void *privateData);
	/* accessing */
        Identity Ident() {return ident;}
        int Priority() {return priority;}
        TaskControlBlock *Link() {return link;}
	/* scheduling */
        TaskControlBlock *AddPacket(Packet *p, TaskControlBlock *old);
        virtual TaskControlBlock *ActionFunc(Packet *p, void *handle);
        TaskControlBlock *RunTask();
	void Print() {
	  printf("%d %d %d %d %d ",packetPending,taskWaiting,taskHolding,ident+1,priority);
	  if (input != NoWork) input->Print();
	  switch (ident){
	    case 0: {((IdleTaskRec *)handle)->Print();break;}
	    case 1: {((WorkerTaskRec *)handle)->Print();break;}
	    case 2:
	    case 3: {((HandlerTaskRec *)handle)->Print();break;}
	    case 4:
	    case 5: {((DeviceTaskRec *)handle)->Print();break;}
	  }
	  printf("\n");
	  if (link != NoTask) link->Print();
	}
	  
};

/*
// DeviceTCB 
*/

class DeviceTCB : public TaskControlBlock {
    public:
        DeviceTCB(TaskControlBlock *l, Identity id, int prio,
                  Packet *initialWork, TaskState *initialState,
                  void *privateData) :
                 TaskControlBlock(l, id, prio, initialWork, initialState, privateData){}
        TaskControlBlock *ActionFunc(Packet *p, void *handle);
};


/*
// HandlerTCB
*/

class HandlerTCB : public TaskControlBlock {
    public:
        HandlerTCB(TaskControlBlock *l, Identity id, int prio,
                   Packet *initialWork, TaskState *initialState,
                   void *privateData) :
                  TaskControlBlock(l, id, prio, initialWork, initialState, privateData){}
        TaskControlBlock *ActionFunc(Packet *p, void *handle);
};


/*
// IdlerTCB
*/

class IdlerTCB : public TaskControlBlock {
    public:
        IdlerTCB(TaskControlBlock *l, Identity id, int prio,
                 Packet *initialWork, TaskState *initialState,
                 void *privateData) :
                TaskControlBlock(l, id, prio, initialWork, initialState, privateData){}
        TaskControlBlock *ActionFunc(Packet *p, void *handle);
};


/*
// WorkerTCB
*/

class WorkerTCB : public TaskControlBlock {
    public:
        WorkerTCB(TaskControlBlock *l, Identity id, int prio,
                  Packet *initialWork, TaskState *initialState,
                  void *privateData) :
                 TaskControlBlock(l, id, prio, initialWork, initialState, privateData){}
        TaskControlBlock *ActionFunc(Packet *p, void *handle);
};

#endif
