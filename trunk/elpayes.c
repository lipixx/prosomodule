#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include "include/elpayes.h"

MODULE_AUTHOR
  ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION ("ProSO driver: estadistiques");
MODULE_LICENSE ("GPL");

/* Inicialitzacio del modul. */

static int __init
ir_al_huerto_init (void)
{
  int result;

  result= alloc_chrdev_region (maj_min, 0, 1, "payes");
  if(result==-1) return -1;
  register_chrdev_region (*maj_min, 1, "payes");
  new_dev = cdev_alloc ();
  new_dev->owner = THIS_MODULE;
  new_dev->ops = &file_ops;
  cdev_add (new_dev, *maj_min, 1);
  lock = 0;

  printk (KERN_DEBUG
	  "El pages arriba ben descansat a l'hort, preparat per una dura jornada de feina\n");

  return 0;
}

static void __exit
salir_del_huerto_exit (void)
{
  unregister_chrdev_region (*maj_min,1);
  cdev_del (new_dev);

  printk (KERN_DEBUG
	  "El pages ha arribat a casa sa i estalvi, ningu l'hi ha robat els melons\n");

}

module_init (ir_al_huerto_init);
module_exit (salir_del_huerto_exit);

ssize_t sys_read_dev(struct file *f, char __user *buffer, size_t s, loff_t *off){
  return 0;
}
int sys_ioctl_dev(struct inode *i, struct file *f, unsigned int arg1, unsigned long arg2){
  return 0;
}
int sys_open_dev(struct inode *i, struct file *f){
  return 0;  
}   
int sys_release_dev(struct inode *i, struct file *f){
  return 0;
}

void
reset_valors (int proces_monitoritzat)
{
  
}

void
reset_tots_valors (void)
{

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
