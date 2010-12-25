#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	__pid = pid;
    __interval = 1;
    __proc_path = DEFAULT_PROC_PATH;
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
	    sleep(pm->interval());
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
    char filename[32]; //todo
    snprintf(filename, 32, "%s/%d/stat", __proc_path, __pid);
    
    __last_stat = __stat;

    FILE* stream = fopen(filename, "r");
    parse_from(stream, &__stat);
    
    fclose(stream);
    
    snprintf(filename, 32, "%s/stat", __proc_path);
    stream = fopen(filename, "r");
    
    __last_system_stat = __system_stat;
    parse_stat_data(stream, &__system_stat);    
    
    fclose(stream);
}

void ProcessMonitor::parse_proc_stat(int pid, proc_stat_data_t* stat)
{
    char filename[32]; //todo
    snprintf(filename, 32, "%s/%d/stat", __proc_path, pid);

    FILE* stream = fopen(filename, "r");
    parse_from(stream, stat);
    fclose(stream);
}


void ProcessMonitor::parse_thread_stat(int pid, int tid, thread_stat_data_t* stat)
{
    char filename[64]; //todo
    snprintf(filename, 64, "%s/%d/task/%d/stat", __proc_path, pid, tid);

    FILE* stream = fopen(filename, "r");
    parse_from(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_stat(stat_data_t* stat)
{
    char filename[32]; //todo
    snprintf(filename, 32, "%s/stat", __proc_path);

    FILE* stream = fopen(filename, "r");
    parse_stat_data(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_from(FILE* stream, proc_stat_data_t* stat)
{
    fscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &stat->pid, stat->comm, &stat->state, &stat->ppid, &stat->pgrp, &stat->session, &stat->tty_nr,
        &stat->tpgid, &stat->flags, &stat->minflt, &stat->cminflt, &stat->majflt, &stat->cmajflt, &stat->utime, &stat->stime,
        &stat->cutime, &stat->cstime, &stat->priority, &stat->nice, &stat->num_threads, &stat->itrealvalue,
        &stat->starttime, &stat->vsize, &stat->rss, &stat->rsslim, &stat->startcode, &stat->endcode, &stat->startstack,
        &stat->kstkesp, &stat->kstkeip, &stat->signal, &stat->blocked, &stat->sigignore, &stat->sigcatch, &stat->wchan,
        &stat->nswap, &stat->cnswap, &stat->exit_signal, &stat->processor, &stat->rt_priority, &stat->policy,
        &stat->delayacct_blkio_ticks, &stat->guest_time, &stat->cguest_time);
}

void ProcessMonitor::parse_stat_data(FILE* stream, stat_data_t* stat_data)
{
    fscanf(stream, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        &stat_data->utime, &stat_data->nice, &stat_data->stime, &stat_data->idle, &stat_data->iowait,
        &stat_data->irq, &stat_data->softirq, &stat_data->steal, &stat_data->guest);
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

int ProcessMonitor::threads(int pid)
{
    int* pids;
    int tcnt = thread_ids(pid, &pids);
    free(pids);
    return tcnt;
}
    
int ProcessMonitor::thread_ids(int pid, int** ptids)
{
    struct dirent **folders;
    
    char task_path[64]; //todo
    snprintf(task_path, 64, "%s/%d/task", __proc_path, pid);
    
    int tcnt = scandir(task_path, &folders, NULL, NULL) - 2;

    *ptids = (int*) malloc(tcnt * sizeof(int));
        
    for (int i = 0; i < tcnt; ++i)
    {
        (*ptids)[i] = atoi(folders[i+2]->d_name);
    }

    for (int i = 0; i < tcnt+2; ++i)
    {
        free(folders[i]);
    }
            
    free(folders);
    
    return tcnt;
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

char* ProcessMonitor::proc_path()
{
    return __proc_path;
}

void ProcessMonitor::proc_path(char* proc_path)
{
    __proc_path = proc_path; // todo strcpy
}