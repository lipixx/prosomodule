#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

char lock;
int proces_monitoritzat;
int sys_call_monitoritzat;

struct dev_t *maj_min;
struct cdev *new_dev;


ssize_t sys_read_dev(struct file *f, char __user *buffer, size_t s, loff_t *off);
int sys_ioctl_dev(struct inode *i, struct file *f, unsigned int arg1, unsigned long arg2);
int sys_open_dev(struct inode *i, struct file *f);
int sys_release_dev(struct inode *i, struct file *f);

void reset_valors (int pid);
void reset_tots_valors (void);
int activar_sys_call (int quina);
int desactivar_sys_call (int quina);


struct file_operations file_ops = {
 owner: THIS_MODULE,
 read: sys_read_dev,
 open: sys_open_dev,
 release: sys_release_dev,
 ioctl: sys_ioctl_dev,
};

/* Operacions de l altre modul */
extern void activar_monitoritzacio (int num_crida);
extern void desactivar_monitoritzacio (int num_crida);

//extern int N_CRIDES_A_MONITORITZAR;
#define N_CRIDES_A_MONITORITZAR 5	/*NOMES DE PROVA*/
extern struct sysc_stats *sysc_info_table[N_CRIDES_A_MONITORITZAR];
//extern struct t_info;



