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
#include <string.h>
#include <stdlib.h>
#include "tsp.h"


static struct {
  const char *packageName;
  const char *className;
  char *layout;
  char *format;
} classids[] = {
#define TYC_CLASSID(pn,cn,l,fmt)   { #pn, #cn , #l ,fmt },
#include "classids.h"
  { NULL, NULL, NULL, NULL }
};


int main(int argc, char *argv[])
{
  int i;
  const char *name;
  FILE *f;

  if (argc != 3) {
    fprintf(stderr, "Usage: classids <classids.tyc> <storeDescriptors.tab>\n");
    exit(1);
  }
  if ((f = fopen(argv[1], "w")) == NULL) {
    fprintf(stderr, "classids: Cannot open %s\n", argv[1]);
    exit(1);
  }
  for(i=0; (name = classids[i].className) != NULL; ++i) {
    const char *packageName = classids[i].packageName;
    if(strcmp(packageName, "BUILTIN") == 0)
      packageName = "";
    fprintf(f, "class %s%s meta AbstractClass {} ?\n", packageName, name);
  }
  fclose(f);

  if ((f = fopen(argv[2], "w")) == NULL) {
    fprintf(stderr, "classids: Cannot open %s\n", argv[1]);
    exit(1);
  }
  for(i=0; (name = classids[i].className) != NULL; ++i) {
    const char *format = classids[i].format;
    const char *packageName = classids[i].packageName;
    if(strcmp(packageName, "BUILTIN") == 0)
      packageName = "";
    fprintf(f, "%s%s\t%s\t%s\n",
	    packageName, name, classids[i].layout, format ? format : "");
  }
  fclose(f);
  return EXIT_SUCCESS;
}

