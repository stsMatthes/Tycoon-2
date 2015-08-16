class WorkItem {

	int in;
	int result;
	
	WorkItem(int i)
	{
		in = i;
		result = -1;
	}
	
	void eval()
	{
		Work wk = new Work();
		wk.doWork();
  		result = in + in;
	}
}
