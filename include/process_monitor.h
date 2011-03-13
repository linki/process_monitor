#include <pthread.h>

#define PM_DEFAULT_PROCFS_PATH    "/proc"
#define PM_DEFAULT_INTERVAL       2

typedef struct system_data    system_data_t;
typedef struct process_data   process_data_t;
typedef struct process_data   thread_data_t;
typedef struct cpu_data       cpu_data_t;
typedef struct memory_data    memory_data_t;
typedef struct meminfo        meminfo_t;

struct memory_data
{
   // in kbytes
   unsigned long peak;  // VmPeak: Peak virtual memory size.
   unsigned long total; // VmSize: Virtual memory size
   unsigned long rss;   // Resident set size.
};

struct meminfo
{
   // in kbytes
   unsigned long total; // MemTotal in kB
   unsigned long free;  // MemFree in kB
   unsigned long used;  // MemTotal - MemFree in kB
};

struct cpu_data
{
   unsigned long total;   // total is the sum of utime, stime and idle
   unsigned long utime;   // user mode
   unsigned long nice;    // user mode with low priority
   unsigned long stime;   // kernel mode
   unsigned long idle;    // idle task
   unsigned long iowait;  // time waiting for I/O to complete
   unsigned long irq;     // time  servicing  interrupts
   unsigned long softirq; // time servicing softirqs
   unsigned long steal;   // time spent in other operating systems when running in a virtualized environment
   unsigned long guest;   // time spent running a virtual CPU for guest operating systems under the control of the Linux kernel
};

struct system_data
{
   int        _cpu_count;
   cpu_data_t _cpus_data;
   cpu_data_t * _cpu_data;
   meminfo_t  _memory_data;
};

struct process_data
{
   int                _thread_count;
   thread_data_t      * _thread_data;
   memory_data_t      _memory_data;

   unsigned long      total;                 // total is the sum of utime and stime

   int                pid;                   // %d
   char               comm[20];              // %s executable name
   char               state;                 // %c char representing state
   int                ppid;                  // %d parent pid
   int                pgrp;                  // %d process group
   int                session;               // %d session
   int                tty_nr;                // %d controlling terminal
   int                tpgid;                 // %d foreground process
   unsigned int       flags;                 // %u kernel flags
   unsigned long      minflt;                // %lu minor page faults
   unsigned long      cminflt;               // %lu minor faults of waited-for children
   unsigned long      majflt;                // %lu major page faults
   unsigned long      cmajflt;               // %lu major faults of waited-for children

   unsigned long      utime;                 // %lu time spent in user mode in clock ticks (divide by sysconf(_SC_CLK_TCK)), includes guest time
   unsigned long      stime;                 // %lu time spent in kernel mode in clock ticks (divide by sysconf(_SC_CLK_TCK)

   unsigned long      cutime;                // %lu time spent in user mode of waited-for children in clock ticks (divide by sysconf(_SC_CLK_TCK)), includes guest time
   unsigned long      cstime;                // %lu time spent in kernel mode of waited-for children in clock ticks (divide by sysconf(_SC_CLK_TCK)

   long               priority;              // %ld
   long               nice;                  // nice value %ld

   long               num_threads;           // %ld

   long               itrealvalue;           // %ld unused, always 0

   unsigned long long starttime;             // %llu The time in jiffies the process started after system boot.
   unsigned long      vsize;                 // %lu Virtual memory size in bytes. = VMSize * 1024

   long               rss;                   // %ld Resident  Set  Size: number of pages the process has in real memory = VMRSS / 4
   unsigned long      rsslim;                // %lu  Current  soft  limit  in  bytes  on  the  rss of the process

   unsigned long      startcode;             // %lu The address above which program text can run.
   unsigned long      endcode;               // %lu The address below which program text can run.
   unsigned long      startstack;            // %lu The  address  of  the  start  (i.e.,  bottom) of the stack.
   unsigned long      kstkesp;               // %lu The current value of ESP (stack pointer), as found in the kernel stack page for the process.
   unsigned long      kstkeip;               // %lu The current EIP (instruction pointer).

   unsigned long      signal;                // %lu  The  bitmap of pending signals, Obsolete; use /proc/[pid]/status instead.
   unsigned long      blocked;               // %lu The bitmap of blocked signals, Obsolete; use proc/[pid]/status instead.
   unsigned long      sigignore;             // %lu The  bitmap of ignored signals, Obsolete; use /proc/[pid]/status instead.
   unsigned long      sigcatch;              // %lu The bitmap of caught signals, Obsolete; use /proc/[pid]/status instead.

