#include <gtest/gtest.h>

#include "process_monitor.h"

#define EXPECT_SYSTEM_CPUS(container) \
    EXPECT_EQ(4611536, container.total); \
    EXPECT_EQ(2904, container.utime);\
    EXPECT_EQ(0, container.nice);\
    EXPECT_EQ(2375, container.stime);\
    EXPECT_EQ(4606257, container.idle);\
    EXPECT_EQ(5670, container.iowait);\
    EXPECT_EQ(3, container.irq);\
    EXPECT_EQ(23, container.softirq);\
    EXPECT_EQ(0, container.steal);\
    EXPECT_EQ(0, container.guest);

#define EXPECT_SYSTEM_CPU_0(container) \
    EXPECT_EQ(382549, container.total);\
    EXPECT_EQ(435, container.utime);\
    EXPECT_EQ(0, container.nice);\
    EXPECT_EQ(253, container.stime);\
    EXPECT_EQ(381861, container.idle);\
    EXPECT_EQ(2228, container.iowait);\
    EXPECT_EQ(2, container.irq);\
    EXPECT_EQ(4, container.softirq);\
    EXPECT_EQ(0, container.steal);\
    EXPECT_EQ(0, container.guest);

#define EXPECT_SYSTEM_CPU_5(container) \
    EXPECT_EQ(384465, container.total); \
    EXPECT_EQ(100, container.utime); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(72, container.stime); \
    EXPECT_EQ(384293, container.idle); \
    EXPECT_EQ(127, container.iowait); \
    EXPECT_EQ(0, container.irq); \
    EXPECT_EQ(0, container.softirq); \
    EXPECT_EQ(0, container.steal); \
    EXPECT_EQ(0, container.guest);
    
#define EXPECT_SYSTEM_CPU_11(container) \
    EXPECT_EQ(384606, container.total); \
    EXPECT_EQ(25, container.utime); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(131, container.stime); \
    EXPECT_EQ(384450, container.idle); \
    EXPECT_EQ(102, container.iowait); \
    EXPECT_EQ(0, container.irq); \
    EXPECT_EQ(0, container.softirq); \
    EXPECT_EQ(0, container.steal); \
    EXPECT_EQ(0, container.guest);

#define EXPECT_PROCESS_STAT_42(container) \
    EXPECT_EQ(7879, container.total);\
    EXPECT_EQ(4197, container.pid);\
    EXPECT_STREQ("(codeblocks)", container.comm);\
    EXPECT_EQ('S', container.state);\
    EXPECT_EQ(1, container.ppid);\
    EXPECT_EQ(1221, container.pgrp);\
    EXPECT_EQ(1221, container.session);\
    EXPECT_EQ(0, container.tty_nr);\
    EXPECT_EQ(-1, container.tpgid);\
    EXPECT_EQ(4202496, container.flags);\
    EXPECT_EQ(32421, container.minflt);\
    EXPECT_EQ(25985, container.cminflt);\
    EXPECT_EQ(0, container.majflt);\
    EXPECT_EQ(0, container.cmajflt);\
    EXPECT_EQ(5959, container.utime);\
    EXPECT_EQ(1920, container.stime);\
    EXPECT_EQ(8, container.cutime);\
    EXPECT_EQ(7, container.cstime);\
    EXPECT_EQ(20, container.priority);\
    EXPECT_EQ(0, container.nice);\
    EXPECT_EQ(7, container.num_threads);\
    EXPECT_EQ(0, container.itrealvalue);\
    EXPECT_EQ(2146751, container.starttime);\
    EXPECT_EQ(476565504, container.vsize);\
    EXPECT_EQ(16921, container.rss);\
    EXPECT_EQ(18446744073709551615ul, container.rsslim);\
    EXPECT_EQ(4194304, container.startcode);\
    EXPECT_EQ(4855908, container.endcode);\
    EXPECT_EQ(140736585266400, container.startstack);\
    EXPECT_EQ(140736585265520, container.kstkesp);\
    EXPECT_EQ(140702147826179, container.kstkeip);\
    EXPECT_EQ(0, container.signal);\
    EXPECT_EQ(0, container.blocked);\
    EXPECT_EQ(4096, container.sigignore);\
    EXPECT_EQ(1224, container.sigcatch);\
    EXPECT_EQ(18446744073709551615ul, container.wchan);\
    EXPECT_EQ(0, container.nswap);\
    EXPECT_EQ(0, container.cnswap);\
    EXPECT_EQ(17, container.exit_signal);\
    EXPECT_EQ(0, container.processor);\
    EXPECT_EQ(0, container.rt_priority);\
    EXPECT_EQ(0, container.policy);\
    EXPECT_EQ(4, container.delayacct_blkio_ticks);\
    EXPECT_EQ(0, container.guest_time);\
    EXPECT_EQ(0, container.cguest_time);

