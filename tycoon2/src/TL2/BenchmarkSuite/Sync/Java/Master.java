class Master extends Thread {

	WorkItem workList[];
	WorkQueue workQueue;
	int items;

	Master(WorkQueue q, int i)
	{
		workQueue = q;
		items = i;
		workList = new WorkItem[i];
	}

	public void run()
	{
		int i = 0;
		for(i = 0; i < items; i++) {
			WorkItem w = new WorkItem(i);
			workQueue.addWork(w);
			workList[i] = w;
		}
	}
}
