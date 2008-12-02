#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

int pid = 1;
module_param(pid,int,0);
MODULE_LICENSE("GPL");
MODULE_PARAM_DESC(pid, "Process ID to monitor (def 1)");
MODULE_AUTHOR("Felip Moll <lipixx@gmail.com>, Josep Marti <one.neuron@gmail.com>");
MODULE_DESCRIPTION("Modul que no fa res.");

/* Inicialitzem el modul */

static int __init Mymodule_init(void)
{
  printk(KERN_DEBUG "Modul que no fa res carregat!\n");
  return 0;
}

static void __exit Mymodule_exit(void)
{
}

module_init(Mymodule_init);
module_exit(Mymodule_exit);

