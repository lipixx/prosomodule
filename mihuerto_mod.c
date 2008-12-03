#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include "include/mihuerto.h"

int pid_inicial = 1;
module_param(pid_inicial,int,0);

MODULE_AUTHOR ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION("ProSO driver: estadistiques");
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(pid_inicial,"PID del proces");
/* Inicialitzacio del modul. */

static int __init comprar_huerto_init(void)
{
   /* Codi d’inicialitzacio */

   /* Guardam les adreces originals de les crides a sistema */
  sys_call_table_originals[OPEN] = sys_call_table[POS_SYSCALL_OPEN]; ///////21
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
   
   printk(KERN_DEBUG "Hem comprat el nostre hort amb exit\n");

   return 0;
/* Aquesta funció retorna 0 si tot ha anat correctament i < 0 en cas d’error */
}

/* Descarrega el modul. */

static void __exit vender_huerto_exit(void)
{
  /* Codi de finalitzacio */
  //int adresa;

  //nou_pid=0; /* Inicialitzem la posicio on cal guardar els pids monitoritzats */

   /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
   sys_call_table[POS_SYSCALL_OPEN] = sys_call_table_originals[OPEN];
   sys_call_table[POS_SYSCALL_CLOSE] = sys_call_table_originals[CLOSE]; 
   sys_call_table[POS_SYSCALL_WRITE] = sys_call_table_originals[WRITE];
   sys_call_table[POS_SYSCALL_CLONE] = sys_call_table_originals[CLONE];
   sys_call_table[POS_SYSCALL_LSEEK] = sys_call_table_originals[LSEEK];

   //   adresa = pid_monitoritzat(pid_inicial); /* Retorna adresa del proces amb PID=pid, altrament retorna -1 */
   //if(adresa!=-1) imprimir_estadistiques(pid_inicial,&adresa);
   //else printk(KERN_DEBUG "El proces amb PID: "+pid_inicial+" ja la ha palmada!\n");

  printk(KERN_DEBUG "Hem vengut el nostre hort amb exit\n");

}

module_init(comprar_huerto_init);
module_exit(vender_huerto_exit);



   // try_module_get(THIS_MODULE);   <- aixo va a l'inici de cada funcio i NO aqui!
  //  module_put(THIS_MODULE); <- aixo va al final de cada funcio i NO aqui!

inline void init_est(struct th_info_est * tinfo_est, struct thread_info * mi_th_info,int NCRIDA)
{
  int pid;
  struct task_struct * t; /* de prova <-- TEMPORAL!! */

  mi_th_info = current_thread_info();
  tinfo_est = (struct th_info_est*) mi_th_info;
  t= current_thread_info()->task;
  pid = (int)t->pid;
  //pid = (int)(current_thread_info()->task)->pid;
  if (pid != tinfo_est->estadistiques->pid) reset_info(pid, tinfo_est);  
  tinfo_est->estadistiques->num_entrades++;     /* Incrementem el numero de crides per proces */      
  sysc_info_table[NCRIDA].num_crides++;	    /* Incrementem el numero de crides a la crida */
}

inline void fin_est(int resultat, struct th_info_est * tinfo_est,unsigned long long inici, unsigned long long final, int NCRIDA)
{
  if(resultat==0)					
    {							
      tinfo_est->estadistiques->num_sortides_ok++;			
      sysc_info_table[OPEN].num_satisfactories++;	
    } 
  else 
    {
      tinfo_est->estadistiques->num_sortides_error++;				
      sysc_info_table[OPEN].num_fallides++;			
    }		
  tinfo_est->estadistiques->durada_total += (final-inici);			
  sysc_info_table[OPEN].temps_execucio += (final-inici);
}

int
sys_open_local(const char __user * filename, int flags, int mode)
{
  SYS_CALL_GENERIC(OPEN,filename,flags,mode);
}

int
sys_close_local(unsigned int fd)
{
  SYS_CALL_GENERIC(CLOSE,fd);
}

int 
sys_write_local(unsigned int fd, const char __user * buf, size_t count)
{
  SYS_CALL_GENERIC(WRITE,fd,buf,count);
}

int
sys_clone_local(struct pt_regs regs)
{
  SYS_CALL_GENERIC(CLONE,regs);
}

int
sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin)
{
  SYS_CALL_GENERIC(LSEEK,fd,offset,origin);
}

int activar_monitoritzacio (int num_crida)
{
  /* Intercerptar les crides */
  
  /* Redireccionam les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_locals[num_crida];
  
  return 0;
}
EXPORT_SYMBOL(activar_monitoritzacio);

int desactivar_monitoritzacio (int num_crida)
{
  /* Desinterceptar les crides */
  
  /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_originals[num_crida];
  
  return 0;
}
EXPORT_SYMBOL(desactivar_monitoritzacio);

void reset_info(int pid, struct th_info_est * est)
{
  /* Inicialitzem les estadistiques */
  est->estadistiques->pid=pid;
  est->estadistiques->num_entrades=0;
  est->estadistiques->num_sortides_ok=0;
  est->estadistiques->num_sortides_error=0;
  est->estadistiques->durada_total=0;
}

void  imprimir_estadistiques(int pid)
{
  int n_crides, n_fall, n_sat, temps;
  struct th_info_est *tinfo_est; 
  struct thread_info * mi_th_info;
  
  cercar proces per PID :
  mi_th_info = adresa;
  tinfo_est = (struct th_info_est *) mi_th_info;
  
  n_crides = tinfo_est->estadistiques->num_entrades;
  n_fall = tinfo_est->estadistiques->num_sortides_ok;
  n_sat = tinfo_est->estadistiques->num_sortides_error;
  temps = tinfo_est->estadistiques->durada_total;
  
  printk(KERN_DEBUG "PID: %d\n",pid);
  printk(KERN_DEBUG "Nombre de crides: %d",n_crides);
  printk(KERN_DEBUG "Nombre sortides ok: %d\n",n_sat);
  printk(KERN_DEBUG "Nombre sortides error: %d\n",n_fall);
  printk(KERN_DEBUG "Durada total: %d\n",temps);
  
}
EXPORT_SYMBOL(imprimir_estadistiques);
