class Packet {
	Packet link;		// next packet in queue
	Identity ident;		// Idle, Worker, DeviceA, ...
	PacketKind kind;	// DevicePacket or WorkPacket
	int datum;
	char data[] = {0, 0, 0, 0};

	Packet(Packet l, Identity id, PacketKind k)
	{
    	link = l;
    	ident = id;
    	kind = k;
    	datum = 1;
    	for(int i = 0; i < 4; i++) data[i] = 0;
	}
	
	char[] Data() {return data;}
	void SetData(char d[]) {for(int i=0; i < 4; i++) data[i] = d[i];}
	int Datum() {return datum;}
	void SetDatum(int n) {datum = n;}
	Identity Ident() {return ident;}
	void SetIdent(Identity i) {ident = i;}
	PacketKind Kind() {return kind;}
	void SetKind(PacketKind k) {kind = k;}
	Packet Link() {return link;}
	void SetLink(Packet l) {link = l;}
    void Print() {}
}