#define EXPECT_THREAD_STAT_1729(container) \
    EXPECT_EQ(700, container.total); \
    EXPECT_EQ(1729, container.pid); \
    EXPECT_STREQ("(parsum)", container.comm); \
    EXPECT_EQ('S', container.state); \
    EXPECT_EQ(1629, container.ppid); \
    EXPECT_EQ(1729, container.pgrp); \
    EXPECT_EQ(1629, container.session); \
    EXPECT_EQ(34817, container.tty_nr); \
    EXPECT_EQ(1629, container.tpgid); \
    EXPECT_EQ(4202496, container.flags); \
    EXPECT_EQ(268, container.minflt); \
    EXPECT_EQ(0, container.cminflt); \
    EXPECT_EQ(0, container.majflt); \
    EXPECT_EQ(0, container.cmajflt); \
    EXPECT_EQ(400, container.utime); \
    EXPECT_EQ(300, container.stime); \
    EXPECT_EQ(0, container.cutime); \
    EXPECT_EQ(0, container.cstime); \
    EXPECT_EQ(20, container.priority); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(4, container.num_threads); \
    EXPECT_EQ(0, container.itrealvalue); \
    EXPECT_EQ(34355, container.starttime); \
    EXPECT_EQ(31535104, container.vsize); \
    EXPECT_EQ(86, container.rss); \
    EXPECT_EQ(18446744073709551615ul, container.rsslim); \
    EXPECT_EQ(4194304, container.startcode); \
    EXPECT_EQ(4198948, container.endcode); \
    EXPECT_EQ(140735433694176, container.startstack); \
    EXPECT_EQ(140735433693520, container.kstkesp); \
    EXPECT_EQ(139673659507333, container.kstkeip); \
    EXPECT_EQ(0, container.signal); \
    EXPECT_EQ(0, container.blocked); \
    EXPECT_EQ(0, container.sigignore); \
    EXPECT_EQ(0, container.sigcatch); \
    EXPECT_EQ(18446744071579437533ul, container.wchan); \
    EXPECT_EQ(0, container.nswap); \
    EXPECT_EQ(0, container.cnswap); \
    EXPECT_EQ(17, container.exit_signal); \
    EXPECT_EQ(1, container.processor); \
    EXPECT_EQ(0, container.rt_priority); \
    EXPECT_EQ(0, container.policy); \
    EXPECT_EQ(0, container.delayacct_blkio_ticks); \
    EXPECT_EQ(0, container.guest_time); \
    EXPECT_EQ(0, container.cguest_time);

