class HandlerTCB extends TaskControlBlock {

	HandlerTCB(TaskControlBlock l, Identity id, int prio,
               Packet initialWork, TaskState initialState,
               Object privateData)
	{
		super(l, id, prio, initialWork, initialState, privateData);
	}
	TaskControlBlock ActionFunc(Packet p, Object handle)
	{   HandlerTaskRec data = (HandlerTaskRec)handle;
    	Packet work, devicePacket;
    	int count;

    	if (p != Global.NoWork) {
         	if (p.Kind() == Global.WorkPacket)
              	data.WorkInAdd(p);
         	else data.DeviceInAdd(p);
    	}
    	if ((work = data.WorkIn()) == Global.NoWork)
         	return Global.bm.Wait();
    	else {
         	count = work.Datum();
         	if (count > 4) {
              	data.SetWorkIn(work.Link());
              	return Global.bm.QueuePacket(work);
         	}
         	else {
              	if ((devicePacket = data.DeviceIn()) == Global.NoWork)
                   	return Global.bm.Wait();
              	else {
                   	data.SetDeviceIn(devicePacket.Link());
                   	devicePacket.SetDatum(work.Data()[count-1]);
                   	work.SetDatum(count + 1);
                   	return Global.bm.QueuePacket(devicePacket);
              	}
         	}
    	}
	}
}

