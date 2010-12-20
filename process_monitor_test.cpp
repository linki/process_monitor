#include <gtest/gtest.h>

TEST(PROCESS_MONITOR, PROC_STAT)
{
    char* stream = "1580 (codeblocks) S 1 1221 1221 0 -1 4202496 259035 642512 40 25 50570 12453 1643 1088 20 0 7 0 13232 489279488 17877 18446744073709551615 4194304 4855908 140734284851088 140734284850208 139774647153155 0 0 4096 1224 18446744073709551615 0 0 17 0 0 0 344 0 0\n";

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

    sscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld", &pid, &comm, &state, &ppid, &pgrp, &session, &tty_nr,
                                              &tpgid, &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
                                              &cutime, &cstime, &priority, &nice, &num_threads, &itrealvalue,
                                              &starttime, &vsize, &rss, &rsslim, &startcode, &endcode, &startstack,
                                              &kstkesp, &kstkeip, &signal, &blocked, &sigignore, &sigcatch, &wchan,
                                              &nswap, &cnswap, &exit_signal, &processor, &rt_priority, &policy,
                                              &delayacct_blkio_ticks, &guest_time, &cguest_time );

    EXPECT_EQ(1580, pid);
    EXPECT_STREQ("(codeblocks)", comm);
    EXPECT_EQ('S', state);
    EXPECT_EQ(1, ppid);
    EXPECT_EQ(1221, pgrp);
    EXPECT_EQ(1221, session);
    EXPECT_EQ(0, tty_nr);
    EXPECT_EQ(-1, tpgid);
    EXPECT_EQ(4202496, flags);
    EXPECT_EQ(259035, minflt);
    EXPECT_EQ(642512, cminflt);
    EXPECT_EQ(40, majflt);
    EXPECT_EQ(25, cmajflt);
    EXPECT_EQ(50570, utime);
    EXPECT_EQ(12453, stime);
    EXPECT_EQ(1643, cutime);
    EXPECT_EQ(1088, cstime);
    EXPECT_EQ(20, priority);
    EXPECT_EQ(0, nice);
    EXPECT_EQ(7, num_threads);
    EXPECT_EQ(0, itrealvalue);
    EXPECT_EQ(13232, starttime);
    EXPECT_EQ(489279488, vsize);
    EXPECT_EQ(17877, rss);
    EXPECT_EQ(18446744073709551615, rsslim);
    EXPECT_EQ(4194304, startcode);
    EXPECT_EQ(4855908, endcode);
    EXPECT_EQ(140734284851088, startstack);
    EXPECT_EQ(140734284850208, kstkesp);
    EXPECT_EQ(139774647153155, kstkeip);
    EXPECT_EQ(0, signal);
    EXPECT_EQ(0, blocked);
    EXPECT_EQ(4096, sigignore);
    EXPECT_EQ(1224, sigcatch);
    EXPECT_EQ(18446744073709551615, wchan);
    EXPECT_EQ(0, nswap);
    EXPECT_EQ(0, cnswap);
    EXPECT_EQ(17, exit_signal);
    EXPECT_EQ(0, processor);
    EXPECT_EQ(0, rt_priority);
    EXPECT_EQ(0, policy);
    EXPECT_EQ(344, delayacct_blkio_ticks);
    EXPECT_EQ(0, guest_time);
    EXPECT_EQ(0, cguest_time);
}


TEST(PROCESS_MONITOR, PROC_STATM)
{
    char* stream = "119626 18050 6660 162 0 43782 0\n";

    unsigned long long size; // total program size
    unsigned long long resident; // resident set size
    unsigned long long share; // shared pages (from shared mappings)
    unsigned long long text; // text (code)
    unsigned long long lib; // library (unused in Linux 2.6)
    unsigned long long data; // data + stack
    unsigned long long dt; // dirty pages (unused in Linux 2.6)

    sscanf(stream, "%llu %llu %llu %llu %llu %llu %llu", &size, &resident, &share, &text, &lib, &data, &dt);

    EXPECT_EQ(119626, size);
    EXPECT_EQ(18050, resident);
    EXPECT_EQ(6660, share);
    EXPECT_EQ(162, text);
    EXPECT_EQ(0, lib);
    EXPECT_EQ(43782, data);
    EXPECT_EQ(0, dt);
}

TEST(PROCESS_MONITOR, LOADAVG)
{
    char* stream = "1.35 1.53 1.46 1/272 3848\n";

    long double loadavg1;
    long double loadavg5;
    long double loadavg15;
    unsigned long current;
    unsigned long total;
    int last;

    sscanf(stream, "%Lg %Lg %Lg %lu/%lu %d", &loadavg1, &loadavg5, &loadavg15, &current, &total, &last);

    EXPECT_EQ(135, (unsigned long long) (loadavg1 * 100));
    EXPECT_EQ(153, (unsigned long long) (loadavg5 * 100));
    EXPECT_EQ(146, (unsigned long long) (loadavg15 * 100));
    EXPECT_EQ(1, current);
    EXPECT_EQ(272, total);
    EXPECT_EQ(3848, last);
}
