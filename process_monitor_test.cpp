#include "process_monitor.h"
#include <gtest/gtest.h>

TEST(PROCESS_MONITOR, INITIALIZATION)
{
    // takes pid as argument
    ProcessMonitor* pm = new ProcessMonitor(123);
    EXPECT_EQ(123, pm->pid());

    // defaults to interval of 1000 ms
    EXPECT_EQ(1, pm->interval());
}

TEST(PROCESS_MONITOR, PARSE_PROCESS_STAT)
{
    ProcessMonitor* pm = new ProcessMonitor(123);

    const char* proc_stat = "1580 (codeblocks) S 1 1221 1221 0 -1 4202496 259035 642512 40 25 50570 12453 1643 1088 20 0 7 0 13232 489279488 17877 18446744073709551615 4194304 4855908 140734284851088 140734284850208 139774647153155 0 0 4096 1224 18446744073709551615 0 0 17 0 0 0 344 0 0\n";
    pm->parse(proc_stat);

    EXPECT_EQ(1580, pm->__stat.pid);
    EXPECT_STREQ("(codeblocks)", pm->__stat.comm);
    EXPECT_EQ('S', pm->__stat.state);
    EXPECT_EQ(1, pm->__stat.ppid);
    EXPECT_EQ(1221, pm->__stat.pgrp);
    EXPECT_EQ(1221, pm->__stat.session);
    EXPECT_EQ(0, pm->__stat.tty_nr);
    EXPECT_EQ(-1, pm->__stat.tpgid);
    EXPECT_EQ(4202496, pm->__stat.flags);
    EXPECT_EQ(259035, pm->__stat.minflt);
    EXPECT_EQ(642512, pm->__stat.cminflt);
    EXPECT_EQ(40, pm->__stat.majflt);
    EXPECT_EQ(25, pm->__stat.cmajflt);
    EXPECT_EQ(50570, pm->__stat.utime);
    EXPECT_EQ(12453, pm->__stat.stime);
    EXPECT_EQ(1643, pm->__stat.cutime);
    EXPECT_EQ(1088, pm->__stat.cstime);
    EXPECT_EQ(20, pm->__stat.priority);
    EXPECT_EQ(0, pm->__stat.nice);
    EXPECT_EQ(7, pm->__stat.num_threads);
    EXPECT_EQ(0, pm->__stat.itrealvalue);
    EXPECT_EQ(13232, pm->__stat.starttime);
    EXPECT_EQ(489279488, pm->__stat.vsize);
    EXPECT_EQ(17877, pm->__stat.rss);
    EXPECT_EQ(18446744073709551615ul, pm->__stat.rsslim);
    EXPECT_EQ(4194304, pm->__stat.startcode);
    EXPECT_EQ(4855908, pm->__stat.endcode);
    EXPECT_EQ(140734284851088, pm->__stat.startstack);
    EXPECT_EQ(140734284850208, pm->__stat.kstkesp);
    EXPECT_EQ(139774647153155, pm->__stat.kstkeip);
    EXPECT_EQ(0, pm->__stat.signal);
    EXPECT_EQ(0, pm->__stat.blocked);
    EXPECT_EQ(4096, pm->__stat.sigignore);
    EXPECT_EQ(1224, pm->__stat.sigcatch);
    EXPECT_EQ(18446744073709551615ul, pm->__stat.wchan);
    EXPECT_EQ(0, pm->__stat.nswap);
    EXPECT_EQ(0, pm->__stat.cnswap);
    EXPECT_EQ(17, pm->__stat.exit_signal);
    EXPECT_EQ(0, pm->__stat.processor);
    EXPECT_EQ(0, pm->__stat.rt_priority);
    EXPECT_EQ(0, pm->__stat.policy);
    EXPECT_EQ(344, pm->__stat.delayacct_blkio_ticks);
    EXPECT_EQ(0, pm->__stat.guest_time);
    EXPECT_EQ(0, pm->__stat.cguest_time);
}

