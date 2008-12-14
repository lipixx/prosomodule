#define POS_SYSCALL_OPEN   5
#define POS_SYSCALL_CLOSE  6
#define POS_SYSCALL_WRITE  4
#define POS_SYSCALL_CLONE  120
#define POS_SYSCALL_LSEEK  19

#define N_CRIDES_A_MONITORITZAR 5
#define MAX_PIDS_MONITORITZATS 1000

#define OPEN    0
#define CLOSE   1
#define WRITER  2
#define CLONE   3
#define LSEEK   4

typedef unsigned long long quad;
pid_t pid;

/* proso_get_cycles serveix per comptar el temps */
#define proso_rdtsc(low,high) \
  __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

extern void *sys_call_table[];
void *sys_call_table_originals[N_CRIDES_A_MONITORITZAR];
void *sys_call_table_locals[N_CRIDES_A_MONITORITZAR];

static inline quad
proso_get_cycles (void)
{
  unsigned long eax, edx;
  proso_rdtsc (eax, edx);
  return ((quad) edx << 32) + eax;
}

struct pid_stats
{
  //int pid;
  int num_entrades;
  int num_sortides_ok;
  int num_sortides_error;
  quad durada_total;
};

struct sysc_stats
{
  int num_crides;
  int num_fallides;
  int num_satisfactories;
  quad temps_execucio;
} sysc_info_table[N_CRIDES_A_MONITORITZAR];

struct th_info_est
{
  struct thread_info info_th;
  struct pid_stats estadistiques[N_CRIDES_A_MONITORITZAR + 1];	/* +1 pq aixi en aquesta posicio guardarem les estadistiques globals del proces */
  int pid;			/* pid nomes s utilitza per comprovar que les estadistiques guardades son del proces que toca */
};

/* Aquesta taula serveix per poder fer referencia a la posicio de 
la sys_call_table mitjansant les crides que nosaltres esteim monitoritzant */
int taula_de_constants[] = { 5, 6, 4, 120, 19 };

//static int __init comprar_huerto_init (void);
//static void __exit vender_huerto_exit (void);


long sys_open_local (const char __user * filename, int flags, int mode);
long sys_close_local (unsigned int fd);
ssize_t sys_write_local (unsigned int fd, const char __user * buf,
			 size_t count);
int sys_clone_local (struct pt_regs regs);
off_t sys_lseek_local (unsigned int fd, off_t offset, unsigned int origin);


void activar_monitoritzacio (int num_crida);
void desactivar_monitoritzacio (int num_crida);
void reset_info (int pid, struct th_info_est *tinfo_est);
void imprimir_estadistiques (int pid);
int obtenir_estadistiques (int pid, int crida, struct pid_stats * stats);

