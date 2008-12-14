#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "include/jp_elpayes.h"

int pid_monitoritzat;

void
print_stats (struct pid_stats *pidstats)
{
  //Aquesta funcio mes envant podria rebre un parametre
  //diferent per imprimir mes estadistiques. P.ex un vector
  //de pid_stats que contingues la info de totes les crides del proces
  printf ("Info proces_monitoritzat:\n");
  printf ("-------------------------\n");
  printf ("Pid          : %i\n", pid_monitoritzat);
  printf ("Num crides   : %i\n", pidstats->num_entrades);
  printf ("Ret correcte : %i\n", pidstats->num_sortides_ok);
  printf ("Ret erroni   : %i\n", pidstats->num_sortides_error);
  printf ("Temps total  : %lld\n", pidstats->durada_total);
}

int
main ()
{
  short int error;
  int fd, fd2, res;
  char buffer[50];
  struct pid_stats stats;

  pid_monitoritzat = 0;
  error = 0;

  //Creem el nou dispositiu amb major 254 i minor 0
  //concorda amb el de elpayes.h
  system ("mknod /dev/payes c 254 0\n");
  printf ("\n(open) Obrint el dispositiu:");
  fd = open ("/dev/payes", O_RDONLY);
  error++;
  if (fd < 0)
    goto msg_error;
  printf ("Ok\n(open) Tornant a provar amb error:");
  fd2 = open ("/dev/payes", O_RDONLY);
  error++;
  if (fd2 >= 0)
    goto msg_error;
  printf ("Ok\n(read) Llegim dades del pid_inicial:");
  res = read (fd, &stats, sizeof (struct pid_stats));
  error++;
  if (res < 0)
    goto msg_error;
  print_stats (&stats);

  pid_monitoritzat = getpid ();
  printf ("Ok\n(ioctl) Canviem a  pid %i:", getpid ());
  res = ioctl (fd, CH_PID, getpid ());
  error++;
  if (res < 0)
    goto msg_error;
  printf ("Ok\n(ioctl) Canviem syscall a OPEN:\n");
  res = ioctl (fd, CH_SYSC, 0);
  error++;
  if (res < 0)
    goto msg_error;
  printf ("Ok\n(read) Pintem les dades del proces %i:", getpid ());
  res = read (fd, &stats, sizeof (struct pid_stats));
  error++;
  if (res < 0)
    goto msg_error;
  print_stats (&stats);

  printf ("Ok\n(ioctl) Resetejem vals del proces %i:", getpid ());
  res = ioctl (fd, RESET_VALS, getpid ());
  error++;
  if (res < 0)
    goto msg_error;
  printf ("Ok\n(read) Pintem valors resetejats (no estaran a 0):");
  res = read (fd, &stats, sizeof (struct pid_stats));
  error++;
  if (res < 0)
    goto msg_error;
  print_stats (&stats);
  printf ("Ok\n(ioctl) Resetejem totes les estadistiques dels processos:");
  res = ioctl (fd, RESET_ALL_VALS, 0);
  error++;
  if (res < 0)
    goto msg_error;
  printf ("Ok\n(ioctl) Desactivar totes les syscalls:");
  res = ioctl (fd, DEACT_SYSC, -1);
  error++;
  if (res < 0)
    goto msg_error;

  printf ("Ok\n(ioctl) Activant totes les syscalls:");
  res = ioctl (fd, ACT_SYSC, -1);
  error++;
  if (res < 0)
    goto msg_error;

  printf ("Ok\n(release) Tancant el dispositiu:");
  res = close (fd);
  error++;
  if (res < 0)
    goto msg_error;

  printf ("Ok\n Joc de proves superat!\n");

msg_error:
  if (error != 0)
    printf ("ERR\nHi ha hagut un error (00%i)\n", error);
  exit (0);
}
