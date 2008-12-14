#define MAJ 254
#define MIN 0

short int lock;
int proces_monitoritzat;
int sys_call_monitoritzat;

dev_t maj_min;
struct cdev *new_dev;

ssize_t pages_read_dev (struct file *f, char __user * buffer, size_t s,
			loff_t * off);
int pages_ioctl_dev (struct inode *i, struct file *f, unsigned int arg1,
		     unsigned long arg2);
int pages_open_dev (struct inode *i, struct file *f);
int pages_release_dev (struct inode *i, struct file *f);

int reset_valors (int pid);
void reset_tots_valors (void);
int activar_sys_call (int quina);
int desactivar_sys_call (int quina);

unsigned long copy_to_user (void *to, const void *from, unsigned long count);

struct file_operations file_ops = {
owner:THIS_MODULE,
read:pages_read_dev,
ioctl:pages_ioctl_dev,
open:pages_open_dev,
release:pages_release_dev,
};
