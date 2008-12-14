#define CH_PID 0
#define CH_SYSC 1
#define RESET_VALS 2
#define RESET_ALL_VALS 3
#define ACT_SYSC 4
#define DEACT_SYSC 5

struct pid_stats{
  int num_entrades;
  int num_sortides_ok;
  int num_sortides_error;
  unsigned long long durada_total;
};