#define EXPECT_THREAD_STAT_1730(container) \
    EXPECT_EQ(4186, container.total); \
    EXPECT_EQ(1730, container.pid); \
    EXPECT_STREQ("(parsum)", container.comm); \
    EXPECT_EQ('R', container.state); \
    EXPECT_EQ(1629, container.ppid); \
    EXPECT_EQ(1729, container.pgrp); \
    EXPECT_EQ(1629, container.session); \
    EXPECT_EQ(34817, container.tty_nr); \
    EXPECT_EQ(1629, container.tpgid); \
    EXPECT_EQ(4202560, container.flags); \
    EXPECT_EQ(0, container.minflt); \
    EXPECT_EQ(0, container.cminflt); \
    EXPECT_EQ(0, container.majflt); \
    EXPECT_EQ(0, container.cmajflt); \
    EXPECT_EQ(4133, container.utime); \
    EXPECT_EQ(53, container.stime); \
    EXPECT_EQ(0, container.cutime); \
    EXPECT_EQ(0, container.cstime); \
    EXPECT_EQ(20, container.priority); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(4, container.num_threads); \
    EXPECT_EQ(0, container.itrealvalue); \
    EXPECT_EQ(34360, container.starttime); \
    EXPECT_EQ(31535104, container.vsize); \
    EXPECT_EQ(86, container.rss); \
    EXPECT_EQ(18446744073709551615ul, container.rsslim); \
    EXPECT_EQ(4194304, container.startcode); \
    EXPECT_EQ(4198948, container.endcode); \
    EXPECT_EQ(140735433694176, container.startstack); \
    EXPECT_EQ(139673655787200, container.kstkesp); \
    EXPECT_EQ(4196512, container.kstkeip); \
    EXPECT_EQ(0, container.signal); \
    EXPECT_EQ(0, container.blocked); \
    EXPECT_EQ(0, container.sigignore); \
    EXPECT_EQ(0, container.sigcatch); \
    EXPECT_EQ(0, container.wchan); \
    EXPECT_EQ(0, container.nswap); \
    EXPECT_EQ(0, container.cnswap); \
    EXPECT_EQ(-1, container.exit_signal); \
    EXPECT_EQ(1, container.processor); \
    EXPECT_EQ(0, container.rt_priority); \
    EXPECT_EQ(0, container.policy); \
    EXPECT_EQ(0, container.delayacct_blkio_ticks); \
    EXPECT_EQ(0, container.guest_time); \
    EXPECT_EQ(0, container.cguest_time);

#define EXPECT_THREAD_STAT_1731(container) \
    EXPECT_EQ(3166, container.total); \
    EXPECT_EQ(1731, container.pid); \
    EXPECT_STREQ("(parsum)", container.comm); \
    EXPECT_EQ('R', container.state); \
    EXPECT_EQ(1629, container.ppid); \
    EXPECT_EQ(1729, container.pgrp); \
    EXPECT_EQ(1629, container.session); \
    EXPECT_EQ(34817, container.tty_nr); \
    EXPECT_EQ(1629, container.tpgid); \
    EXPECT_EQ(4202560, container.flags); \
    EXPECT_EQ(0, container.minflt); \
    EXPECT_EQ(0, container.cminflt); \
    EXPECT_EQ(0, container.majflt); \
    EXPECT_EQ(0, container.cmajflt); \
    EXPECT_EQ(3157, container.utime); \
    EXPECT_EQ(9, container.stime); \
    EXPECT_EQ(0, container.cutime); \
    EXPECT_EQ(0, container.cstime); \
    EXPECT_EQ(20, container.priority); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(4, container.num_threads); \
    EXPECT_EQ(0, container.itrealvalue); \
    EXPECT_EQ(34360, container.starttime); \
    EXPECT_EQ(31535104, container.vsize); \
    EXPECT_EQ(86, container.rss); \
    EXPECT_EQ(18446744073709551615ul, container.rsslim); \
    EXPECT_EQ(4194304, container.startcode); \
    EXPECT_EQ(4198948, container.endcode); \
    EXPECT_EQ(140735433694176, container.startstack); \
    EXPECT_EQ(139673647394496, container.kstkesp); \
    EXPECT_EQ(4196530, container.kstkeip); \
    EXPECT_EQ(0, container.signal); \
    EXPECT_EQ(0, container.blocked); \
    EXPECT_EQ(0, container.sigignore); \
    EXPECT_EQ(0, container.sigcatch); \
    EXPECT_EQ(0, container.wchan); \
    EXPECT_EQ(0, container.nswap); \
    EXPECT_EQ(0, container.cnswap); \
    EXPECT_EQ(-1, container.exit_signal); \
    EXPECT_EQ(0, container.processor); \
    EXPECT_EQ(0, container.rt_priority); \
    EXPECT_EQ(0, container.policy); \
    EXPECT_EQ(0, container.delayacct_blkio_ticks); \
    EXPECT_EQ(0, container.guest_time); \
    EXPECT_EQ(0, container.cguest_time);

