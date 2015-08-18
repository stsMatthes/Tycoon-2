class Global {

	static Packet NoWork = null;
	static TaskControlBlock NoTask = null;
	
	static PacketKind DevicePacket = new PacketKind(0);
	static PacketKind WorkPacket = new PacketKind(1);

	static Identity Idler = new Identity(0);
	static Identity Worker = new Identity(1);
	static Identity HandlerA = new Identity(2);
	static Identity HandlerB = new Identity(3);
	static Identity DeviceA = new Identity(4);
	static Identity DeviceB = new Identity(5);
	
	static int NTASKS = 6;

	static RBench bm;
	
	static Packet AddToList(Packet list, Packet elem)
	{   Packet p, next;

    	elem.SetLink(NoWork);
    	if (list != NoWork) {
         	p = list;
         	while((next = p.Link()) != NoWork) p = next;
         	p.SetLink(elem);
    	}
    	else list = elem;
    	return list;
	}

	public static void main(String[] argv)
	{
		int i, loops = Integer.valueOf(argv[0]).intValue();
		bm = new RBench();
		System.out.println("Starting...");
		for(i = 0; i < loops; i++) {
			bm.Start(false);
			System.out.print(".");
		}
		System.out.println("");
		System.out.println("Finished...");
	}

}