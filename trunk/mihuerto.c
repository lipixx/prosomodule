#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include "include/mihuerto.h"

pid_t pid_inicial = 1;
module_param (pid_inicial, int, 0);
MODULE_PARM_DESC (pid_inicial, "PID del proces");
MODULE_AUTHOR
  ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION ("ProSO driver: estadistiques");
MODULE_LICENSE ("GPL");

EXPORT_SYMBOL(activar_monitoritzacio);
EXPORT_SYMBOL(desactivar_monitoritzacio);
EXPORT_SYMBOL(sysc_info_table);
EXPORT_SYMBOL(obtenir_estadistiques);
EXPORT_SYMBOL(reset_info);

static int __init
comprar_huerto_init (void)
{
  struct task_struct *t;
  
  /* Guardam les adreces originals de les crides a sistema */
  sys_call_table_originals[OPEN] = sys_call_table[POS_SYSCALL_OPEN];
  sys_call_table_originals[CLOSE] = sys_call_table[POS_SYSCALL_CLOSE];
  sys_call_table_originals[WRITER] = sys_call_table[POS_SYSCALL_WRITE];
  sys_call_table_originals[CLONE] = sys_call_table[POS_SYSCALL_CLONE];
  sys_call_table_originals[LSEEK] = sys_call_table[POS_SYSCALL_LSEEK];

  /* Guardem les adreces de les funcions locals a la taula sys_call_table_locals */
  sys_call_table_locals[OPEN] = sys_open_local;
  sys_call_table_locals[CLOSE] = sys_close_local;
  sys_call_table_locals[WRITER] = sys_write_local;
  sys_call_table_locals[CLONE] = sys_clone_local;
  sys_call_table_locals[LSEEK] = sys_lseek_local;

  /* Redireccionam les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[POS_SYSCALL_OPEN] = sys_call_table_locals[OPEN];
  sys_call_table[POS_SYSCALL_CLOSE] = sys_call_table_locals[CLOSE];
  sys_call_table[POS_SYSCALL_WRITE] = sys_call_table_locals[WRITER];
  sys_call_table[POS_SYSCALL_CLONE] = sys_call_table_locals[CLONE];
  sys_call_table[POS_SYSCALL_LSEEK] = sys_call_table_locals[LSEEK];

  t = find_task_by_pid (pid_inicial);

  if (t == NULL)
    {
      pid_inicial = 1;
      t = find_task_by_pid (pid_inicial);
      printk (KERN_DEBUG "El pid no existex, s'agafa el default 1");
    }

  pid=pid_inicial;/* Feim una copia per accedir des de el payes */
  
  reset_info (pid_inicial, (struct th_info_est *) t->thread_info);
  printk (KERN_DEBUG "MiHuerto: Loaded\n");

  return 0;
}

/* Descarrega el modul. */

static void __exit
vender_huerto_exit (void)
{
  struct task_struct *t;
  
  /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[POS_SYSCALL_OPEN] = sys_call_table_originals[OPEN];
  sys_call_table[POS_SYSCALL_CLOSE] = sys_call_table_originals[CLOSE];
  sys_call_table[POS_SYSCALL_WRITE] = sys_call_table_originals[WRITER];
  sys_call_table[POS_SYSCALL_CLONE] = sys_call_table_originals[CLONE];
  sys_call_table[POS_SYSCALL_LSEEK] = sys_call_table_originals[LSEEK];
  
  t = find_task_by_pid (pid_inicial);
  
  if (t == NULL)
    printk (KERN_DEBUG "El pid ja no existex\n");
  else
    imprimir_estadistiques (pid_inicial);
  
  printk (KERN_DEBUG "MiHuerto: Unloaded\n");
}

module_init (comprar_huerto_init);
module_exit (vender_huerto_exit);


