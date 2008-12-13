#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

//#define printOk(...) { printf(" Ok!\n"); }

//#include <stdlib.h>
int main()
{
  pid_t pid;
  int vfork,fd, status;
  char c;
  char buffer[] = "Hola";

  pid = getpid();
  printf("Iniciant el joc de proves...\n");
  printf("Treballarem amb el pid: %i\n",pid);

  vfork = fork();
  
  if (vfork == 0)
    {
      char arg[20];
      int uid;
      uid = getuid();
      if (uid != 0)
	  exit(2);

      sprintf(arg,"pid_inicial=%l",pid);
      execlp("insmod","insmod","module.ko",arg,'\0');
      exit(0);
    }
  
  waitpid(vfork,&status,0);

  if (status>>8 == 2)
    {
      printf("Hi ha hagut un error al fer l'insmod, tens permissos?\n");
      exit(0);
    }
  
  fd=open("del_tmp",O_RDWR|O_CREAT);
  if (fd < 0) goto error;
  if (open("nonexistent",O_RDWR) >= 0) goto error;
  if (write(fd,&buffer,4) < 0) goto error;
  if (write(-1,&buffer,-1) >= 0) goto error;
  if (lseek(fd,1,SEEK_SET) < 0) goto error;
  if (lseek(-1,1,SEEK_CUR) >= 0) goto error;
 
  //  vfork = fork();
  //if (vfork == 0) exit(0);
  //if (vfork < 0) goto error;

  if (clone(0,0,0) >= 0) goto error;
  if (close(fd) < 0) goto error;
  if (close(-1) >= 0) goto error;

  //Ens servira per el Clone
  vfork = fork();
  if (vfork == 0)
    {
      execlp("rmmod","rmmod","mihuerto","");
      exit(0);
    }

  printf("Modul descarregat, les proves s'han realitzat amb exit!\n");
  printf("A continuació un tail de /var/log/messages\n\n\n");

  vfork = fork();
  if (vfork == 0)
    {
      execlp("tail","tail","/var/log/messages",(char *) NULL);
      exit(0);
    }

  execlp("rm","rm","-f","del_tmp",(char *) NULL);
  printf("\nEnding...\n");
  exit(0);

error:
  printf(" Something wrong, quitting!\n");
  execlp("rm","rm","-f","del_tmp","");
  exit(0);
}
