#include <stdio.h>
#include <stdlib.h>
#include "MS.h"

WorkItem::WorkItem(int i)
{
  in = i;
  result = -1;
}

void WorkItem::eval()
{
  Work * wk = new Work();
  wk->doWork();
  result = in + in;
}

