class TaskState {

	boolean packetPending, taskWaiting, taskHolding;
	
	/* initializing */
	TaskState() {packetPending = true; taskWaiting = taskHolding = false;}
	void PacketPending()
		{packetPending = true; taskHolding = taskWaiting = false;}
	void Waiting() {packetPending = taskHolding = false; taskWaiting = true;}
	void Running() {packetPending = taskWaiting = taskHolding = false;}
	void WaitingWithPacket()
		{packetPending = taskWaiting = true; taskHolding = false;}
	/* accessing */
	boolean IsPacketPending() {return packetPending;}
	boolean IsTaskWaiting() {return taskWaiting;}
	boolean IsTaskHolding() {return taskHolding;}
	void SetTaskHolding(boolean state) {taskHolding = state;}
	void SetTaskWaiting(boolean state) {taskWaiting = state;}
	/* testing */
	boolean IsRunning()
		{return !packetPending && !taskWaiting && !taskHolding;}
	boolean IsTaskHoldingOrWaiting()
		{return taskHolding || !packetPending && taskWaiting;}
	boolean IsWaiting()
		{return !packetPending && taskWaiting && !taskHolding;}
	boolean IsWaitingWithPacket()
		{return packetPending && taskWaiting && !taskHolding;}
}
