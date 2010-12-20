#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *stream;
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

    stream = fopen("/proc/1580/stat", "r");

    /* Put in various data. */
    fscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld", &pid, &comm, &state, &ppid, &pgrp, &session, &tty_nr,
                                              &tpgid, &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
                                              &cutime, &cstime, &priority, &nice, &num_threads, &itrealvalue,
                                              &starttime, &vsize, &rss, &rsslim, &startcode, &endcode, &startstack,
                                              &kstkesp, &kstkeip, &signal, &blocked, &sigignore, &sigcatch, &wchan,
                                              &nswap, &cnswap, &exit_signal, &processor, &rt_priority, &policy,
                                              &delayacct_blkio_ticks, &guest_time, &cguest_time );

    printf("%d.%s.%c.%d.%d.%d.%d.%d.%u.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%ld.%ld.%ld.%ld.%llu.%lu.%ld.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%lu.%d.%d.%u.%u.%llu.%lu.%ld\n", pid, comm, state, ppid, pgrp, session, tty_nr,
                                     tpgid, flags, minflt, cminflt, majflt, cmajflt, utime, stime,
                                     cutime, cstime, priority, nice, num_threads, itrealvalue,
                                     starttime, vsize, rss, rsslim, startcode, endcode, startstack,
                                     kstkesp, kstkeip, signal, blocked, sigignore, sigcatch, wchan,
                                     nswap, cnswap, exit_signal, processor, rt_priority, policy,
                                     delayacct_blkio_ticks, guest_time, cguest_time);

    unsigned long long size; // total program size
    unsigned long long resident; // resident set size
    unsigned long long share; // shared pages (from shared mappings)
    unsigned long long text; // text (code)
    unsigned long long lib; // library (unused in Linux 2.6)
    unsigned long long data; // data + stack
    unsigned long long dt; // dirty pages (unused in Linux 2.6)

    stream = fopen("/proc/1580/statm", "r");

    /* Put in various data. */
    fscanf(stream, "%llu %llu %llu %llu %llu %llu %llu", &size, &resident, &share, &text, &lib, &data, &dt);

    printf("%llu.%llu.%llu.%llu.%llu.%llu.%llu", size, resident, share, text, lib, data, dt);

    // each thread has a directory in /proc/pid/task/

    double loadavg_1;
    double loadavg_5;
    double loadavg_15;
    unsigned long current;
    unsigned long total;
    int last;


    stream = fopen("/proc/loadavg", "r");

    /* Put in various data. */
    fscanf(stream, "%e %e %e", &loadavg_1, &loadavg_5, &loadavg_15);

    printf("%e.%e.%e", loadavg_1, loadavg_5, loadavg_15);

    return 0;
}
