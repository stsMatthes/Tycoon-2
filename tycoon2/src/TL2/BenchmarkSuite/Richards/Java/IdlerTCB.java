class IdlerTCB extends TaskControlBlock {

	IdlerTCB(TaskControlBlock l, Identity id, int prio,
             Packet initialWork, TaskState initialState,
             Object privateData)
	{
    	super(l, id, prio, initialWork, initialState, privateData);
	}
	TaskControlBlock ActionFunc(Packet p, Object handle)
	{   IdleTaskRec data = (IdleTaskRec)handle;

    	data.SetCount(data.Count() - 1);
    	if (data.Count() == 0)
         	return Global.bm.HoldSelf();
    	else if ((data.Control() & 1) == 0) {
              	data.SetControl(data.Control() / 2);
              	return Global.bm.Release(Global.DeviceA);
         	}
         	else {
              	data.SetControl((data.Control() / 2) ^ 53256);
              	return Global.bm.Release(Global.DeviceB);
         	}
	}
}