#define EXPECT_THREAD_STAT_1732(container) \
    EXPECT_EQ(3435, container.total); \
    EXPECT_EQ(1732, container.pid); \
    EXPECT_STREQ("(parsum)", container.comm); \
    EXPECT_EQ('R', container.state); \
    EXPECT_EQ(1629, container.ppid); \
    EXPECT_EQ(1729, container.pgrp); \
    EXPECT_EQ(1629, container.session); \
    EXPECT_EQ(34817, container.tty_nr); \
    EXPECT_EQ(1629, container.tpgid); \
    EXPECT_EQ(4202560, container.flags); \
    EXPECT_EQ(0, container.minflt); \
    EXPECT_EQ(0, container.cminflt); \
    EXPECT_EQ(0, container.majflt); \
    EXPECT_EQ(0, container.cmajflt); \
    EXPECT_EQ(3403, container.utime); \
    EXPECT_EQ(32, container.stime); \
    EXPECT_EQ(0, container.cutime); \
    EXPECT_EQ(0, container.cstime); \
    EXPECT_EQ(20, container.priority); \
    EXPECT_EQ(0, container.nice); \
    EXPECT_EQ(4, container.num_threads); \
    EXPECT_EQ(0, container.itrealvalue); \
    EXPECT_EQ(34360, container.starttime); \
    EXPECT_EQ(31535104, container.vsize); \
    EXPECT_EQ(86, container.rss); \
    EXPECT_EQ(18446744073709551615ul, container.rsslim); \
    EXPECT_EQ(4194304, container.startcode); \
    EXPECT_EQ(4198948, container.endcode); \
    EXPECT_EQ(140735433694176, container.startstack); \
    EXPECT_EQ(139673639001792, container.kstkesp); \
    EXPECT_EQ(4196521, container.kstkeip); \
    EXPECT_EQ(0, container.signal); \
    EXPECT_EQ(0, container.blocked); \
    EXPECT_EQ(0, container.sigignore); \
    EXPECT_EQ(0, container.sigcatch); \
    EXPECT_EQ(0, container.wchan); \
    EXPECT_EQ(0, container.nswap); \
    EXPECT_EQ(0, container.cnswap); \
    EXPECT_EQ(-1, container.exit_signal); \
    EXPECT_EQ(0, container.processor); \
    EXPECT_EQ(0, container.rt_priority); \
    EXPECT_EQ(0, container.policy); \
    EXPECT_EQ(0, container.delayacct_blkio_ticks); \
    EXPECT_EQ(0, container.guest_time); \
    EXPECT_EQ(0, container.cguest_time);
    
#define EXPECT_PROCESS_STATM_42(container) \
    EXPECT_EQ(3543, container.size); \
    EXPECT_EQ(125, container.resident); \
    EXPECT_EQ(99, container.share); \
    EXPECT_EQ(2, container.text); \
    EXPECT_EQ(0, container.lib); \
    EXPECT_EQ(2117, container.data); \
    EXPECT_EQ(0, container.dt);

#define EXPECT_PROCESS_STATUS_42(container) \
    EXPECT_EQ(1592900, container.vm_size); \
    EXPECT_EQ(1111020, container.vm_rss);

TEST(ProcessMonitor, Initialization)
{
    ProcessMonitor pm(42);

    EXPECT_EQ(42, pm.pid());
    EXPECT_EQ(2, pm.interval());
    EXPECT_STREQ("/proc", pm.procfs_path());
}

TEST(ProcessMonitor, ChangeProcPath)
{
    ProcessMonitor pm(42, "/some/other/procfs/path");
    EXPECT_STREQ("/some/other/procfs/path", pm.procfs_path());
}

TEST(ProcessMonitor, FetchUpdatesTheData)
{
    ProcessMonitor pm(42, "test/proc");

    pm.fetch();

    EXPECT_PROCESS_STAT_42(pm._process_data);
    
    EXPECT_THREAD_STAT_1729(pm._process_data._thread_data[0]);
    EXPECT_THREAD_STAT_1730(pm._process_data._thread_data[1]);
    EXPECT_THREAD_STAT_1731(pm._process_data._thread_data[2]);
    EXPECT_THREAD_STAT_1732(pm._process_data._thread_data[3]);
    
    EXPECT_PROCESS_STATUS_42(pm._process_data._memory_data);
}

