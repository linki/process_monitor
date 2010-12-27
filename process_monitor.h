#include <stdio.h>
#include <pthread.h>

#define DEFAULT_PROCFS_PATH "/proc"

struct meminfo
{
    unsigned long long total; // MemTotal in kB
    unsigned long long free; // MemFree in kB
};

typedef struct meminfo meminfo_t;

struct process_datam
{
    unsigned long long size; // total program size
    unsigned long long resident; // resident set size
    unsigned long long share; // shared pages (from shared mappings)
    unsigned long long text; // text (code)
    unsigned long long lib; // library (unused in Linux 2.6)
    unsigned long long data; // data + stack
    unsigned long long dt; // dirty pages (unused in Linux 2.6)
};

typedef struct process_datam process_datam_t;

struct cpu_data
{
    unsigned long total; // total is the sum of utime, stime and idle
    unsigned long utime; // user mode
    unsigned long nice; // user mode with low priority
    unsigned long stime; // kernel mode
    unsigned long idle; // idle task
    unsigned long iowait; // time waiting for I/O to complete
    unsigned long irq; // time  servicing  interrupts
    unsigned long softirq; // time servicing softirqs
    unsigned long steal; // time spent in other operating systems when running in a virtualized environment
    unsigned long guest; // time spent running a virtual CPU for guest operating systems under the control of the Linux kernel
};

typedef struct cpu_data cpu_data_t;

struct system_data
{
    int cpu_count;
    cpu_data_t cpus;
    cpu_data_t* cpu;
};

typedef struct system_data system_data_t;

struct process_data;

typedef struct process_data process_data_t;
typedef struct process_data thread_data_t;
    
struct process_data
{
    int _threads;
    
    thread_data_t* _thread_data;
    
    process_datam_t _memory;
    

    
    unsigned long total; // total is the sum of utime and stime
    

    int pid; // %d
    char comm[20]; // %s executable name
    char state; // %c char representing state
    int ppid; // %d parent pid
    int pgrp; // %d process group
    int session; // %d session
    int tty_nr; // %d controlling terminal
    int tpgid; // %d foreground process
    unsigned int flags; // %u kernel flags
    unsigned long minflt; // %lu minor page faults
    unsigned long cminflt; // %lu minor faults of waited-for children
    unsigned long majflt; // %lu major page faults
    unsigned long cmajflt; // %lu major faults of waited-for children

    unsigned long utime; // %lu time spent in user mode in clock ticks (divide by sysconf(_SC_CLK_TCK)), includes guest time
    unsigned long stime; // %lu time spent in kernel mode in clock ticks (divide by sysconf(_SC_CLK_TCK)
    
    unsigned long cutime; // %lu time spent in user mode of waited-for children in clock ticks (divide by sysconf(_SC_CLK_TCK)), includes guest time
    unsigned long cstime; // %lu time spent in kernel mode of waited-for children in clock ticks (divide by sysconf(_SC_CLK_TCK)

    long priority; // %ld
    long nice; // nice value %ld

    long num_threads; // %ld

    long itrealvalue; // %ld unused, always 0

    unsigned long long starttime; // %llu The time in jiffies the process started after system boot.
    unsigned long vsize; // %lu Virtual memory size in bytes.

    long rss; // %ld Resident  Set  Size: number of pages the process has in real memory
    unsigned long rsslim; // %lu  Current  soft  limit  in  bytes  on  the  rss of the process

    unsigned long startcode; // %lu The address above which program text can run.
    unsigned long endcode; // %lu The address below which program text can run.
    unsigned long startstack; // %lu The  address  of  the  start  (i.e.,  bottom) of the stack.
    unsigned long kstkesp; // %lu The current value of ESP (stack pointer), as found in the kernel stack page for the process.
    unsigned long kstkeip; // %lu The current EIP (instruction pointer).

    unsigned long signal; // %lu  The  bitmap of pending signals, Obsolete; use /proc/[pid]/status instead.
    unsigned long blocked; // %lu The bitmap of blocked signals, Obsolete; use proc/[pid]/status instead.
    unsigned long sigignore;// %lu The  bitmap of ignored signals, Obsolete; use /proc/[pid]/status instead.
    unsigned long sigcatch; // %lu The bitmap of caught signals, Obsolete; use /proc/[pid]/status instead.

    unsigned long wchan; // %lu This  is the "channel" in which the process is waiting
    unsigned long nswap; // %lu Number of pages swapped (not maintained).
    unsigned long cnswap; // %lu Cumulative nswap for child processes (not maintained).

    int exit_signal; // %d Signal to be sent to parent when we die.
    int processor; // %d CPU number last executed on.

    unsigned int rt_priority; // %u Real-time scheduling priority
    unsigned int policy; // %u Scheduling policy

    unsigned long long delayacct_blkio_ticks; // %llu Aggregated block I/O delays, measured in clock ticks (centiseconds).
    unsigned long guest_time; // %lu Guest time of the process (time spent running a virtual CPU for a guest operating system), measured in clock ticks (divide by sysconf(_SC_CLK_TCK).
    long cguest_time; // %ld Guest time  of  the process's children, measured in clock ticks (divide by sysconf(_SC_CLK_TCK).
};

class ProcessMonitor
{
    pthread_t _runner;

    int _pid;
    unsigned _interval;
    char* _procfs_path;
    
public:

    int __threads;

    cpu_data_t __system_stat;
    cpu_data_t __last_system_stat;    
    
    system_data_t _system_data;
    system_data_t _last_system_data;

    process_data_t _process_data;
    process_data_t _last_process_data;
    
    meminfo_t _memory_data;

    // constructors
    explicit ProcessMonitor(int pid);
    
    // methods
    void fetch();
    void parse_process(int pid, process_data_t* process_data);    
    
    void parse_process_stat_file(int pid, process_data_t* stat);
    void parse_thread_stat_file(int pid, int tid, thread_data_t* stat);
    void parse_system_stat(system_data_t* system_data);
    
    void parse_process_statm_file(int pid, process_datam_t* data);
    void parse_meminfo(meminfo_t* data);
            
    void parse_thread_stat_stream(FILE* stream, process_data_t* stat);
    void parse_stat_data(FILE* stream, system_data_t* stat_data);

    void parse_process_statm_stream(FILE* stream, process_datam_t* data);
    void parse_meminfo_data(FILE* stream, meminfo_t* data);    
    void parse_cpu_count_data(FILE* stream, system_data_t* stat_data);
    
    void parse(const char* stream);
    
    void parse_cpu_count(system_data_t* stat_data);
    
    int parse_thread_count(int pid);
    int parse_thread_ids(int pid, int** ptids);

    // control
    static void* run(void* data);
    void start();
    void stop();

    // extended accessors
    char* get_path(const char* name, char** path);
    char* get_path(int pid, const char* name, char** path);
    char* get_path(int pid, int tid, const char* name, char** path);    

    // accessors
    int pid();
    unsigned interval();
    char* procfs_path();
    void procfs_path(const char* procfs_path);

    int cpu_usage();
    int cpu_usage(int cid);
    
    int process_cpu_usage();

    int global_thread_cpu_usage(int tid);
    int thread_cpu_usage(int tid);    

    int cpu_count();
    
    unsigned long cpus();
    unsigned long cpu(int cid);

    int threads();
    
    unsigned long utime();
    unsigned long utime(int tid);
    
    char state();
    
    unsigned long mem_total();
    unsigned long mem_free();
    
    void copy_system_data(system_data_t* dest_data, system_data_t* src_data);
    void copy_process_data(process_data_t* dest_data, process_data_t* src_data);    



};
