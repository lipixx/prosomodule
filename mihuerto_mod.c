#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include "include/mihuerto.h"

pid_t pid_inicial = 1;
module_param(pid_inicial,int,0);
MODULE_PARM_DESC(pid_inicial,"PID del proces");
MODULE_AUTHOR ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION("ProSO driver: estadistiques");
MODULE_LICENSE("GPL");


static int __init comprar_huerto_init(void)
{
  struct task_struct * t;
    
  /* Guardam les adreces originals de les crides a sistema */
  sys_call_table_originals[OPEN] = sys_call_table[POS_SYSCALL_OPEN];
  sys_call_table_originals[CLOSE] = sys_call_table[POS_SYSCALL_CLOSE]; 
  sys_call_table_originals[WRITE] = sys_call_table[POS_SYSCALL_WRITE];
  sys_call_table_originals[CLONE] = sys_call_table[POS_SYSCALL_CLONE];
  sys_call_table_originals[LSEEK] = sys_call_table[POS_SYSCALL_LSEEK];

  /* Guardem les adreces de les funcions locals a la taula sys_call_table_locals */
  sys_call_table_locals[OPEN] =  sys_open_local;
  sys_call_table_locals[CLOSE] = sys_close_local; 
  sys_call_table_locals[WRITE] = sys_write_local;
  sys_call_table_locals[CLONE] = sys_clone_local;
  sys_call_table_locals[LSEEK] = sys_lseek_local;

   /* Redireccionam les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[POS_SYSCALL_OPEN] = sys_open_local;
  sys_call_table[POS_SYSCALL_CLOSE] = sys_close_local; 
  sys_call_table[POS_SYSCALL_WRITE] = sys_write_local;
  sys_call_table[POS_SYSCALL_CLONE] = sys_clone_local;
  sys_call_table[POS_SYSCALL_LSEEK] = sys_lseek_local;
 
  t = find_task_by_pid(pid_inicial);
  
  if (!t) 
    { 
      pid_inicial = 1;
      t = find_task_by_pid(pid_inicial);
      printk (KERN_DEBUG "El pid no existex, s'agafa el default 1");
    }
  
  reset_info(pid_inicial, (struct th_info_est *) t->thread_info);  
  printk(KERN_DEBUG "MiHuerto: Loaded\n");
  
  return 0;
}

/* Descarrega el modul. */

static void __exit vender_huerto_exit(void)
{
  /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[POS_SYSCALL_OPEN] = sys_call_table_originals[OPEN];
  sys_call_table[POS_SYSCALL_CLOSE] = sys_call_table_originals[CLOSE]; 
  sys_call_table[POS_SYSCALL_WRITE] = sys_call_table_originals[WRITE];
  sys_call_table[POS_SYSCALL_CLONE] = sys_call_table_originals[CLONE];
  sys_call_table[POS_SYSCALL_LSEEK] = sys_call_table_originals[LSEEK];
  
  imprimir_estadistiques(pid_inicial);
    
  printk(KERN_DEBUG "MiHuerto: Unloaded\n");
}

module_init(comprar_huerto_init);
module_exit(vender_huerto_exit);


int
sys_open_local(const char __user * filename, int flags, int mode)
{
  long (*crida)(const char __user* filename, int flags, int mode);
  quad inici, final;		  
  struct th_info_est * thinfo_stats;
  struct pid_stats * pidstats;
  int resultat;				  
  int pid;
  
  try_module_get(THIS_MODULE);
  
  crida = sys_call_table_originals[OPEN];
  thinfo_stats = (struct th_info_est * ) current_thread_info();
  pid = thinfo_stats->info_th->task->pid;
  pidstats = thinfo_stats->estadistiques;

  if (pid != pidstats->pid)
    reset_info(pid, thinfo_stats);  
  
  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;     
  sysc_info_table[OPEN].num_crides++;
  
  /* Comptem el temps de la crida */
  inici = proso_get_cycles();			  
  resultat = crida (filename, flags, mode);
  final = proso_get_cycles();		
  
  if(resultat>=0)
    {							
      pidstats->num_sortides_ok++;			
      sysc_info_table[OPEN].num_satisfactories++;	
    } 
  else 
    {
      pidstats->num_sortides_error++;				
      sysc_info_table[OPEN].num_fallides++;			
    }
  
  pidstats->durada_total += (final-inici);			
  sysc_info_table[OPEN].temps_execucio += (final-inici);

  module_put(THIS_MODULE);
  return resultat;
}

int
sys_close_local(unsigned int fd)
{
  long (*crida)(unsigned int fd);
  quad inici, final;		  
  struct th_info_est * thinfo_stats;
  struct pid_stats * pidstats;
  int resultat;				  
  int pid;
  
  try_module_get(THIS_MODULE);
  
  crida = sys_call_table_originals[CLOSE];
  thinfo_stats = (struct th_info_est * ) current_thread_info();
  pid = thinfo_stats->info_th->task->pid;
  pidstats = thinfo_stats->estadistiques;

  if (pid != pidstats->pid)
    reset_info(pid, thinfo_stats);  
  
  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;     
  sysc_info_table[CLOSE].num_crides++;
  
  /* Comptem el temps de la crida */
  inici = proso_get_cycles();			  
  resultat = crida (fd);
  final = proso_get_cycles();		
  
  if(resultat>=0)
    {							
      pidstats->num_sortides_ok++;			
      sysc_info_table[CLOSE].num_satisfactories++;	
    } 
  else 
    {
      pidstats->num_sortides_error++;				
      sysc_info_table[CLOSE].num_fallides++;			
    }
  
  pidstats->durada_total += (final-inici);			
  sysc_info_table[CLOSE].temps_execucio += (final-inici);

  module_put(THIS_MODULE);
  return resultat;
}

int 
sys_write_local(unsigned int fd, const char __user * buf, size_t count)
{
  long (*crida)(unsigned int fd, const char __user * buf, size_t count);
  quad inici, final;		  
  struct th_info_est * thinfo_stats;
  struct pid_stats * pidstats;
  int resultat;				  
  int pid;
  
  try_module_get(THIS_MODULE);
  
  crida = sys_call_table_originals[WRITE];
  thinfo_stats = (struct th_info_est * ) current_thread_info();
  pid = thinfo_stats->info_th->task->pid;
  pidstats = thinfo_stats->estadistiques;

  if (pid != pidstats->pid)
    reset_info(pid, thinfo_stats);  
  
  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;     
  sysc_info_table[WRITE].num_crides++;
  
  /* Comptem el temps de la crida */
  inici = proso_get_cycles();			  
  resultat = crida (fd,buf,count);
  final = proso_get_cycles();		
  
  if(resultat>=0)
    {							
      pidstats->num_sortides_ok++;			
      sysc_info_table[WRITE].num_satisfactories++;	
    } 
  else 
    {
      pidstats->num_sortides_error++;				
      sysc_info_table[WRITE].num_fallides++;			
    }
  
  pidstats->durada_total += (final-inici);			
  sysc_info_table[WRITE].temps_execucio += (final-inici);

  module_put(THIS_MODULE);
  return resultat;
}

int
sys_clone_local(struct pt_regs regs)
{
  long (*crida)(struct pt_regs regs);
  quad inici, final;		  
  struct th_info_est * thinfo_stats;
  struct pid_stats * pidstats;
  int resultat;				  
  int pid;
  
  try_module_get(THIS_MODULE);
  
  crida = sys_call_table_originals[CLONE];
  thinfo_stats = (struct th_info_est * ) current_thread_info();
  pid = thinfo_stats->info_th->task->pid;
  pidstats = thinfo_stats->estadistiques;

  if (pid != pidstats->pid)
    reset_info(pid, thinfo_stats);  
  
  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;     
  sysc_info_table[CLONE].num_crides++;
  
  /* Comptem el temps de la crida */
  inici = proso_get_cycles();			  
  resultat = crida (regs);
  final = proso_get_cycles();		
  
  if(resultat>=0)
    {							
      pidstats->num_sortides_ok++;			
      sysc_info_table[CLONE].num_satisfactories++;	
    } 
  else 
    {
      pidstats->num_sortides_error++;				
      sysc_info_table[CLONE].num_fallides++;			
    }
  
  pidstats->durada_total += (final-inici);			
  sysc_info_table[CLONE].temps_execucio += (final-inici);

  module_put(THIS_MODULE);
  return resultat;
}

int
sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin)
{
  long (*crida)(unsigned int fd, off_t offset, unsigned int origin);
  quad inici, final;		  
  struct th_info_est * thinfo_stats;
  struct pid_stats * pidstats;
  int resultat;				  
  int pid;
  
  try_module_get(THIS_MODULE);
  
  crida = sys_call_table_originals[LSEEK];
  thinfo_stats = (struct th_info_est * ) current_thread_info();
  pid = thinfo_stats->info_th->task->pid;
  pidstats = thinfo_stats->estadistiques;

  if (pid != pidstats->pid)
    reset_info(pid, thinfo_stats);  
  
  /*
   * Incrementem el numero de crides per proces i      
   * incrementem el numero de crides a la crida
   */
  pidstats->num_entrades++;     
  sysc_info_table[LSEEK].num_crides++;
  
  /* Comptem el temps de la crida */
  inici = proso_get_cycles();			  
  resultat = crida (fd,offset,origin);
  final = proso_get_cycles();		
  
  if(resultat>=0)
    {							
      pidstats->num_sortides_ok++;			
      sysc_info_table[LSEEK].num_satisfactories++;	
    } 
  else 
    {
      pidstats->num_sortides_error++;				
      sysc_info_table[LSEEK].num_fallides++;			
    }
  
  pidstats->durada_total += (final-inici);			
  sysc_info_table[LSEEK].temps_execucio += (final-inici);

  module_put(THIS_MODULE);
  return resultat;
}

