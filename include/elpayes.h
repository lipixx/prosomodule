char lock;
int proces_monitoritzat;
int sys_call_monitoritzat;

dev_t *maj_min; 
struct cdev *new_dev;
typedef unsigned long long quad;

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

/*Aquesta estructura es la que es retorna a
  l'usuari a un sys_release, segons l'enunciat!!*/
/* Crec que val mes no repetir coses, aixo ja ho tenim a mihuerto */
/* A s enunciat posa t_info, i a mihuerto empram pid_stats */
/*struct tinfo {
  int num_entrades;
  int num_sortides_ok;
  int num_sortides_error;
  quad durada_total;
  }*/

/* Operacions de l altre modul */
extern void activar_monitoritzacio (int num_crida);
extern void desactivar_monitoritzacio (int num_crida);

extern struct sysc_stats sysc_info_table[N_CRIDES_A_MONITORITZAR];
