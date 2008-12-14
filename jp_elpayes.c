#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>


int main()
{
  int fd, r, maj, min;
  char buffer[50];
  for(r=0;r<50;r++) buffer[r]='G';
  mknod("/dev/pages",254,0);
  fd=open("/dev/payes",O_RDONLY);
  r=read(fd,&buffer,50);
  write(0,&buffer,50);
  exit(0);
}
