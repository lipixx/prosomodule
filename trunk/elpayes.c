#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include "include/mihuerto.h"
#include "include/elpayes.h"

MODULE_AUTHOR
  ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@ciutadella.es>");
MODULE_DESCRIPTION ("ProSO driver: estadistiques");
MODULE_LICENSE ("GPL");

/* Inicialitzacio del modul. */

static int __init
ir_al_huerto_init (void)
{
  int result;
  //int minor, major;

  proces_monitoritzat=pid;
  sys_call_monitoritzat=0;

  /* result = alloc_chrdev_region (&maj_min, 0, 1, "payes");
  if (result<0){
    printk("ERROR: alloc_chrdev_region");   
 return result;
 }*/
  maj_min = MKDEV(MAJ, MIN);
  result = register_chrdev_region (maj_min, 1, "payes");
  if(result<0){
    printk("ERROR: register_chrdev_region");
    return result;
    }
  new_dev = cdev_alloc ();
  if(new_dev==NULL){
    printk("ERROR: cdev_alloc");
    return -1;
  }
  new_dev->owner = THIS_MODULE;
  new_dev->ops = &file_ops;
  result= cdev_add (new_dev, maj_min, 1);
  if(result<0){
    printk("ERROR: cdev_add");
    return result;
  }
  lock = 0;

  /*major=MAJOR(maj_min);
  minor=MINOR(maj_min);
  printk("\nMAJMIN: %i %i\n",major,minor);
  */
  printk (KERN_DEBUG
	  "El pages arriba ben descansat a l'hort, preparat per una dura jornada de feina\n");

  return 0;
}

static void __exit
salir_del_huerto_exit (void)
{
  unregister_chrdev_region (maj_min, 1);
  cdev_del (new_dev);

  printk (KERN_DEBUG
	  "El pages ha arribat a casa sa i estalvi, ningu l'hi ha robat els melons\n");

}

module_init (ir_al_huerto_init);
module_exit (salir_del_huerto_exit);


ssize_t
pages_read_dev (struct file *f, char __user * buffer, size_t s, loff_t * off)
{
  /* Al llegir aquest dispositiu retornara a l espai d usuari (buffer) una estructura
     amb informacio sobre la crida a sistema monitoritzada actualment. L estructura ha
     d'haver estat creada previament. El nombre de bytes a llegir sera el minim entre s i
     sizeof(struct t_info). */

  /*  struct sysc_stats *crida = sysc_info_table[sys_call_monitoritzat];
      IMPROVEMENT?¿
   */  
  struct pid_stats info;
  size_t mida;
  int resultat;
  //unsigned long size;

  try_module_get(THIS_MODULE);  

  resultat =
    obtenir_estadistiques (proces_monitoritzat, sys_call_monitoritzat, &info);
  if (resultat < 0)
    return resultat;

  if (s < 0)
    return -EINVAL;
  if(s < sizeof (struct pid_stats)) mida = s;
  else mida = sizeof (struct pid_stats);

  module_put(THIS_MODULE);
  printk(KERN_DEBUG"\nhola\n%i%i%i%lld",info.num_entrades,info.num_sortides_ok,info.num_sortides_error,info.durada_total);
  return (ssize_t) copy_to_user (buffer,&info,mida);
  //  return (ssize_t) size;
}