   unsigned long      wchan;                 // %lu This  is the "channel" in which the process is waiting
   unsigned long      nswap;                 // %lu Number of pages swapped (not maintained).
   unsigned long      cnswap;                // %lu Cumulative nswap for child processes (not maintained).

   int                exit_signal;           // %d Signal to be sent to parent when we die.
   int                processor;             // %d CPU number last executed on.

   unsigned int       rt_priority;           // %u Real-time scheduling priority
   unsigned int       policy;                // %u Scheduling policy

   unsigned long long delayacct_blkio_ticks; // %llu Aggregated block I/O delays, measured in clock ticks (centiseconds).
   unsigned long      guest_time;            // %lu Guest time of the process (time spent running a virtual CPU for a guest operating system), measured in clock ticks (divide by sysconf(_SC_CLK_TCK).
   long               cguest_time;           // %ld Guest time  of  the process's children, measured in clock ticks (divide by sysconf(_SC_CLK_TCK).
};

class ProcessMonitor
{
public:

   int       _pid;
   int       _interval;
   char      * _procfs_path;

   pthread_t _runner;
   int       _running;

   void initialize(int pid, int interval, const char* procfs_path);

   system_data_t  _system_data;
   system_data_t  _last_system_data;

   process_data_t _process_data;
   process_data_t _last_process_data;

   char           * _system_stat_path;
   char           * _system_meminfo_path;
   char           * _process_stat_path;
   char           * _process_status_path;
   char           * _process_task_path;

   // constructors take pid, interval and path to procfs
   explicit ProcessMonitor(int pid);
   explicit ProcessMonitor(int pid, int interval);
   explicit ProcessMonitor(int pid, const char* procfs_path);
   explicit ProcessMonitor(int pid, int interval, const char* procfs_path);

   // destructor
   ~ProcessMonitor();

   // control
   int fetch();
   void start();
   void stop();

   // accessors
   int pid();
   int interval();
   char* procfs_path();
   int has_valid_procfs_path();
   int is_running();
   int num_cpus();
   int num_threads();

   // remember start time of processmonitor and scope these values in that timeframe
   // instead of the system start
   unsigned long cpus_clock_ticks_total();
   unsigned long cpu_clock_ticks_total(int cid);
   unsigned long cpus_clock_ticks_used();
   unsigned long cpu_clock_ticks_used(int cid);

   unsigned long system_mem_total();
   unsigned long system_mem_free();
   unsigned long system_mem_used();

   char state();
   char* executable_name();

   unsigned long process_mem_total();
   unsigned long process_mem_used();
   unsigned long process_mem_peak();

   double cpus_usage();
   double cpu_usage(int cid);

   double system_mem_usage();
   double process_mem_usage();

   double process_cpus_usage();

   double thread_cpus_usage(int tid);
   double process_thread_cpus_usage(int tid);

   // thread loop
   static void* run(void* data);

   // parsing
   void parse_system_data(system_data_t* system_data);
   void parse_process_data(int pid, process_data_t* process_data);
   void parse_process_threads(int pid, thread_data_t** thread_data, int* thread_count);
   void parse_thread(int pid, int tid, thread_data_t* thread_data);

   void parse_process_stat_file(int pid, process_data_t* stat);
   void parse_thread_stat_file(int pid, int tid, thread_data_t* stat);
   void parse_system_stat_file(system_data_t* system_data);

   void parse_process_status_file(int pid, memory_data_t* data);
   void parse_system_meminfo_file(meminfo_t* data);

   void parse_process_stat_stream(FILE* stream, process_data_t* stat);
   void parse_process_status_stream(FILE* stream, memory_data_t* data);
   void parse_system_stat_stream(FILE* stream, system_data_t* stat_data);
   void parse_meminfo_stream(FILE* stream, meminfo_t* data);

   int parse_system_stat_stream_for_cpu_count(FILE* stream);

   int parse_process_thread_ids(int pid, int** ptids);

   // copy helpers
   static void copy_system_data(system_data_t* dest_data, system_data_t* src_data);
   static void copy_process_data(process_data_t* dest_data, process_data_t* src_data);

   // path helpers
   static char* path_to(const char* procfs_path, const char* name);
   static char* path_to(const char* procfs_path, int pid, const char* name);
   static char* path_to(const char* procfs_path, int pid, int tid, const char* name);

   // initializers
   static void initialize_system_data(system_data_t* system_data);
   static void initialize_process_data(process_data_t* process_data);
   static void initialize_thread_data(thread_data_t** thread_data, int* thread_count);
};