long
sys_open_local (const char __user * filename, int flags, int mode)
{
  long (*crida) (const char __user * filename, int flags, int mode);
  quad inici, final;
  struct th_info_est *thinfo_stats;
  struct pid_stats *pidstats;
  int resultat;
  int pid;

  try_module_get (THIS_MODULE);

  crida = sys_call_table_originals[OPEN];
  thinfo_stats = (struct th_info_est *) current_thread_info ();
  pid = current_thread_info ()->task->pid;
  pidstats = &(thinfo_stats->estadistiques[OPEN]);

  //  if (pid != pidstats->pid)
  if (pid != thinfo_stats->pid)
    reset_info (pid, thinfo_stats);

  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;
  sysc_info_table[OPEN].num_crides++;
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_entrades++;

  /* Comptem el temps de la crida */
  inici = proso_get_cycles ();
  resultat = crida (filename, flags, mode);
  final = proso_get_cycles ();

  if (resultat >= 0)
    {
      pidstats->num_sortides_ok++;
      sysc_info_table[OPEN].num_satisfactories++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_ok++;
    }
  else
    {
      pidstats->num_sortides_error++;
      sysc_info_table[OPEN].num_fallides++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_error++;
    }

  pidstats->durada_total += (final - inici);
  sysc_info_table[OPEN].temps_execucio += (final - inici);
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].durada_total +=
    (final - inici);

  module_put (THIS_MODULE);
  return resultat;
}

long
sys_close_local (unsigned int fd)
{
  long (*crida) (unsigned int fd);
  quad inici, final;
  struct th_info_est *thinfo_stats;
  struct pid_stats *pidstats;
  int resultat;
  int pid;

  try_module_get (THIS_MODULE);

  crida = sys_call_table_originals[CLOSE];
  thinfo_stats = (struct th_info_est *) current_thread_info ();
  pid = current_thread_info ()->task->pid;
  pidstats = &(thinfo_stats->estadistiques[CLOSE]);

  //  if (pid != pidstats->pid)
  if (pid != thinfo_stats->pid)
    reset_info (pid, thinfo_stats);

  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;
  sysc_info_table[CLOSE].num_crides++;
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_entrades++;


  /* Comptem el temps de la crida */
  inici = proso_get_cycles ();
  resultat = crida (fd);
  final = proso_get_cycles ();

  if (resultat >= 0)
    {
      pidstats->num_sortides_ok++;
      sysc_info_table[CLOSE].num_satisfactories++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_ok++;
    }
  else
    {
      pidstats->num_sortides_error++;
      sysc_info_table[CLOSE].num_fallides++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_error++;
    }

  pidstats->durada_total += (final - inici);
  sysc_info_table[CLOSE].temps_execucio += (final - inici);
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].durada_total +=
    (final - inici);

  module_put (THIS_MODULE);
  return resultat;
}

ssize_t
sys_write_local (unsigned int fd, const char __user * buf, size_t count)
{
  long (*crida) (unsigned int fd, const char __user * buf, size_t count);
  quad inici, final;
  struct th_info_est *thinfo_stats;
  struct pid_stats *pidstats;
  int resultat;
  int pid;

  try_module_get (THIS_MODULE);

  crida = sys_call_table_originals[WRITER];
  thinfo_stats = (struct th_info_est *) current_thread_info ();
  pid = current_thread_info ()->task->pid;
  pidstats = &(thinfo_stats->estadistiques[WRITER]);

  //  if (pid != pidstats->pid)
  if (pid != thinfo_stats->pid)
    reset_info (pid, thinfo_stats);

  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;
  sysc_info_table[WRITER].num_crides++;
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_entrades++;


  /* Comptem el temps de la crida */
  inici = proso_get_cycles ();
  resultat = crida (fd, buf, count);
  final = proso_get_cycles ();

  if (resultat >= 0)
    {
      pidstats->num_sortides_ok++;
      sysc_info_table[WRITER].num_satisfactories++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_ok++;
    }
  else
    {
      pidstats->num_sortides_error++;
      sysc_info_table[WRITER].num_fallides++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_error++;
    }

  pidstats->durada_total += (final - inici);
  sysc_info_table[WRITER].temps_execucio += (final - inici);
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].durada_total +=
    (final - inici);

  module_put (THIS_MODULE);
  return resultat;
}

