#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	_pid = pid;
    _interval = 1;
    
    _procfs_path = (char*) malloc(strlen(DEFAULT_PROCFS_PATH) + 1);
    strcpy(_procfs_path, DEFAULT_PROCFS_PATH);
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

    int status = pthread_create(&_runner, &attr, run, this);

	if (status)
	{
		printf("ERROR: return code from pthread_create() is %d\n", status);
		exit(-1);
	}

	pthread_attr_destroy(&attr);
}

void ProcessMonitor::stop()
{
    pthread_cancel(_runner);

	int status = pthread_join(_runner, NULL);

	if (status)
	{
		printf("ERROR: return code from pthread_join() is %d\n", status);
		exit(-1);
	}
}

void ProcessMonitor::fetch()
{
    char filename[32]; //todo
    snprintf(filename, 32, "%s/%d/stat", _procfs_path, _pid);
    
    // __last_stat = _process_data;

    FILE* stream = fopen(filename, "r");
    parse_from(stream, &_process_data);
    fclose(stream);
    
    int* tids;
    _process_data._threads = parse_thread_ids(_pid, &tids);
    _process_data._thread_data = (thread_data_t*) malloc(_process_data._threads * sizeof(thread_data_t));
    
    for (int i = 0; i < _process_data._threads; ++i)
    {
        char threadfile[64]; //todo
        snprintf(threadfile, 64, "%s/%d/task/%d/stat", _procfs_path, _pid, tids[i]);
        
        FILE* stream = fopen(threadfile, "r");
        parse_from(stream, &_process_data._thread_data[i]);
        fclose(stream);
    }
    
    parse_system_stat(&_system_data);    
}

void ProcessMonitor::parse_proc_stat(int pid, process_data_t* stat)
{
    char filename[32]; //todo
    snprintf(filename, 32, "%s/%d/stat", _procfs_path, pid);

    FILE* stream = fopen(filename, "r");
    parse_from(stream, stat);
    fclose(stream);
}


