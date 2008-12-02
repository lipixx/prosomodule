
char lock;
int proces_monitoritzat;
int sys_call_monitoritzat;

struct dev_t * maj_min;
struct cdev * new_dev;

struct file_operations file_ops = {
 owner: THIS_MODULE;
 read: sys_read_dev;
 open: sys_open_dev;
 release: sys_release_dev;
 ioctl: sys_ioctl_dev;
};

extern sysc_info sysc_info_table[];
extern struct t_info;
extern pids_monitoritzats[];

extern N_CRIDES_A_MONITORITZAR