int activar_monitoritzacio (int num_crida)
{
  /* Intercerptar les crides:
   * Redireccionam les crides a sistema amb les
   * adreces de les nostres crides monitoritzades 
   */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_locals[num_crida];
  
  return 0;
}
EXPORT_SYMBOL(activar_monitoritzacio);

int desactivar_monitoritzacio (int num_crida)
{
  /* Desinterceptar les crides:
   * Restauram les crides a sistema amb les
   * adreces de les nostres crides monitoritzades 
   */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_originals[num_crida];
  
  return 0;
}
EXPORT_SYMBOL(desactivar_monitoritzacio);

void reset_info(int pid, struct th_info_est * est)
{
  struct pid_stats * pidstats;

  pidstats = est->estadistiques;

  /* Re-Inicialitzem les estadistiques */
  pidstats->pid=pid;
  pidstats->num_entrades=0;
  pidstats->num_sortides_ok=0;
  pidstats->num_sortides_error=0;
  pidstats->durada_total=0;
}

void  imprimir_estadistiques(int pid)
{
  struct task_struct * task_pid;
  struct pid_stats * pidstats;
  
  //Cercar proces per PID:
  task_pid =  find_task_by_pid((pid_t) pid);
  
  if (task_pid)
    {
      pidstats = ((struct th_info_est *) task_pid->thread_info)->estadistiques; 

      printk("    --MiHuerto --\n");
      printk("Pid          : %i\n",pidstats->pid);
      printk("Num crides   : %i\n",pidstats->num_entrades);
      printk("Ret correcte : %i\n",pidstats->num_sortides_ok);
      printk("Ret erroni   : %i\n",pidstats->num_sortides_error);
      printk("Temps total  : %i\n",pidstats->durada_total);
      printk("    -------------\n");
    }
  else
    printk("MiHuerto: No such pid\n");
}
EXPORT_SYMBOL(imprimir_estadistiques);
