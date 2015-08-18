#include <stdio.h>
#include <stdlib.h>
#include "MS.h"

Work::Work()
{
  int i = 0;
  elem = 50;
  arr = (int*) malloc(elem * sizeof(int));
  for(i = 0; i < elem; i++) {
    arr[i] = elem - i;
  }
}

void Work::doWork()
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