int
sys_clone_local (struct pt_regs regs)
{
  long (*crida) (struct pt_regs regs);
  quad inici, final;
  struct th_info_est *thinfo_stats;
  struct pid_stats *pidstats;
  int resultat;
  int pid;

  try_module_get (THIS_MODULE);

  crida = sys_call_table_originals[CLONE];
  thinfo_stats = (struct th_info_est *) current_thread_info ();
  pid = current_thread_info ()->task->pid;
  pidstats = &(thinfo_stats->estadistiques[CLONE]);

  //  if (pid != pidstats->pid)
  if (pid != thinfo_stats->pid)
    reset_info (pid, thinfo_stats);

  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;
  sysc_info_table[CLONE].num_crides++;
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_entrades++;

  /* Comptem el temps de la crida */
  inici = proso_get_cycles ();
  resultat = crida (regs);
  final = proso_get_cycles ();

  if (resultat >= 0)
    {
      pidstats->num_sortides_ok++;
      sysc_info_table[CLONE].num_satisfactories++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_ok++;
    }
  else
    {
      pidstats->num_sortides_error++;
      sysc_info_table[CLONE].num_fallides++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_error++;
    }

  pidstats->durada_total += (final - inici);
  sysc_info_table[CLONE].temps_execucio += (final - inici);
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].durada_total +=
    (final - inici);

  module_put (THIS_MODULE);
  return resultat;
}

off_t
sys_lseek_local (unsigned int fd, off_t offset, unsigned int origin)
{
  long (*crida) (unsigned int fd, off_t offset, unsigned int origin);
  quad inici, final;
  struct th_info_est *thinfo_stats;
  struct pid_stats *pidstats;
  int resultat;
  int pid;

  try_module_get (THIS_MODULE);

  crida = sys_call_table_originals[LSEEK];
  thinfo_stats = (struct th_info_est *) current_thread_info ();
  pid = current_thread_info ()->task->pid;
  pidstats = &(thinfo_stats->estadistiques[LSEEK]);

  //  if (pid != pidstats->pid)
  if (pid != thinfo_stats->pid)
    reset_info (pid, thinfo_stats);

  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;
  sysc_info_table[LSEEK].num_crides++;
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_entrades++;

  /* Comptem el temps de la crida */
  inici = proso_get_cycles ();
  resultat = crida (fd, offset, origin);
  final = proso_get_cycles ();

  if (resultat >= 0)
    {
      pidstats->num_sortides_ok++;
      sysc_info_table[LSEEK].num_satisfactories++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_ok++;

    }
  else
    {
      pidstats->num_sortides_error++;
      sysc_info_table[LSEEK].num_fallides++;
      thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].num_sortides_error++;
    }

  pidstats->durada_total += (final - inici);
  sysc_info_table[LSEEK].temps_execucio += (final - inici);
  thinfo_stats->estadistiques[N_CRIDES_A_MONITORITZAR].durada_total +=
    (final - inici);

  module_put (THIS_MODULE);
  return resultat;
}

void
activar_monitoritzacio (int num_crida)
{
  /* Intercerptar les crides:
   * Redireccionam les crides a sistema amb les
   * adreces de les nostres crides monitoritzades 
   */
  sys_call_table[taula_de_constants[num_crida]] =
    sys_call_table_locals[num_crida];
}

void
desactivar_monitoritzacio (int num_crida)
{
  /* Desinterceptar les crides:
   * Restauram les crides a sistema amb les
   * adreces de les nostres crides monitoritzades 
   */
  sys_call_table[taula_de_constants[num_crida]] =
    sys_call_table_originals[num_crida];
}

void
reset_info (pid_t pid, struct th_info_est *est)
{
  int i;
  struct pid_stats *pidstats;

  est->pid = pid;

  for (i = 0; i <= N_CRIDES_A_MONITORITZAR; i++)
    {

      pidstats = &(est->estadistiques[i]);

      /* Re-Inicialitzem les estadistiques */
      pidstats->num_entrades = 0;
      pidstats->num_sortides_ok = 0;
      pidstats->num_sortides_error = 0;
      pidstats->durada_total = 0;
    }
}

