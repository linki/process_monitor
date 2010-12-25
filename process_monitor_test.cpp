#include "process_monitor.h"
#include <gtest/gtest.h>

TEST(ProcessMonitor, Initialization)
{
    ProcessMonitor* pm = new ProcessMonitor(42);

    EXPECT_EQ(42, pm->pid());
    EXPECT_EQ(1, pm->interval());
    EXPECT_STREQ("/proc", pm->proc_path());
}

TEST(ProcessMonitor, ChangeProcPath)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("/some/other/proc/path");
    EXPECT_STREQ("/some/other/proc/path", pm->proc_path());
}

TEST(ProcessMonitor, FetchUpdatesTheData)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    EXPECT_EQ(0, pm->__stat.utime);
    pm->fetch();
    EXPECT_EQ(5959, pm->__stat.utime);
}

TEST(ProcessMonitor, GetNumberOfThreads)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    EXPECT_EQ(4, pm->threads(42));
}

TEST(ProcessMonitor, GetThreadIdsForPid)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    int* tids;
    int tcnt = pm->thread_ids(42, &tids);
    
    EXPECT_EQ(4, tcnt);
    
    EXPECT_EQ(1729, tids[0]);
    EXPECT_EQ(1730, tids[1]);
    EXPECT_EQ(1731, tids[2]);
    EXPECT_EQ(1732, tids[3]);
    
    free(tids);
}

TEST(ProcessMonitor, ParseSystemStat)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    stat_data_t stat_data;
    pm->parse_stat(&stat_data);

    EXPECT_EQ(2904, stat_data.utime);
    EXPECT_EQ(0, stat_data.nice);
    EXPECT_EQ(2375, stat_data.stime);
    EXPECT_EQ(4606257, stat_data.idle);
    EXPECT_EQ(5670, stat_data.iowait);
    EXPECT_EQ(3, stat_data.irq);
    EXPECT_EQ(23, stat_data.softirq);
    EXPECT_EQ(0, stat_data.steal);
    EXPECT_EQ(0, stat_data.guest);
}

TEST(ProcessMonitor, ParseProcessStat)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    proc_stat_data_t stat_data;
    pm->parse_proc_stat(42, &stat_data);

    EXPECT_EQ(4197, stat_data.pid);
    EXPECT_STREQ("(codeblocks)", stat_data.comm);
    EXPECT_EQ('S', stat_data.state);
    EXPECT_EQ(1, stat_data.ppid);
    EXPECT_EQ(1221, stat_data.pgrp);
    EXPECT_EQ(1221, stat_data.session);
    EXPECT_EQ(0, stat_data.tty_nr);
    EXPECT_EQ(-1, stat_data.tpgid);
    EXPECT_EQ(4202496, stat_data.flags);
    EXPECT_EQ(32421, stat_data.minflt);
    EXPECT_EQ(25985, stat_data.cminflt);
    EXPECT_EQ(0, stat_data.majflt);
    EXPECT_EQ(0, stat_data.cmajflt);
    EXPECT_EQ(5959, stat_data.utime);
    EXPECT_EQ(1920, stat_data.stime);
    EXPECT_EQ(8, stat_data.cutime);
    EXPECT_EQ(7, stat_data.cstime);
    EXPECT_EQ(20, stat_data.priority);
    EXPECT_EQ(0, stat_data.nice);
    EXPECT_EQ(7, stat_data.num_threads);
    EXPECT_EQ(0, stat_data.itrealvalue);
    EXPECT_EQ(2146751, stat_data.starttime);
    EXPECT_EQ(476565504, stat_data.vsize);
    EXPECT_EQ(16921, stat_data.rss);
    EXPECT_EQ(18446744073709551615ul, stat_data.rsslim);
    EXPECT_EQ(4194304, stat_data.startcode);
    EXPECT_EQ(4855908, stat_data.endcode);
    EXPECT_EQ(140736585266400, stat_data.startstack);
    EXPECT_EQ(140736585265520, stat_data.kstkesp);
    EXPECT_EQ(140702147826179, stat_data.kstkeip);
    EXPECT_EQ(0, stat_data.signal);
    EXPECT_EQ(0, stat_data.blocked);
    EXPECT_EQ(4096, stat_data.sigignore);
    EXPECT_EQ(1224, stat_data.sigcatch);
    EXPECT_EQ(18446744073709551615ul, stat_data.wchan);
    EXPECT_EQ(0, stat_data.nswap);
    EXPECT_EQ(0, stat_data.cnswap);
    EXPECT_EQ(17, stat_data.exit_signal);
    EXPECT_EQ(0, stat_data.processor);
    EXPECT_EQ(0, stat_data.rt_priority);
    EXPECT_EQ(0, stat_data.policy);
    EXPECT_EQ(4, stat_data.delayacct_blkio_ticks);
    EXPECT_EQ(0, stat_data.guest_time);
    EXPECT_EQ(0, stat_data.cguest_time);
}

