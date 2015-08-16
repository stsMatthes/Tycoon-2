class DeviceTaskRec {

	Packet pending;

	DeviceTaskRec() {pending = Global.NoWork;}
	Packet Pending() {return pending;}
	void SetPending(Packet p) {pending = p;}
	void Print() {}
}

