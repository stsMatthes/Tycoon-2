abstract class TaskControlBlock extends TaskState {

	TaskControlBlock link;
	Identity ident;
	int priority;
	Packet input;
	Object handle;
	
	/* initializing */
	TaskControlBlock(TaskControlBlock l, Identity id, int prio,
                     Packet initialWork, TaskState initialState,
                     Object privateData)
	{
    	link = l; ident = id; priority = prio; input = initialWork;
    	packetPending = initialState.IsPacketPending();
    	taskWaiting = initialState.IsTaskWaiting();
    	taskHolding = initialState.IsTaskHolding();
    	handle = privateData;
	}
	/* accessing */
	Identity Ident() {return ident;}
	int Priority() {return priority;}
	TaskControlBlock Link() {return link;}
	/* scheduling */
	TaskControlBlock AddPacket(Packet p, TaskControlBlock oldTask)
	{
    	if (input == Global.NoWork) {
        	input = p;
         	packetPending = true;
         	if (priority > oldTask.priority) return this;
    	}
    	else Global.AddToList(input, p);
    	return oldTask;
	}
	abstract TaskControlBlock ActionFunc(Packet p, Object handle);
	TaskControlBlock RunTask()
	{   Packet msg;
    	if (IsWaitingWithPacket()) {
        	msg = input;
        	input = input.Link();
        	if (input == Global.NoWork)
            	Running();
        	else PacketPending();
    	}
    	else msg = Global.NoWork;
    	return ActionFunc(msg, handle);
	}
	void Print() {}	  
}
