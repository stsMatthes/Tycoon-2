class Slave extends Thread {

	WorkQueue workQueue;
	
	Slave(WorkQueue queue)
	{
		workQueue = queue;
	}
	

	public void run()
	{
	    boolean loop = true;
    	while(loop) {
      		WorkItem w = workQueue.getWork();
      		if(w == null) {
      			loop = false;
			}
			else {
				w.eval();
			}
		}
	}
}