TEST(PROCESS_MONITOR, PARSE_PROCESS_STAT_FROM_FILE)
{
    ProcessMonitor* pm = new ProcessMonitor(123);

    proc_stat_data_t stat_data;

    FILE* stream = fopen("proc_stat", "r");
    pm->parse_from(stream, &stat_data);
    fclose(stream);

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

TEST(PROCESS_MONITOR, PARSE_PROCESS_THREAD_STAT)
{
    ProcessMonitor* pm = new ProcessMonitor(123);

    const char* proc_stat = "1580 (codeblocks) S 1 1221 1221 0 -1 4202496 259035 642512 40 25 50570 12453 1643 1088 20 0 7 0 13232 489279488 17877 18446744073709551615 4194304 4855908 140734284851088 140734284850208 139774647153155 0 0 4096 1224 18446744073709551615 0 0 17 0 0 0 344 0 0\n";
    pm->parse(proc_stat);

    const char* thread_stat = "4344 (codeblocks) S 1 1221 1221 0 -1 4202560 52 24868 0 0 0 0 3 6 20 0 6 0 2147156 359444480 11028 18446744073709551615 4194304 4855908 140736585266400 140701987634104 140702160767644 0 0 4096 1224 18446744071579437197 0 0 -1 0 0 0 0 0 0\n";
    pm->parse(thread_stat);

    EXPECT_EQ(4344, pm->__stat.pid);
    EXPECT_STREQ("(codeblocks)", pm->__stat.comm);
    EXPECT_EQ('S', pm->__stat.state);
    EXPECT_EQ(1, pm->__stat.ppid);
    EXPECT_EQ(1221, pm->__stat.pgrp);
    EXPECT_EQ(1221, pm->__stat.session);
    EXPECT_EQ(0, pm->__stat.tty_nr);
    EXPECT_EQ(-1, pm->__stat.tpgid);
    EXPECT_EQ(4202560, pm->__stat.flags);
    EXPECT_EQ(52, pm->__stat.minflt);
    EXPECT_EQ(24868, pm->__stat.cminflt);
    EXPECT_EQ(0, pm->__stat.majflt);
    EXPECT_EQ(0, pm->__stat.cmajflt);
    EXPECT_EQ(0, pm->__stat.utime);
    EXPECT_EQ(0, pm->__stat.stime);
    EXPECT_EQ(3, pm->__stat.cutime);
    EXPECT_EQ(6, pm->__stat.cstime);
    EXPECT_EQ(20, pm->__stat.priority);
    EXPECT_EQ(0, pm->__stat.nice);
    EXPECT_EQ(6, pm->__stat.num_threads);
    EXPECT_EQ(0, pm->__stat.itrealvalue);
    EXPECT_EQ(2147156, pm->__stat.starttime);
    EXPECT_EQ(359444480, pm->__stat.vsize);
    EXPECT_EQ(11028, pm->__stat.rss);
    EXPECT_EQ(18446744073709551615ul, pm->__stat.rsslim);
    EXPECT_EQ(4194304, pm->__stat.startcode);
    EXPECT_EQ(4855908, pm->__stat.endcode);
    EXPECT_EQ(140736585266400, pm->__stat.startstack);
    EXPECT_EQ(140701987634104, pm->__stat.kstkesp);
    EXPECT_EQ(140702160767644, pm->__stat.kstkeip);
    EXPECT_EQ(0, pm->__stat.signal);
    EXPECT_EQ(0, pm->__stat.blocked);
    EXPECT_EQ(4096, pm->__stat.sigignore);
    EXPECT_EQ(1224, pm->__stat.sigcatch);
    EXPECT_EQ(18446744071579437197ul, pm->__stat.wchan);
    EXPECT_EQ(0, pm->__stat.nswap);
    EXPECT_EQ(0, pm->__stat.cnswap);
    EXPECT_EQ(-1, pm->__stat.exit_signal);
    EXPECT_EQ(0, pm->__stat.processor);
    EXPECT_EQ(0, pm->__stat.rt_priority);
    EXPECT_EQ(0, pm->__stat.policy);
    EXPECT_EQ(0, pm->__stat.delayacct_blkio_ticks);
    EXPECT_EQ(0, pm->__stat.guest_time);
    EXPECT_EQ(0, pm->__stat.cguest_time);
}

TEST(PROCESS_MONITOR, PROC_STATM)
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

TEST(PROCESS_MONITOR, LOADAVG)
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

TEST(PROCESS_MONITOR, STAT)
{
    ProcessMonitor* pm = new ProcessMonitor(123);
    
    stat_data_t stat_data;
    
    FILE* stream = fopen("stat", "r");
    pm->parse_stat_data(stream, &stat_data);
    fclose(stream);

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
