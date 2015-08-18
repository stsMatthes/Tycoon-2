class Perms {

  int permArray[];
  int permCount;


  void permSwap(int i, int j)
  {
    int temp = permArray[i];
    permArray[i] = permArray[j];
    permArray[j] = temp; 
  }


  void permute(int n)
  {
    permCount = permCount + 1;
    if(n != 1) {
      permute(n - 1);
      int k = n - 1;
      while(k >= 1) {
	permSwap(n, k);
	permute(n - 1);
	permSwap(n, k);
	k = k - 1;
      }
    }
  }


  Perms()
  {
    int i = 0;
    permCount = 0;
    permArray = new int[10];
    for (i =0; i <= 7; i++) {
      permArray[i] = i - 1;
    }
  }

  public static void main(String argv[])
  {
    int i, loops = (new Integer(argv[0])).intValue();

    for(; loops > 0; loops--) {
      Perms pm = new Perms();
      for(i = 1; i <= 5; i++) {
	pm.permute(7);
      }
      if(pm.permCount != 43300) {
	System.out.println("Error in permutations");
      }
      else {
	System.out.println("Permutations completed");
      }
    }
  }
}   

