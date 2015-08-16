class Work {

  int arr[];
  int elem;

  Work()
    {
      int i = 0;
      elem = 50;
      arr = new int[elem];
      for(i = 0; i < elem; i++) {
	arr[i] = elem - i;
      }
    }

  void doWork()
    {
      int top = elem - 1;
      while(top > 0) {
	int i = 0;
	while(i < top) {
	  if(arr[i] > arr[i + 1]) {
	    int t = arr[i];
	    arr[i] = arr[i + 1];
	    arr[i + 1] = t;
	  }
	  i = i + 1;
	}
	top = top - 1;
      }
    }

}