TEST(ProcessMonitor, ParseProcessThreadStat)
{
    ProcessMonitor* pm = new ProcessMonitor(42);
    pm->proc_path("test/proc");

    thread_stat_data_t stat_data;
    pm->parse_thread_stat(42, 1732, &stat_data);
               
    EXPECT_EQ(1732, stat_data.pid);
    EXPECT_STREQ("(parsum)", stat_data.comm);
    EXPECT_EQ('R', stat_data.state);
    EXPECT_EQ(1629, stat_data.ppid);
    EXPECT_EQ(1729, stat_data.pgrp);
    EXPECT_EQ(1629, stat_data.session);
    EXPECT_EQ(34817, stat_data.tty_nr);
    EXPECT_EQ(1629, stat_data.tpgid);
    EXPECT_EQ(4202560, stat_data.flags);
    EXPECT_EQ(0, stat_data.minflt);
    EXPECT_EQ(0, stat_data.cminflt);
    EXPECT_EQ(0, stat_data.majflt);
    EXPECT_EQ(0, stat_data.cmajflt);
    EXPECT_EQ(3403, stat_data.utime);
    EXPECT_EQ(32, stat_data.stime);
    EXPECT_EQ(0, stat_data.cutime);
    EXPECT_EQ(0, stat_data.cstime);
    EXPECT_EQ(20, stat_data.priority);
    EXPECT_EQ(0, stat_data.nice);
    EXPECT_EQ(4, stat_data.num_threads);
    EXPECT_EQ(0, stat_data.itrealvalue);
    EXPECT_EQ(34360, stat_data.starttime);
    EXPECT_EQ(31535104, stat_data.vsize);
    EXPECT_EQ(86, stat_data.rss);
    EXPECT_EQ(18446744073709551615ul, stat_data.rsslim);
    EXPECT_EQ(4194304, stat_data.startcode);
    EXPECT_EQ(4198948, stat_data.endcode);
    EXPECT_EQ(140735433694176, stat_data.startstack);
    EXPECT_EQ(139673639001792, stat_data.kstkesp);
    EXPECT_EQ(4196521, stat_data.kstkeip);
    EXPECT_EQ(0, stat_data.signal);
    EXPECT_EQ(0, stat_data.blocked);
    EXPECT_EQ(0, stat_data.sigignore);
    EXPECT_EQ(0, stat_data.sigcatch);
    EXPECT_EQ(0, stat_data.wchan);
    EXPECT_EQ(0, stat_data.nswap);
    EXPECT_EQ(0, stat_data.cnswap);
    EXPECT_EQ(-1, stat_data.exit_signal);
    EXPECT_EQ(0, stat_data.processor);
    EXPECT_EQ(0, stat_data.rt_priority);
    EXPECT_EQ(0, stat_data.policy);
    EXPECT_EQ(0, stat_data.delayacct_blkio_ticks);
    EXPECT_EQ(0, stat_data.guest_time);
    EXPECT_EQ(0, stat_data.cguest_time);
}

TEST(ProcessMonitor, ParseSystemStatm)
{
    const char* stream = "119626 18050 6660 162 0 43782 0\n";

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

TEST(ProcessMonitor, ParseLoadAvg)
{
    const char* stream = "1.35 1.53 1.46 1/272 3848\n";

    long double loadavg1;
    long double loadavg5;
    long double loadavg15;
    unsigned long current;
    unsigned long total;
    int last;

    sscanf(stream, "%Lf %Lf %Lf %lu/%lu %d", &loadavg1, &loadavg5, &loadavg15, &current, &total, &last);

    EXPECT_EQ(135, (unsigned long long) (loadavg1 * 100));
    EXPECT_EQ(153, (unsigned long long) (loadavg5 * 100));
    EXPECT_EQ(146, (unsigned long long) (loadavg15 * 100));
    EXPECT_EQ(1, current);
    EXPECT_EQ(272, total);
    EXPECT_EQ(3848, last);
}
