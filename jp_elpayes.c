#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "include/jp_elpayes.h"

int main()
{
  int fd, r;
  char buffer[50];
  struct pid_stats stats;

  for(r=0;r<50;r++) buffer[r]='G';
  mknod("/dev/pages",254,0);
  fd=open("/dev/payes",O_RDONLY);
  r=read(fd,&stats,50);
  
  printf("\na=%i\nb=%i\nc=%i\nd=%lld\n\n",stats.num_entrades,stats.num_sortides_ok,stats.num_sortides_error,stats.durada_total);


  exit(0);
}
