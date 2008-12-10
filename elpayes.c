#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include "include/elpayes.h"

MODULE_AUTHOR
  ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION ("ProSO driver: estadistiques");
MODULE_LICENSE ("GPL");

/* Inicialitzacio del modul. */

static int __init
ir_al_huerto_init (void)
{

  alloc_chrdev_region (maj_min, 0, 1, "payes");
  register_chrdev_region (maj_min, 1, "payes");
  new_dev = cdev_alloc ();
  new_dev->owner = THIS_MODULE;
  new_dev->ops = &file_ops;
  c_dev_add (new_dev, maj_min, 1);
  lock = 0;

  printk (KERN_DEBUG
	  "El pages arriba ben descansat a l'hort, preparat per una dura jornada de feina\n");

  return 0;
}

static void __exit
salir_del_huerto (void)
{
  unregister_chrdev (maj_min, 1);
  cdev_del (dew_dev);

  printk (KERN_DEBUG
	  "El pages ha arribat a casa sa i estalvi, ningu l'hi ha robat els melons\n");

}

module_init (ir_al_huerto_init);
module_exit (salir_del_huerto_exit);


int
sys_open_dev ()
{
  return 0;
}

int
sys_read_dev ()
{
  return 0;
}

int
sys_release_dev ()
{
  return 0;
}

int
sys_ioctl_dev ()
{
  return 0;
}

void
reset_valors (int proces_monitoritzat)
{

}

void
reset_tots_valors ()
{

}

int
activar_sys_call (int quina)
{
  int i;
  if (quina < -1 || quina > N_CRIDES_A_MONITORITZAR)
    return -EINVAL;
  else if (quina != -1)
    activar_monitoritzatcio (quina);
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