void ProcessMonitor::parse_thread_stat(int pid, int tid, thread_data_t* stat)
{
    char filename[64]; //todo
    snprintf(filename, 64, "%s/%d/task/%d/stat", _procfs_path, pid, tid);

    FILE* stream = fopen(filename, "r");
    parse_from(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_system_stat(system_data_t* stat)
{
    char filename[32]; //todo
    snprintf(filename, 32, "%s/stat", _procfs_path);
    
    // todo
    if (stat->cpu == NULL)
    {
        FILE* stream = fopen(filename, "r");
        parse_cpu_count_data(stream, stat);
        stat->cpu = (cpu_data_t*) malloc(stat->cpu_count * sizeof(cpu_data_t));
        fclose(stream);
    }

    FILE* stream = fopen(filename, "r");
    parse_stat_data(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_from(FILE* stream, process_data_t* stat)
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

void ProcessMonitor::parse_stat_data(FILE* stream, system_data_t* stat_data)
{
    fscanf(stream, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        &stat_data->cpus.utime, &stat_data->cpus.nice, &stat_data->cpus.stime, &stat_data->cpus.idle, &stat_data->cpus.iowait,
        &stat_data->cpus.irq, &stat_data->cpus.softirq, &stat_data->cpus.steal, &stat_data->cpus.guest);

    int i = 0;

    while
    (
        fscanf(stream, "\ncpu%*d %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
            &stat_data->cpu[i].utime, &stat_data->cpu[i].nice, &stat_data->cpu[i].stime, &stat_data->cpu[i].idle, &stat_data->cpu[i].iowait,
            &stat_data->cpu[i].irq, &stat_data->cpu[i].softirq, &stat_data->cpu[i].steal, &stat_data->cpu[i].guest)
    ) { ++i; }
}

void ProcessMonitor::parse_cpu_count_data(FILE* stream, system_data_t* stat_data)
{
    int i;
    
    while (fscanf(stream, "\ncpu%d %*u %*u %*u %*u %*u %*u %*u %*u %*u", &i));
           
    stat_data->cpu_count = i + 1;
}
void ProcessMonitor::parse(const char* stream)
{
    sscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &_process_data.pid, _process_data.comm, &_process_data.state, &_process_data.ppid, &_process_data.pgrp, &_process_data.session, &_process_data.tty_nr,
        &_process_data.tpgid, &_process_data.flags, &_process_data.minflt, &_process_data.cminflt, &_process_data.majflt, &_process_data.cmajflt, &_process_data.utime, &_process_data.stime,
        &_process_data.cutime, &_process_data.cstime, &_process_data.priority, &_process_data.nice, &_process_data.num_threads, &_process_data.itrealvalue,
        &_process_data.starttime, &_process_data.vsize, &_process_data.rss, &_process_data.rsslim, &_process_data.startcode, &_process_data.endcode, &_process_data.startstack,
        &_process_data.kstkesp, &_process_data.kstkeip, &_process_data.signal, &_process_data.blocked, &_process_data.sigignore, &_process_data.sigcatch, &_process_data.wchan,
        &_process_data.nswap, &_process_data.cnswap, &_process_data.exit_signal, &_process_data.processor, &_process_data.rt_priority, &_process_data.policy,
        &_process_data.delayacct_blkio_ticks, &_process_data.guest_time, &_process_data.cguest_time);
}

void ProcessMonitor::parse_cpu_count(system_data_t* stat_data)
{
    char stat_path[16]; //todo
    snprintf(stat_path, 16, "%s/stat", _procfs_path);
    
    FILE* stream = fopen(stat_path, "r");
    parse_cpu_count_data(stream, stat_data);
    fclose(stream);
}

// todo
int ProcessMonitor::parse_thread_count(int pid)
{
    int* pids;
    int tcnt = parse_thread_ids(pid, &pids);
    free(pids);
    return tcnt;
}
    
int ProcessMonitor::parse_thread_ids(int pid, int** ptids)
{
    struct dirent **folders;
    
    char task_path[64]; //todo
    snprintf(task_path, 64, "%s/%d/task", _procfs_path, pid);
    
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

char* ProcessMonitor::process_path(int pid, char** ps)
{
    int length = strlen(_procfs_path) + 10; // todo roughly + 10
    *ps = (char*) malloc(length);
    
    snprintf(*ps, length, "%s/%d", _procfs_path, pid);
    
    return *ps;
}

char* ProcessMonitor::thread_path(int pid, int tid, char** ps)
{
    int length = strlen(_procfs_path) + 20; // todo roughly + 20
    *ps = (char*) malloc(length);
    
    snprintf(*ps, length, "%s/%d/task/%d", _procfs_path, pid, tid);
    
    return *ps;
}

int ProcessMonitor::cpu_count()
{
    return _system_data.cpu_count;
}
    
unsigned long ProcessMonitor::cpus()
{
    return _system_data.cpus.utime;
}

unsigned long ProcessMonitor::cpu(int cid)
{
    return _system_data.cpu[cid].utime;
}

int ProcessMonitor::threads()
{
    return _process_data._threads;
}
    
int ProcessMonitor::pid()
{
    return _pid;
}

unsigned ProcessMonitor::interval()
{
    return _interval;
}

char* ProcessMonitor::procfs_path()
{
    return _procfs_path;
}

void ProcessMonitor::procfs_path(const char* procfs_path)
{
    _procfs_path = (char*) realloc(_procfs_path, strlen(procfs_path) + 1);
    strcpy(_procfs_path, procfs_path);
}

unsigned long ProcessMonitor::utime()
{
    return _process_data.utime;
}

unsigned long ProcessMonitor::utime(int tid)
{
    return _process_data._thread_data[tid].utime;
}

char ProcessMonitor::state()
{
    return _process_data.state;
}