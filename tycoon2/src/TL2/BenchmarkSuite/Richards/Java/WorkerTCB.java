class WorkerTCB extends TaskControlBlock {

	WorkerTCB(TaskControlBlock l, Identity id, int prio,
              Packet initialWork, TaskState initialState,
              Object privateData)
	{
    	super(l, id, prio, initialWork, initialState, privateData);
	}
    TaskControlBlock ActionFunc(Packet p, Object handle)
	{   WorkerTaskRec data = (WorkerTaskRec)handle;
    	Identity dest;

    	if (p == Global.NoWork)
         	return Global.bm.Wait();
    	else {
         	dest = (data.Destination() == Global.HandlerA) ? Global.HandlerB : Global.HandlerA;
         	data.SetDestination(dest);
         	p.SetIdent(dest);
         	p.SetDatum(1);
         	for(int i = 0; i < 4; i++) {
              	data.SetCount(data.Count() + 1);
              	if (data.Count() > 26) data.SetCount(1);
              	p.Data()[i] = (char)((int)'A' + data.Count());
         	}
         	return Global.bm.QueuePacket(p);
    	}
	}
}
