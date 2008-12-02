#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <elpayes.h>


MODULE_AUTHOR ("Josep Marti <one.neuron@gmail.com>, Felip Moll <lipixx@gmail.com>");
MODULE_DESCRIPTION("ProSO driver: estadistiques");
MODULE_LICENSE("GPL");

/* Inicialitzacio del modul. */

static int __init ir_al_huerto_init(void)
{

  alloc_chrdev_region(maj_min, 0, 1,"/dev/payes");
  register_chrdev_region(maj_min, 1, "/dev/payes");
  new_dev = cdev_alloc();
  new_dev->owner = THIS_MODULE;
  new_dev->ops=&file_ops;
  c_dev_add(new_dev,maj_min,1);
  lock=0;
  return 0;
}

static void __exit salir_del_huerto(void)
{

}