TEST(ProcessMonitor, ComputeCorrectProcfsPaths)
{
    ProcessMonitor pm(42, "/proc");
    
    char* system_stat_path;
    char* process_stat_path;
    char* thread_stat_path;
    
    pm.get_path("stat", &system_stat_path);
    pm.get_path(42, "stat", &process_stat_path);
    pm.get_path(42, 1732, "stat", &thread_stat_path);
    
    EXPECT_STREQ("/proc/stat", system_stat_path);
    EXPECT_STREQ("/proc/42/stat", process_stat_path);
    EXPECT_STREQ("/proc/42/task/1732/stat", thread_stat_path);
    
    free(system_stat_path);
    free(process_stat_path);
    free(thread_stat_path);
}

TEST(ProcessMonitor, GetThreadIdsForPid)
{
    ProcessMonitor pm(42, "test/proc");

    int* tids;
    int tcnt = pm.parse_process_thread_ids(42, &tids);
    
    EXPECT_EQ(4, tcnt);
    
    EXPECT_EQ(1729, tids[0]);
    EXPECT_EQ(1730, tids[1]);
    EXPECT_EQ(1731, tids[2]);
    EXPECT_EQ(1732, tids[3]);
    
    free(tids);
}

TEST(ProcessMonitor, GetAttributes)
{
    ProcessMonitor pm(42, "test/proc");
    
    pm.fetch();
    
    EXPECT_EQ(12, pm.cpu_count());
    
    EXPECT_EQ(2904, pm.cpus());
    EXPECT_EQ(435, pm.cpu(0));
    EXPECT_EQ(100, pm.cpu(5));
    EXPECT_EQ(25, pm.cpu(11));
    
    EXPECT_EQ(4, pm.threads());
    
    EXPECT_EQ(5959, pm.utime());
    
    EXPECT_EQ(400, pm.utime(0));
    EXPECT_EQ(4133, pm.utime(1));
    EXPECT_EQ(3157, pm.utime(2));
    EXPECT_EQ(3403, pm.utime(3));
    
    EXPECT_EQ('S', pm.state());
    
    EXPECT_EQ(49550504, pm.mem_total());
    EXPECT_EQ(47442424, pm.mem_free());
}

TEST(ProcessMonitor, ParseProcessStat)
{
    ProcessMonitor pm(42, "test/proc");

    process_data_t stat_data;
    pm.parse_process_stat_file(42, &stat_data);
    
    EXPECT_PROCESS_STAT_42(stat_data);
}

TEST(ProcessMonitor, ParseProcessThreadStat)
{
    ProcessMonitor pm(42, "test/proc");

    thread_data_t stat_data;
    pm.parse_thread_stat_file(42, 1732, &stat_data);
    
    EXPECT_THREAD_STAT_1732(stat_data);
}

// // measured in pages
// TEST(ProcessMonitor, ParseProcessStatm)
// {
//     ProcessMonitor* pm = new ProcessMonitor(42);
//     pm.procfs_path("test/proc");
// 
//     process_datam_t data;
//     pm.parse_process_statm_file(42, &data);
//         
//     EXPECT_PROCESS_STATM_42(data);
// }

TEST(ProcessMonitor, ParseProcessStatus)
{
    ProcessMonitor pm(42, "test/proc");

    memory_data_t data;
    pm.parse_process_status_file(42, &data);
        
    EXPECT_PROCESS_STATUS_42(data);
}

TEST(ProcessMonitor, ParseSystemMeminfo)
{
    ProcessMonitor pm(42, "test/proc");

    meminfo_t data;
    pm.parse_system_meminfo_file(&data);
          
    EXPECT_EQ(49550504, data.total); // kb
    EXPECT_EQ(47442424, data.free); //kb
    EXPECT_EQ( 2108080, data.used); //kb
    


    
    // mem_usage is missing

    // VmRSS / MemTotal = Memory Usage laut top
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

TEST(ProcessMonitor, ComputeSystemCPUUsage)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    pm._system_data.cpus.total = 100;
    pm._system_data.cpus.idle = 100;
    
    pm.fetch();

    pm._system_data.cpus.total = 240;
    pm._system_data.cpus.idle = 180;
    
    EXPECT_EQ(42.86, pm.cpu_usage());
}

TEST(ProcessMonitor, ComputeSystemCPUxUsage)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    // todo allocate
    pm._system_data.cpu[0].total = 100;
    pm._system_data.cpu[0].idle = 100;

    pm._system_data.cpu[1].total = 300;
    pm._system_data.cpu[1].idle = 300;
    
    pm.fetch();

    pm._system_data.cpu[0].total = 320;
    pm._system_data.cpu[0].idle = 180;

    pm._system_data.cpu[1].total = 770;
    pm._system_data.cpu[1].idle = 400;
    
    EXPECT_EQ(63.64, pm.cpu_usage(0));
    EXPECT_EQ(78.73, pm.cpu_usage(1));    
}

