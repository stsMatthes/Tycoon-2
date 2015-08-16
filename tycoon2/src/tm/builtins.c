/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>


static struct {
  const char *className;
  const char *selector;
  int arity;
  int sorts;
} builtins[] = {
  #define TVM_BUILTIN(cn,s,_,arity)   { #cn ,s,arity,0 },
  #define TVM_BUILTIN_C(cn,s,_,arity,sorts)   { #cn ,s,arity,sorts },
#include "builtins.h"
  { NULL, NULL, 0, 0 }
};


int main(int argc, char *argv[])
{
  int i;

  for(i=0; builtins[i].className != NULL; ++i) {
    const char *selector = builtins[i].selector;
    if (selector[0] == '\0') {
      selector = ".";
    }
    printf("%s\t%s\t%d\t%d\n",
	   builtins[i].className, selector, builtins[i].arity, builtins[i].sorts);
  }
  return EXIT_SUCCESS;
}

