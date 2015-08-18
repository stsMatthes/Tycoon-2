class MasterSlaves {

	int m;
	int s;
	int wpm;
	int debug;
	
	MasterSlaves(int mm, int ss, int wwpm, int d)
	{
		m = mm;
		s = ss;
		wpm = wwpm;
		debug = d;
	}

	void go()
	{
	int i;
  	Master masters[] = new Master[m];
  	Slave slaves[] = new Slave[s];
  	WorkQueue queue = new WorkQueue();
  	/* start masters */
  	for(i = 0; i < m; i++) {
    	Master ma = new Master(queue, wpm);
    	masters[i] = ma;
		ma.start();
  	}
  	if(debug != 0) { System.out.println("masters started"); }
  	/* start slaves */
  	for(i = 0; i < s; i++) {
    	Slave sl = new Slave(queue);
    	slaves[i] = sl;
		sl.start(); 
  	}
  	if(debug != 0) { System.out.println("slaves started"); }
  	/* join masters */
  	for(i = 0; i < m; i++) {
	  try{
    	masters[i].join();
		} catch(Exception e) {
		  System.out.println("Exception: " + e.getMessage());
		}
  	}
  	if(debug != 0) { System.out.println("masters joined"); }
  	/* stop queue */
  	queue.setFinished();
  	if(debug != 0) { System.out.println("queue state reset"); }
  	/* join slaves */
  	for(i = 0; i < s; i++) {
	  try{
    	slaves[i].join();
		} catch(Exception e) {
		  System.out.println("Exception: " + e.getMessage());
		}
  	}
  	if(debug != 0) { System.out.println("slaves joined"); }
  	/* output result */
  	if(debug != 0) {
  		for(i = 0; i < m; i++) {
			WorkItem it[] = masters[i].workList;
			int j = 0;
			for(j = 0; j < wpm; j++) {
				System.out.print(it[j].result + " ");
			}
		}
	}
    }

  public static void main(String argv[])
  {
    int i, loops = (new Integer(argv[0])).intValue();
	int m = (new Integer(argv[1])).intValue();
	int s = (new Integer(argv[2])).intValue();
	int w = (new Integer(argv[3])).intValue();
	int d = (new Integer(argv[4])).intValue();
    for(; loops > 0; loops--) {
      MasterSlaves ms = new MasterSlaves(m, s, w, d);
	  ms.go();
    }
  }
}