int
pages_ioctl_dev (struct inode *i, struct file *f, unsigned int arg1,
		 unsigned long arg2)
{				/* JOSEP */
  /* Amb aquesta crida modificarem el comportament del dispositiu (proces
     seleccionat, etc). */
  int res;
  struct task_struct *task;
  struct th_info_est *thinfo_stats;
  
  try_module_get(THIS_MODULE);
  
  /*
    PREGUNTA:   NECESSITAM FER CAP COPY_FROM_USER ?¿ per agafar es parametres o ho podem fer a saco?
  */
  switch (arg1)
    {
    case 0:
      /*CANVI PROCES (arg1=0) El parametre arg2 indica, per referencia, el nou
	identificador de proces que cal analitzar. Si el punter es NULL, vol dir que cal
	tornar al proces que ha realitzat l open. Si el proces no existeix cal retornar
	error. */
      if (arg2 < 0)
	return -EINVAL;
      task = find_task_by_pid((pid_t)arg2);
      if (task == NULL)
	proces_monitoritzat = pid;
      else
	{
	  task = find_task_by_pid ((pid_t) arg2);
	  if (task == NULL)
	    return -ESRCH;
	  
	  thinfo_stats = (struct th_info_est *) task;
	  //p = current_thread_info ()->task->pid;
	  
	  if (arg2 != thinfo_stats->pid)
	    reset_info ((pid_t)arg2, thinfo_stats);
	  
	  proces_monitoritzat = arg2;
	}
      break;
    case 1:
      /* CANVI SYSCALL (arg1=1) Permet canviar la crida de la que consultem les
	 estadistiques. El parametre arg2 indica:
	 • OPEN (0)
	 • WRITE (1)
	 • LSEEK (2)
	 • CLOSE (3)
	 • CLONE (4)*/
      if (arg2 < 0 || arg2 >= N_CRIDES_A_MONITORITZAR)
	return -EINVAL;
      sys_call_monitoritzat = arg2;
      break;
    case 2:
      /* RESET_VALORS (arg1=2) Posa a zero els valors del proces que s esta
         analitzant en aquest moment. */
      res = reset_valors (proces_monitoritzat);
      break;
    case 3:
      /* RESET_VALORS_TOTS_PROCESSOS (arg1=3) Posa a zero els valors de tots els
         processos que s estan analitzant. */
      reset_tots_valors ();
      break;
    case 4:
      if (arg2 < -1 || arg2 >= N_CRIDES_A_MONITORITZAR)
	return -EINVAL;
      /* Cal tenir present que si arg2=-1 s activaran totes les sys_calls */
      activar_sys_call (arg2);
      break;
    case 5:
      if (arg2 < -1 || arg2 >= N_CRIDES_A_MONITORITZAR)
	return -EINVAL;
      /* Cal tenir present que si arg2=-1 es desactivaran totes les sys_calls */
      desactivar_sys_call (arg2);
      break;
    default:
      /* Parametre incorrecte */
      return -EINVAL;
      break;
    }
 module_put(THIS_MODULE);
  return 0;
}

int
pages_open_dev (struct inode *i, struct file *f)
{

  try_module_get(THIS_MODULE);
  printk("\nEN FELIP ES UN PUTA GAY DE MERDA\n lock: %i",lock);
  if (lock!=0) return -EPERM;
  if (current->uid!=0) return -EACCES;
  lock++;

  module_put(THIS_MODULE);
   
  return 0;  
}   

int
pages_release_dev (struct inode *i, struct file *f)
{
  try_module_get(THIS_MODULE);
  if (!lock) return -EPERM;
  lock--;

  module_put(THIS_MODULE);

  return 0;
}

int
reset_valors (pid_t pid)
{
  struct task_struct * t;

  if (pid < 0)
    return -EINVAL;

  t = find_task_by_pid (pid);

  if (t < 0)
    return -ESRCH;

  reset_info (pid, (struct th_info_est *) t->thread_info);

  return 0;
}

void
reset_tots_valors (void)
{
  struct task_struct *t;

  for_each_process (t)
    reset_info (t->pid, (struct th_info_est *) t->thread_info);
}

int
activar_sys_call (int quina)
{
  int i;
  if (quina < -1 || quina > N_CRIDES_A_MONITORITZAR)
    return -EINVAL;
  else if (quina != -1)
    activar_monitoritzacio (quina);
  else
    for (i = 0; i < N_CRIDES_A_MONITORITZAR; i++)
      activar_monitoritzacio (i);
  return 0;
}

int
desactivar_sys_call (int quina)
{
  int i;
  if (quina < -1 || quina > N_CRIDES_A_MONITORITZAR)
    return -EINVAL;
  else if (quina != -1)
    desactivar_monitoritzacio (quina);
  else
    for (i = 0; i < N_CRIDES_A_MONITORITZAR; i++)
      desactivar_monitoritzacio (i);
  return 0;
}

//Funcio inutil de moment creada per en felip moll marques
//i dictada per en josep marti pascual el rei de les funcions
//estupides i sense sentit i inutils, i que repetint ada es fi de puta
//te un pute vuid.

/*void
imprimir_estadistiques_sysc (int sysc)
{*/
  /* ESTA DESFASAT! HEM CANVIAT LES ESTRUCTURES, ARA TENIM UNA TAULA DE PID_STATS! */

/* struct sysc_stats *crida = (struct sysc_stats *) sysc_info_table[sysc];

  printk ("    --El Pages --\n");
  printk ("Num crides   : %i\n", crida->num_crides);
  printk ("Ret correcte : %i\n", crida->num_fallides);
  printk ("Ret erroni   : %i\n", crida->num_satisfactories);
  printk ("Temps total  : %lld\n", crida->temps_execucio);
  printk ("    -------------\n");

}*/



//PD: NO EMPRAM PER RES SA TAULA SYSC_INFO_TABLE !!!!!!!!!!!! IMPROVEMENET!!!!
