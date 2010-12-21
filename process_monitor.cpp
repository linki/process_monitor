#include <stdio.h>
#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	__pid = pid;
    __interval = 1000;
}

void ProcessMonitor::parse(const char* stream)
{
    sscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &__stat.pid, __stat.comm, &__stat.state, &__stat.ppid, &__stat.pgrp, &__stat.session, &__stat.tty_nr,
        &__stat.tpgid, &__stat.flags, &__stat.minflt, &__stat.cminflt, &__stat.majflt, &__stat.cmajflt, &__stat.utime, &__stat.stime,
        &__stat.cutime, &__stat.cstime, &__stat.priority, &__stat.nice, &__stat.num_threads, &__stat.itrealvalue,
        &__stat.starttime, &__stat.vsize, &__stat.rss, &__stat.rsslim, &__stat.startcode, &__stat.endcode, &__stat.startstack,
        &__stat.kstkesp, &__stat.kstkeip, &__stat.signal, &__stat.blocked, &__stat.sigignore, &__stat.sigcatch, &__stat.wchan,
        &__stat.nswap, &__stat.cnswap, &__stat.exit_signal, &__stat.processor, &__stat.rt_priority, &__stat.policy,
        &__stat.delayacct_blkio_ticks, &__stat.guest_time, &__stat.cguest_time);
}

int ProcessMonitor::pid()
{
    return __pid;
}

unsigned ProcessMonitor::interval()
{
    return __interval;
}