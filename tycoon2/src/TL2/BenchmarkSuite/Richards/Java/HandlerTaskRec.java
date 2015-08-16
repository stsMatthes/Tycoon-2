class HandlerTaskRec {

	Packet workIn, deviceIn;

	HandlerTaskRec() {workIn = deviceIn = Global.NoWork;}
	Packet WorkIn() {return workIn;}
	void SetWorkIn(Packet p) {workIn = p;}
	Packet WorkInAdd(Packet p) {return workIn = Global.AddToList(workIn, p);}
	Packet DeviceIn() {return deviceIn;}
	void SetDeviceIn(Packet p) {deviceIn = p;}
	Packet DeviceInAdd(Packet p) {return deviceIn = Global.AddToList(deviceIn, p);}
	void Print() {}
}

