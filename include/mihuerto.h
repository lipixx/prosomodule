#define POS_SYSCALL_OPEN   5
#define POS_SYSCALL_CLOSE  6
#define POS_SYSCALL_WRITE  4
#define POS_SYSCALL_CLONE  120
#define POS_SYSCALL_LSEEK  19  

#define N_CRIDES_A_MONITORITZAR 5
#define MAX_PIDS_MONITORITZATS 1000

#define OPEN   0
#define CLOSE  1
#define WRITE  2
#define CLONE  3
#define LSEEK  4

/* proso_get_cycles serveix per comptar el temps */
#define proso_rdtsc(low,high)				\
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

static inline unsigned long long proso_get_cycles (void) {
  unsigned long eax, edx;
  proso_rdtsc(eax, edx);
  return ((unsigned long long) edx << 32) + eax;
}


struct t_info {
  int pid;
  int num_entrades;
  int num_sortides_ok;
  int num_sortides_error;
  unsigned long long durada_total;
};

struct sysc_info{
  int num_crides;
  int num_fallides;
  int num_satisfactories;
  int temps_execucio;
};

int nou_pid; /* Indica la posicio de la taula pids_monitoritzats on hem de posar el proxim PID */
int pids_monitoritzats[MAX_PIDS_MONITORITZATS];

sysc_info sysc_info_table[N_CRIDES_A_MONITORITZAR];

struct th_info_est
{
  struct thread_info * info_th;
  struct t_info * estadistiques;
};

void * sys_call_table_original[N_CRIDES_A_MONITORITZAR];
void * sys_call_table_locals[N_CRIDES_A_MONITORITZAR];

/* Aquesta taula serveix per poder fer referencia a la posicio de 
la sys_call_table mitjansant les crides que nosaltres esteim monitoritzant */
int taula_de_constants[]={5,6,4,120,19};

static int __init comprar_huerto_init(void);
static void __exit vender_huerto_exit(void);
int sys_open_local(const char __user * filename, int flags, int mode);
void reset_info(int pid, struct th_info_est * tinfo_est);
inline void init_est(struct th_info_est * tinfo_est, struct thread_info * mi_th_info,int NCRIDA);
inline fin_est(int resultat, th_info_est tinfo_est, int NCRIDA);
int sys_open_local(const char __user * filename, int flags, int mode);
int sys_close_local(unsigned int fd);
int sys_write_local(unsigned int fd, const char __user * buf, size_t count);
int sys_clone_local(struct pt_regs regs);
int sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin);
int activar_monitoritzacio (int num_crida);
int desactivar_monitoritzacio (int num_crida);
void reset_info(int pid, struct th_info_est * est);
void  imprimir_estadistiques(int pid, int *adresa);
int pid_monitoritzat(int pid);