TEST(ProcessMonitor, ComputeCorrectProcessCPUUsage)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    pm._system_data.cpus.total = 100;
    pm._process_data.total = 100;
    
    pm.fetch();

    pm._system_data.cpus.total = 200;
    pm._process_data.total = 180;
    
    EXPECT_EQ(80, pm.process_cpu_usage());
}

TEST(ProcessMonitor, ComputeCorrectGlobalThreadCPUUsage)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    // todo allocate
    pm._system_data.cpus.total = 100;
    pm._process_data._thread_data[0].total = 100;
    
    pm.fetch();

    pm._system_data.cpus.total = 200;
    pm._process_data._thread_data[0].total = 120;
    
    EXPECT_EQ(20, pm.global_thread_cpu_usage(0));
}

TEST(ProcessMonitor, ComputeCorrectLocalThreadCPUUsage)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    // todo allocate
    pm._system_data.cpus.total = 100;
    pm._process_data.total = 100;
    pm._process_data._thread_data[0].total = 100;
    
    pm.fetch();

    pm._system_data.cpus.total = 200;
    pm._process_data.total = 150;
    pm._process_data._thread_data[0].total = 125;
    
    // todo method name
    EXPECT_EQ(25, pm.global_thread_cpu_usage(0));    
    EXPECT_EQ(50, pm.thread_cpu_usage(0));
}

TEST(ProcessMonitor, ComputeCorrectMemoryUsage)
{
    ProcessMonitor pm(42, "test/proc");
    // todo allocate
    // size of process divided by total for now
    pm._system_data._memory_data.total = 200;
    pm._process_data._memory_data.vm_rss = 50;    
    
    EXPECT_EQ(25, pm.process_mem_usage());

    pm.fetch();
    
    EXPECT_EQ(2, pm.process_mem_usage());
    
//    EXPECT_EQ( 4.25, pm.mem_usage()); //kb    
    
  //  VmRSS:	 1111020 kB
    
//    1111020/49550504
}


TEST(ProcessMonitor, ParseSystemData)
{
    ProcessMonitor pm(42, "test/proc");

    // allocate before
    system_data_t system_data;
    
    ProcessMonitor::_initialize_system_data(&system_data);

    pm.parse_system_stat_file(&system_data);

    EXPECT_SYSTEM_CPUS(system_data.cpus);
    EXPECT_SYSTEM_CPU_0(system_data.cpu[0]);
    EXPECT_SYSTEM_CPU_5(system_data.cpu[5]);
    EXPECT_SYSTEM_CPU_11(system_data.cpu[11]);
}





TEST(ProcessMonitor, ParseProcessDataWithThreadsAndMemory)
{
    ProcessMonitor pm(42, "test/proc");

    process_data_t stat_data;
    
    ProcessMonitor::_initialize_process_data(&stat_data);

    pm.parse_process_data(42, &stat_data);
    
    EXPECT_PROCESS_STAT_42(stat_data);
    
    EXPECT_THREAD_STAT_1729(stat_data._thread_data[0]);
    EXPECT_THREAD_STAT_1730(stat_data._thread_data[1]);
    EXPECT_THREAD_STAT_1731(stat_data._thread_data[2]);
    EXPECT_THREAD_STAT_1732(stat_data._thread_data[3]);
    
    EXPECT_PROCESS_STATUS_42(stat_data._memory_data);
}

TEST(ProcessMonitor, ParseProcessThreads)
{
    ProcessMonitor pm(42, "test/proc");

    thread_data_t* thread_data;
    int thread_count;
    
    ProcessMonitor::_initialize_thread_data(&thread_data, &thread_count);

    pm.parse_process_threads(42, &thread_data, &thread_count);
    
    EXPECT_THREAD_STAT_1729(thread_data[0]);
    EXPECT_THREAD_STAT_1730(thread_data[1]);
    EXPECT_THREAD_STAT_1731(thread_data[2]);
    EXPECT_THREAD_STAT_1732(thread_data[3]);
}

