#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	__pid = pid;
    __interval = 1000;
}

void* ProcessMonitor::run(void* data)
{
	/* enable canceling of thread */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	/* enable immediate cancelation */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    ProcessMonitor* pm = (ProcessMonitor*) data;
    
    while (1)
    {
	    pm->fetch();        
	    printf("%lu\n", pm->utime());
	    sleep(1);
    }
    
	pthread_exit(NULL);
}

void ProcessMonitor::start()
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	
	
    int status = pthread_create(&__runner, &attr, run, this);
	
	if (status)
	{
		printf("ERROR: return code from pthread_create() is %d\n", status);
		exit(-1);
	}
	
	pthread_attr_destroy(&attr);
}

void ProcessMonitor::stop()
{
    pthread_cancel(__runner);
    
	int status = pthread_join(__runner, NULL);
	
	if (status)
	{
		printf("ERROR: return code from pthread_join() is %d\n", status);
		exit(-1);
	}
}

void ProcessMonitor::fetch()
{
    FILE *stream;
    stream = fopen("/proc/4197/stat", "r");
    fscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &__stat.pid, __stat.comm, &__stat.state, &__stat.ppid, &__stat.pgrp, &__stat.session, &__stat.tty_nr,
        &__stat.tpgid, &__stat.flags, &__stat.minflt, &__stat.cminflt, &__stat.majflt, &__stat.cmajflt, &__stat.utime, &__stat.stime,
        &__stat.cutime, &__stat.cstime, &__stat.priority, &__stat.nice, &__stat.num_threads, &__stat.itrealvalue,
        &__stat.starttime, &__stat.vsize, &__stat.rss, &__stat.rsslim, &__stat.startcode, &__stat.endcode, &__stat.startstack,
        &__stat.kstkesp, &__stat.kstkeip, &__stat.signal, &__stat.blocked, &__stat.sigignore, &__stat.sigcatch, &__stat.wchan,
        &__stat.nswap, &__stat.cnswap, &__stat.exit_signal, &__stat.processor, &__stat.rt_priority, &__stat.policy,
        &__stat.delayacct_blkio_ticks, &__stat.guest_time, &__stat.cguest_time);
    fclose(stream);
}

void ProcessMonitor::parse_from(FILE* stream)
{
    fscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &__stat.pid, __stat.comm, &__stat.state, &__stat.ppid, &__stat.pgrp, &__stat.session, &__stat.tty_nr,
        &__stat.tpgid, &__stat.flags, &__stat.minflt, &__stat.cminflt, &__stat.majflt, &__stat.cmajflt, &__stat.utime, &__stat.stime,
        &__stat.cutime, &__stat.cstime, &__stat.priority, &__stat.nice, &__stat.num_threads, &__stat.itrealvalue,
        &__stat.starttime, &__stat.vsize, &__stat.rss, &__stat.rsslim, &__stat.startcode, &__stat.endcode, &__stat.startstack,
        &__stat.kstkesp, &__stat.kstkeip, &__stat.signal, &__stat.blocked, &__stat.sigignore, &__stat.sigcatch, &__stat.wchan,
        &__stat.nswap, &__stat.cnswap, &__stat.exit_signal, &__stat.processor, &__stat.rt_priority, &__stat.policy,
        &__stat.delayacct_blkio_ticks, &__stat.guest_time, &__stat.cguest_time);
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

unsigned long ProcessMonitor::utime()
{
    return __stat.utime;
}
