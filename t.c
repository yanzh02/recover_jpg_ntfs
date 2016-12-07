#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static char *bb[] = {"aa", "bb"};

void main(){
  printf("%s--%s\n", bb[0], bb[1]);
  printf("%d\n", sizeof(bb) / sizeof(void*));
}

