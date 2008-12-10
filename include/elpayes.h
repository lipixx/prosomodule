#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

char lock;
int proces_monitoritzat;
int sys_call_monitoritzat;

struct dev_t *maj_min;
struct cdev *new_dev;


int sys_read_dev ();
int sys_open_dev ();
int sys_release_dev ();
int sys_ioctl_dev ();
void reset_valors (int pid);
void reset_tots_valors ();
int activar_sys_call (int quina);
int desactivar_sys_call (int quina);


struct file_operations file_ops = {
  //  owner: THIS_MODULE,
read:sys_read_dev,
open:sys_open_dev,
release:sys_release_dev,
ioctl:sys_ioctl_dev,
};

/* Operacions de l altre modul */
extern void activar_monitoritzacio (int pid);
extern void desactivar_monitoritzacio (int pid);

extern sysc_info sysc_info_table[];
extern struct t_info;
extern pids_monitoritzats[];

extern N_CRIDES_A_MONITORITZAR