TEST(ProcessMonitor, CopyProcessdata)
{
    ProcessMonitor pm(42, "test/proc");
    
    process_data_t src_data;
    process_data_t dest_data;
    
    ProcessMonitor::_initialize_process_data(&src_data);
    ProcessMonitor::_initialize_process_data(&dest_data);
    
    pm.parse_process_data(42, &src_data);
    pm.copy_process_data(&dest_data, &src_data);

    src_data.utime = 0;
    src_data._thread_count = 0;
    src_data._thread_data[0].utime = 0;
    src_data._thread_data[1].utime = 0;
    src_data._thread_data[2].utime = 0;
    src_data._thread_data[3].utime = 0;    

    EXPECT_PROCESS_STAT_42(dest_data);
    
    EXPECT_EQ(4, dest_data._thread_count);

    EXPECT_THREAD_STAT_1729(dest_data._thread_data[0]);
    EXPECT_THREAD_STAT_1730(dest_data._thread_data[1]);
    EXPECT_THREAD_STAT_1731(dest_data._thread_data[2]);
    EXPECT_THREAD_STAT_1732(dest_data._thread_data[3]);
    
    EXPECT_PROCESS_STATUS_42(dest_data._memory_data);
}

TEST(ProcessMonitor, CopySystemdata)
{
    ProcessMonitor pm(42, "test/proc");

    // allocate before
    system_data_t src_data;
    system_data_t dest_data;

    ProcessMonitor::_initialize_system_data(&src_data);
    ProcessMonitor::_initialize_system_data(&dest_data);

    pm.parse_system_stat_file(&src_data);
    pm.parse_system_meminfo_file(&src_data._memory_data);
    
    pm.copy_system_data(&dest_data, &src_data);

    src_data.cpu_count = 0;
    src_data.cpus.utime = 0;
    src_data.cpu[0].utime = 0;
    src_data.cpu[5].utime = 0;
    src_data.cpu[11].utime = 0;
    src_data._memory_data.total = 0;    

    EXPECT_EQ(12, dest_data.cpu_count);
    
    EXPECT_SYSTEM_CPUS(dest_data.cpus);
    EXPECT_SYSTEM_CPU_0(dest_data.cpu[0]);
    EXPECT_SYSTEM_CPU_5(dest_data.cpu[5]);
    EXPECT_SYSTEM_CPU_11(dest_data.cpu[11]);

    EXPECT_EQ(49550504, dest_data._memory_data.total);    
}

TEST(ProcessMonitor, RememberLastFetch)
{
    ProcessMonitor pm(42, "test/proc");
    pm.fetch();

    pm._system_data.cpus.utime = 1;
    pm._system_data.cpu[5].utime = 3;
    pm._process_data.utime = 5;
    pm.fetch();

    EXPECT_EQ(1, pm._last_system_data.cpus.utime);
    EXPECT_EQ(3, pm._last_system_data.cpu[5].utime);
    EXPECT_EQ(5, pm._last_process_data.utime);

    pm._system_data.cpus.utime = 2;
    pm._system_data.cpu[5].utime = 4;
    pm._process_data.utime = 6;    
    pm.fetch();

    EXPECT_EQ(2, pm._last_system_data.cpus.utime);
    EXPECT_EQ(4, pm._last_system_data.cpu[5].utime);
    EXPECT_EQ(6, pm._last_process_data.utime);
}

TEST(ProcessMonitor, InitSystemData)
{
    system_data_t system_data;

    ProcessMonitor::_initialize_system_data(&system_data);
    
    EXPECT_EQ(0, system_data.cpu_count);
    EXPECT_EQ(NULL, system_data.cpu);    
}

TEST(ProcessMonitor, InitProcessData)
{
    process_data_t process_data;

    ProcessMonitor::_initialize_process_data(&process_data);
    
    EXPECT_EQ(0, process_data._thread_count);
    EXPECT_EQ(NULL, process_data._thread_data);    
}

TEST(ProcessMonitor, InitThreadData)
{
    thread_data_t* thread_data;
    int thread_count;    

    ProcessMonitor::_initialize_thread_data(&thread_data, &thread_count);
    
    EXPECT_EQ(0, thread_count);
    EXPECT_EQ(NULL, thread_data);
}
