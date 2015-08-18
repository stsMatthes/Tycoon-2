import java.util.Vector;

class WorkQueue {

	Vector queue;
	int slaveWaiting;

	boolean finished;

	WorkQueue()
	{
	  	queue = new Vector(100);
  		slaveWaiting = 0;
  		finished = false;
	}

	synchronized void addWork(WorkItem work)
	{
    	queue.addElement(work);
    	if(slaveWaiting > 0) {
    		notifyAll();
		}
	}


	synchronized WorkItem getWork()
	{
    	while(queue.isEmpty() && !finished) {
       		slaveWaiting = slaveWaiting + 1;
		try {
       		wait();
		} catch(Exception e) {
		  System.out.println("Exception: " + e.getMessage());
		}
       		slaveWaiting = slaveWaiting - 1;
    	}
    	if(queue.isEmpty() && finished) {
    		return null;
		}
		else {
		WorkItem w = (WorkItem)queue.firstElement();
		queue.removeElementAt(0);
    		return w;
		}
	}


	synchronized void setFinished()
	{
    	finished = true;
    	notifyAll();
	}
}