void
imprimir_estadistiques (int pid)
{
  struct task_struct *task_pid;
  struct pid_stats *pidstats;

  //Cercar proces per PID:
  task_pid = find_task_by_pid ((pid_t) pid);

  if (task_pid >= 0)
    {
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[OPEN]);

      /*printk ("    --MiHuerto --\n");
         printk ("Pid          : %i\n", pidstats->pid);
         printk ("Num crides   : %i\n", pidstats->num_entrades);
         printk ("Ret correcte : %i\n", pidstats->num_sortides_ok);
         printk ("Ret erroni   : %i\n", pidstats-m>num_sortides_error);
         printk ("Temps total  : %lld\n", pidstats->durada_total);
         printk ("    -------------\n"); */

      printk ("    --MiHuerto --\n");
      printk ("Pid               : %i\n\n", pid);
      printk ("Num OPENS         : %i\n", pidstats->num_entrades);
      printk ("Ret opens ok      : %i\n", pidstats->num_sortides_ok);
      printk ("Ret opens error   : %i\n", pidstats->num_sortides_error);
      printk ("Temps total opens : %lld\n\n", pidstats->durada_total);
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[CLOSE]);
      printk ("Num CLOSES         : %i\n", pidstats->num_entrades);
      printk ("Ret closes ok      : %i\n", pidstats->num_sortides_ok);
      printk ("Ret closes error   : %i\n", pidstats->num_sortides_error);
      printk ("Temps total closes : %lld\n\n", pidstats->durada_total);
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[WRITER]);
      printk ("Num WRITES         : %i\n", pidstats->num_entrades);
      printk ("Ret writes ok      : %i\n", pidstats->num_sortides_ok);
      printk ("Ret writes error   : %i\n", pidstats->num_sortides_error);
      printk ("Temps total writes : %lld\n\n", pidstats->durada_total);
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[CLONE]);
      printk ("Num CLONES         : %i\n", pidstats->num_entrades);
      printk ("Ret clones ok      : %i\n", pidstats->num_sortides_ok);
      printk ("Ret clones error   : %i\n", pidstats->num_sortides_error);
      printk ("Temps total clones : %lld\n\n", pidstats->durada_total);
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[LSEEK]);
      printk ("Num LSEEKS         : %i\n", pidstats->num_entrades);
      printk ("Ret lseeks ok      : %i\n", pidstats->num_sortides_ok);
      printk ("Ret lseeks error   : %i\n", pidstats->num_sortides_error);
      printk ("Temps total lseeks : %lld\n\n", pidstats->durada_total);

      printk ("    -------------\n");
      pidstats =
	&(((struct th_info_est *) task_pid->thread_info)->
	  estadistiques[N_CRIDES_A_MONITORITZAR]);
      printk ("Num TOTAL crides   : %i\n", pidstats->num_entrades);
      printk ("Ret total ok       : %i\n", pidstats->num_sortides_ok);
      printk ("Ret total error    : %i\n", pidstats->num_sortides_error);
      printk ("Temps total crides : %lld\n\n", pidstats->durada_total);
      printk ("    -------------\n");

    }
  else
    printk ("MiHuerto: No such pid\n");
}

EXPORT_SYMBOL (imprimir_estadistiques);

int
obtenir_estadistiques (int pid, int crida, struct pid_stats *stats)
{

  struct th_info_est * task;
  //  struct pid_stats * task_stats;

  if (crida < 0 || crida > N_CRIDES_A_MONITORITZAR)
    return -EINVAL;
  if (pid < 0)
    return -EINVAL;
  task =  (struct th_info_est *) (find_task_by_pid ((pid_t) pid));
  if (task < 0)
    return -ESRCH;

  //task_stats = (struct pid_stats *) &(task->estadistiques[crida]);
  //  task_stats = &(task->estadistiques[0]);
  
  //stats=&task_stats;
  
  printk(KERN_DEBUG "\n1=%i\n2=%i\n3=%i\n4=%lld\n\n",task_stats->num_entrades,task_stats->num_sortides_ok,task_stats->num_sortides_error,task_stats->durada_total);
  
  stats->num_entrades = task->estadistiques[crida]->num_entrades;
  stats->num_sortides_ok = task->estadistiques[crida]->num_sortides_ok;
  stats->num_sortides_error = task->estadistiques[crida]->num_sortides_error;
  stats->durada_total = task->estadistiques[crida]->durada_total;
  
  printk(KERN_EMERG"\n1=%i\n2=%i\n3=%i\n4=%lld\n\n",stats->num_entrades,stats->num_sortides_ok,stats->num_sortides_error,stats->durada_total);
  return 0;
}

