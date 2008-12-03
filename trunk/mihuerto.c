#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <mihuerto.h>

int pid = 1;
module_param(pid,int,0);

MODULE_AUTHOR ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION("ProSO driver: estadistiques");
MODULE_LICENSE("GPL");
MODULE_PARAM_DESC(pid,"PID del proces");
/* Inicialitzacio del modul. */

static int __init comprar_huerto_init(void)
{
   /* Codi d’inicialitzacio */

   /* Guardam les adreces originals de les cirdes a sistema */
   sys_call_table_originals[OPEN] = sys_call_table[POS_SYSCALL_OPEN];
   sys_call_table_originals[CLOSE] = sys_call_table[POS_SYSCALL_CLOSE]; 
   sys_call_table_originals[WRITE] = sys_call_table[POS_SYSCALL_WRITE];
   sys_call_table_originals[CLONE] = sys_call_table[POS_SYSCALL_CLONE];
   sys_call_table_originals[LSEEK] = sys_call_table[POS_SYSCALL_LSEEK];

   /* Guardem les adreces de les funcions locals a la taula sys_call_table_locals */
   sys_call_table_locals[OPEN] = sys_open_local;
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
  int adresa;
  nou_pid=0; /* Inicialitzem la posicio on cal guardar els pids monitoritzats */

   /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
   sys_call_table[POS_SYSCALL_OPEN] = sys_call_table_original[OPEN];
   sys_call_table[POS_SYSCALL_CLOSE] = sys_call_table_original[CLOSE] ; 
   sys_call_table[POS_SYSCALL_WRITE] = sys_call_table_original[WRITE];
   sys_call_table[POS_SYSCALL_CLONE] = sys_call_table_original[CLONE];
   sys_call_table[POS_SYSCALL_LSEEK] = sys_call_table_original[LSEEK];

   adresa = pid_monitoritzat(pid); /* Retorna adresa del proces amb PID=pid, altrament retorna -1 */
   if(adresa!=-1) imprimir_estadistiques(pid,adresa);
   else printk(KERN_DEBUG "El proces amb PID: "+pid+" ja la ha palmada!\n");

  printk(KERN_DEBUG "Hem vengut el nostre hort amb exit\n");

}

module_init(comprar_huerto_init);
module_exit(vender_huerto_exit);



   // try_module_get(THIS_MODULE);   <- aixo va a l'inici de cada funcio i NO aqui!
  //  module_put(THIS_MODULE); <- aixo va al final de cada funcio i NO aqui!

#define INIT_EST(tinfo_est,mi_th_info,NCRIDA) /
    {
      int pid;
      mi_th_info = get_thread_info();
      tinfo_est = (th_info_est) mi_th_info;
      pid = current()->pid;
  
      if (pid != tinfo_est->estadistiques.pid) reset_info(pid, tinfo_est);
  
      tinfo_est->estadistiques->num_entrades++;     /* Incrementem el numero de crides per proces */      
      sysc_info_table[NCRIDA]->num_crides++;	    /* Incrementem el numero de crides a la crida */
    }

#define FIN_EST(resultat, tinfo_est, NCRIDA) /
  {
    /* Actualitzem les estadistiques del proces i de la crida */
    if(resultat==0){
      tinfo_est->sortides_ok++;
      sys_info_table[OPEN]->sortides_satisfactories++;  
    } else {
      tinfo_est->sortides_error++;
      sys_info_table[OPEN]->sortides_fallides++;  
    }
    tinfo_est->durada_total += (final-inici);
    sys_info_table[OPEN]->temps_execucio += (final-inici);
  }

int
sys_open_local(const char __user * filename, int flags, int mode)
{
  unsigned long long inici, final;
  struct th_info_est * tinfo_est;
  struct thread_info * mi_th_info;

  INIT_EST(tinfo_est,mi_th_info,OPEN);

  inici = proso_get_cycles();
  resultat = ((void *) sys_call_table_originals[OPEN]) (filename, flags, mode);
  final = proso_get_cycles();
  
  FIN_EST(resultat,tinfo_est,OPEN);

  return resultat;
}

