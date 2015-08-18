class DeviceTCB extends TaskControlBlock {

	DeviceTCB(TaskControlBlock l, Identity id, int prio,
              Packet initialWork, TaskState initialState,
              Object privateData)
	{
    	super(l, id, prio, initialWork, initialState, privateData);
	}
    TaskControlBlock ActionFunc(Packet p, Object handle)
	{   DeviceTaskRec data = (DeviceTaskRec)handle;

    	if (p == Global.NoWork) {
        	if ((p = data.Pending()) == Global.NoWork)
            	return Global.bm.Wait();
        	else {
            	data.SetPending(Global.NoWork);
              	return Global.bm.QueuePacket(p);
         	}
    	}
    	else {
         	data.SetPending(p);
         	if (Global.bm.tracing) Global.bm.Trace(p.Datum(), "d");
         	return Global.bm.HoldSelf();
    	}
	}
}

