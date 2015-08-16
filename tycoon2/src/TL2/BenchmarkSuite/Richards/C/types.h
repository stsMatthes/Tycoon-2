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
/* types.h -- basic classes (Packet, {Device,Idle,...}TaskRec)	*/

#include "cbase.h"
#include "rbase.h"

/*
// Packet
*/

class Packet {
    private:
        Packet *link;		// next packet in queue
	Identity ident;		// Idle, Worker, DeviceA, ...
	PacketKind kind;	// DevicePacket or WorkPacket
	int datum;
	char data[4];
    public:
        Packet(Packet *l, Identity id, PacketKind k);
	char *Data() {return data;}
	void SetData(char d[4]) {for(int i=0; i < 4; i++) data[i] = d[i];}
        int Datum() {return datum;}
        void SetDatum(int n) {datum = n;}
	Identity Ident() {return ident;}
	void SetIdent(Identity i) {ident = i;}
	PacketKind Kind() {return kind;}
	void SetKind(PacketKind k) {kind = k;}
	Packet *Link() {return link;}
	void SetLink(Packet *l) {link = l;}
        void Print() {
	    printf("[%d,%s,%d,%s]", ident+1,((int)kind == 0)?"d":"w",datum,data);
	    if (link != NoWork) link->Print();
	}
};

/*
// AddToList - list utility (append elem at end of list, return head)
*/
Packet *AddToList(Packet *list, Packet *elem);

/*
// DeviceTaskRec
*/

class DeviceTaskRec {
    private:
        Packet *pending;
    public:
        DeviceTaskRec() {pending = NoWork;}
	Packet *Pending() {return pending;}
	void SetPending(Packet *p) {pending = p;}
	void Print() {printf("device(");if (pending != NoWork) pending->Print();printf(")");}
};



/*
// IdleTaskRec 
*/

class IdleTaskRec {
    private:
        int control, count;
    public:
        IdleTaskRec() {control = 1; count = 10000;}
        int Control() {return control;}
        void SetControl(int n) {control = n;}
        int Count() {return count;}
        void SetCount(int n) {count = n;}
	void Print() {printf("idler(%d,%d)",control,count);}
};


/*
// HandlerTaskRec
*/

class HandlerTaskRec {
    private:
        Packet *workIn, *deviceIn;
    public:
        HandlerTaskRec() {workIn = deviceIn = NoWork;}
        Packet *WorkIn() {return workIn;}
        void SetWorkIn(Packet *p) {workIn = p;}
        Packet *WorkInAdd(Packet *p) {return workIn = AddToList(workIn, p);}
        Packet *DeviceIn() {return deviceIn;}
        void SetDeviceIn(Packet *p) {deviceIn = p;}
        Packet *DeviceInAdd(Packet *p) {return deviceIn = AddToList(deviceIn, p);}
	void Print() {
	  printf("handler(");
	  if (workIn != NoWork) workIn->Print();
	  printf(",");
	  if (deviceIn != NoWork) deviceIn->Print();
	  printf(")");
	}
};


/*
// WorkerTaskRec
*/

class WorkerTaskRec {
    private:
        Identity destination;
	int count;
    public:
        WorkerTaskRec() {destination = HandlerA; count = 0;}
        int Count() {return count;}
        void SetCount(int n) {count = n;}
        Identity Destination() {return destination;}
        void SetDestination(Identity d) {destination = d;}
	void Print() {printf("worker(%d,%d)",destination+1,count);}
};