int sys_close_local(unsigned int fd){
  int pid, pid_est;
  unsigned long long inici, final;
  union task_union *tinfo_est;
  struct thread_info * mi_th_info;
  
  
  mi_th_info = get_thread_info();
  tinfo_est = (th_info_est) mi_th_info;
  pid = current()->pid;
  pid_est = tinfo_est->estadistiques.pid;
  
  if(pid != pid_est){
    reset_info(pid, tinfo_est);
    /* Treure pid_est de la taula -> llista!! */
    pids_monitoritzats[nou_pid++]=pid; /* afegir PID a la taula -> llista*/
  }
  
  tinfo_est->entrades++; /* Incrementem el numero de crides per proces */
  sysc_info_table[CLOSE]->num_crides++; /* Incrementem el numero de crides a la crida */
  
  inici = proso_get_cycles();
  resultat = ((void *) sys_call_table_originals[CLOSE]) (fd);
  final = proso_get_cycles();

  /* Actualitzem les estadistiques del proces i de la crida */
  if(reultat==0){
    tinfo_est->sortides_ok++;
    sys_info_table[CLOSE]->sortides_satisfactories++;  
  } else{
    tinfo_est->sortides_error++;
    sys_info_table[CLOSE]->sortides_fallides++;  
  }
  tinfo_est->durada_total += (final-inici);
  sys_info_table[CLOSE]->temps_execucio += (final-inici);

  return resultat;
}

int sys_write_local(unsigned int fd, const char __user * buf, size_t
count){

   int pid, pid_est;
  unsigned long long inici, final;
  union task_union *tinfo_est;
  struct thread_info * mi_th_info;


  mi_th_info = get_thread_info();
  tinfo_est = (th_info_est) mi_th_info;
  pid = current()->pid;
  pid_est = tinfo_est->estadistiques.pid;
  
  if(pid != pid_est){
    reset_info(pid, tinfo_est);
    /* Treure pid_est de la taula -> llista!! */
    pids_monitoritzats[nou_pid++]=pid; /* afegir PID a la taula -> llista*/
  }
  
  tinfo_est->entrades++; /* Incrementem el numero de crides per proces */
  sysc_info_table[WRITE]->num_crides++; /* Incrementem el numero de crides a la crida */

  inici = proso_get_cycles();
  resultat = ((void *) sys_call_table_originals[WRITE]) (fd, buf, count);
  final = proso_get_cycles();

  /* Actualitzem les estadistiques del proces i de la crida */
  if(reultat==0){
    tinfo_est->sortides_ok++;
    sys_info_table[WRITE]->sortides_satisfactories++;  
  } else{
    tinfo_est->sortides_error++;
    sys_info_table[WRITE]->sortides_fallides++;  
  }
  tinfo_est->durada_total += (final-inici);
  sys_info_table[WRITE]->temps_execucio += (final-inici);

  return resultat;
}

int sys_clone_local(struct pt_regs regs){

  int pid, pid_est;
  unsigned long long inici, final;
  union task_union *tinfo_est;
  struct thread_info * mi_th_info;


  mi_th_info = get_thread_info();
  tinfo_est = (th_info_est) mi_th_info;
  pid = current()->pid;
  pid_est = tinfo_est->estadistiques.pid;
  
  if(pid != pid_est){
    reset_info(pid, tinfo_est);
    /* Treure pid_est de la taula -> llista!! */
    pids_monitoritzats[nou_pid++]=pid; /* afegir PID a la taula -> llista*/
  }
  
  tinfo_est->entrades++; /* Incrementem el numero de crides per proces */
  sysc_info_table[CLONE]->num_crides++; /* Incrementem el numero de crides a la crida */

  inici = proso_get_cycles();
  resultat = ((void *) sys_call_table_originals[CLONE]) (regs);
  final = proso_get_cycles();

  /* Actualitzem les estadistiques del proces i de la crida */
  if(reultat==0){
    tinfo_est->sortides_ok++;
    sys_info_table[CLONE]->sortides_satisfactories++;  
  } else{
    tinfo_est->sortides_error++;
    sys_info_table[CLONE]->sortides_fallides++;  
  }
  tinfo_est->durada_total += (final-inici);
  sys_info_table[CLONE]->temps_execucio += (final-inici);

  return resultat;
}

int sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin){

   int pid, pid_est;
  unsigned long long inici, final;
  union task_union *tinfo_est;
  struct thread_info * mi_th_info;


  mi_th_info = get_thread_info();
  tinfo_est = (th_info_est) mi_th_info;
  pid = current()->pid;
  pid_est = tinfo_est->estadistiques.pid;
  
  if(pid != pid_est){
    reset_info(pid, tinfo_est);
    /* Treure pid_est de la taula -> llista!! */
    pids_monitoritzats[nou_pid++]=pid; /* afegir PID a la taula -> llista*/
  }
  
  tinfo_est->entrades++; /* Incrementem el numero de crides per proces */
  sysc_info_table[LSEEK]->num_crides++; /* Incrementem el numero de crides a la crida */

  inici = proso_get_cycles();
  resultat = ((void *) sys_call_table_originals[LSEEK]) (fd, offset, origin);
  final = proso_get_cycles();

  /* Actualitzem les estadistiques del proces i de la crida */
  if(reultat==0){
    tinfo_est->sortides_ok++;
    sys_info_table[LSEEK]->sortides_satisfactories++;  
  } else{
    tinfo_est->sortides_error++;
    sys_info_table[LSEEK]->sortides_fallides++;  
  }
  tinfo_est->durada_total += (final-inici);
  sys_info_table[LSEEK]->temps_execucio += (final-inici);

  return resultat;
}

EXPORT int activar_monitoritzacio (int num_crida){
  /* Intercerptar les crides */
  
  /* Redireccionam les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_locals[num_crida];
  
  return 0;
}

EXPORT int desactivar_monitoritzacio (int num_crida){
  /* Desinterceptar les crides */
  
  /* Restauram les crides a sistema amb les adreces de les nostres crides monitoritzades */
  sys_call_table[taula_de_constants[num_crida]] = sys_call_table_originals[num_crida];
  
  return 0;
}

void reset_info(int pid, struct th_info_est * est)
{
  /* Inicialitzem les estadistiques */
  est->pid=pid;
  est->num_entrades=0;
  est->num_sortides_ok=0;
  est->num_sortides_error=0;
  est->durada_total=0;
}

void  imprimir_estadistiques(int pid, int *adresa){
  int n_crides, n_fall, n_sat, temps;
  union task_union *tinfo_est; 
  struct thread_info * mi_th_info;
  
  mi_th_info = adresa;
  tinfo_est = (th_info_est) mi_th_info;
 
  n_crides = tinfo_est->estadistiques.num_entrades;
  n_fall = tinfo_est->estadistiques.num_sortides_ok;
  n_sat = tinfo_est->estadistiques.num_sortides_error;
  temps = unio.tinfo_est->estadistiques.durada_total;

   printk(KERN_DEBUG "PID: "+pid+"\n");
   printk(KERN_DEBUG "Nombre de crides: "+n_crides+"\n");
   printk(KERN_DEBUG "Nombre sortides ok: "+n_sat+"\n");
   printk(KERN_DEBUG "Nombre sortides error: "+n_fall+"\n");
   printk(KERN_DEBUG "Durada total: "+temps+"\n");

}

int pid_monitoritzat(int pid){
  /* Indica si hi ha algun proces amb el PID pid a la taula (llista) de pids_monitoritzats,
     i retorna l'adresa del proces en cas afirmatiu, i -1 en cas negatiu */
  return 0;
}
