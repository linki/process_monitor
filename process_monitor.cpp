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
    _interval = 2;
    _procfs_path = (char*) DEFAULT_PROCFS_PATH;
    
    // todo, need to init that
    _system_data.cpu_count = 0;
    _process_data._threads = 0;
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
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}
}

void ProcessMonitor::fetch()
{
    copy_system_data(&_last_system_data, &_system_data);
    parse_system_stat(&_system_data);    
    
    copy_process_data(&_last_process_data, &_process_data);
    parse_process(_pid, &_process_data);

    parse_meminfo(&_memory_data);
}

void ProcessMonitor::parse_process(int pid, process_data_t* process_data)
{
    parse_process_stat_file(pid, process_data);
    
    int* tids;
    process_data->_threads = parse_thread_ids(pid, &tids);
    
    // todo free here or realloc or whatever
    process_data->_thread_data = (thread_data_t*) malloc(process_data->_threads * sizeof(thread_data_t));
    
    for (int i = 0; i < process_data->_threads; ++i)
    {
        parse_thread_stat_file(pid, tids[i], &process_data->_thread_data[i]);
    }
    
    free(tids);
    
    parse_process_statm_file(_pid, &process_data->_memory);
}

void ProcessMonitor::parse_process_stat_file(int pid, process_data_t* stat)
{
    char* filename; //todo
    get_path(pid, "stat", &filename);

    FILE* stream = fopen(filename, "r");
    parse_thread_stat_stream(stream, stat);
    fclose(stream);
    
    free(filename);
}

void ProcessMonitor::parse_thread_stat_file(int pid, int tid, thread_data_t* stat)
{
    char* filename; //todo
    get_path(pid, tid, "stat", &filename);

    FILE* stream = fopen(filename, "r");
    parse_thread_stat_stream(stream, stat);
    fclose(stream);
    
    free(filename);
}

void ProcessMonitor::parse_thread_stat_stream(FILE* stream, process_data_t* stat)
{
    fscanf(stream, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld",
        &stat->pid, stat->comm, &stat->state, &stat->ppid, &stat->pgrp, &stat->session, &stat->tty_nr,
        &stat->tpgid, &stat->flags, &stat->minflt, &stat->cminflt, &stat->majflt, &stat->cmajflt, &stat->utime, &stat->stime,
        &stat->cutime, &stat->cstime, &stat->priority, &stat->nice, &stat->num_threads, &stat->itrealvalue,
        &stat->starttime, &stat->vsize, &stat->rss, &stat->rsslim, &stat->startcode, &stat->endcode, &stat->startstack,
        &stat->kstkesp, &stat->kstkeip, &stat->signal, &stat->blocked, &stat->sigignore, &stat->sigcatch, &stat->wchan,
        &stat->nswap, &stat->cnswap, &stat->exit_signal, &stat->processor, &stat->rt_priority, &stat->policy,
        &stat->delayacct_blkio_ticks, &stat->guest_time, &stat->cguest_time);
        
    stat->total = stat->utime + stat->stime;
}

void ProcessMonitor::parse_process_statm_file(int pid, process_datam_t* data)
{
    char* filename; //todo
    get_path(pid, "statm", &filename);

    FILE* stream = fopen(filename, "r");
    parse_process_statm_stream(stream, data);
    fclose(stream);

    free(filename);
}

void ProcessMonitor::parse_process_statm_stream(FILE* stream, process_datam_t* data)
{
    fscanf(stream, "%llu %llu %llu %llu %llu %llu %llu\n",
        &data->size, &data->resident, &data->share, &data->text,
        &data->lib, &data->data, &data->dt);    
}

void ProcessMonitor::parse_system_stat(system_data_t* stat)
{
    char* filename; //todo
    get_path("stat", &filename);
    
    // BIG todo
    if (1)
    {
        FILE* stream = fopen(filename, "r");
        parse_cpu_count_data(stream, stat);
        stat->cpu = (cpu_data_t*) malloc(stat->cpu_count * sizeof(cpu_data_t));
        fclose(stream);
    }

    FILE* stream = fopen(filename, "r");
    parse_system_stat_stream(stream, stat);
    fclose(stream);
    
    free(filename);
}

void ProcessMonitor::parse_system_stat_stream(FILE* stream, system_data_t* stat_data)
{
    fscanf(stream, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
        &stat_data->cpus.utime, &stat_data->cpus.nice, &stat_data->cpus.stime, &stat_data->cpus.idle, &stat_data->cpus.iowait,
        &stat_data->cpus.irq, &stat_data->cpus.softirq, &stat_data->cpus.steal, &stat_data->cpus.guest);
        
    stat_data->cpus.total = stat_data->cpus.utime + stat_data->cpus.stime + stat_data->cpus.idle;

    int i = 0;

    while
    (
        fscanf(stream, "cpu%*d %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
            &stat_data->cpu[i].utime, &stat_data->cpu[i].nice, &stat_data->cpu[i].stime, &stat_data->cpu[i].idle, &stat_data->cpu[i].iowait,
            &stat_data->cpu[i].irq, &stat_data->cpu[i].softirq, &stat_data->cpu[i].steal, &stat_data->cpu[i].guest)
    )
    {
        stat_data->cpu[i].total = stat_data->cpu[i].utime + stat_data->cpu[i].stime + stat_data->cpu[i].idle;
        ++i;
    }
}
void ProcessMonitor::parse_meminfo(meminfo_t* data)
{
    char* filename; //todo
    get_path("meminfo", &filename);
    
    FILE* stream = fopen(filename, "r");
    parse_meminfo_data(stream, data);
    fclose(stream);
    
    free(filename); 
}
    



void ProcessMonitor::parse_meminfo_data(FILE* stream, meminfo_t* data)
{
    fscanf(stream, "MemTotal: %llu kB\n   \
                    MemFree: %llu kB\n",
                    &data->total,
                    &data->free);    
    
   // char line[64];
   // while (!feof(stream))
   // {
   //     fgets(line, 64, stream);
   //     sscanf(line, "MemTotal: %llu kB\n", &data->total);
   //     sscanf(line, "MemFree: %llu kB\n", &data->free);
   // }
}
    
void ProcessMonitor::parse_cpu_count_data(FILE* stream, system_data_t* stat_data)
{
    int i;
    
    while (fscanf(stream, "cpu%d %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", &i));
           
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
    char* filename; //todo
    get_path("stat", &filename);
    
    FILE* stream = fopen(filename, "r");
    parse_cpu_count_data(stream, stat_data);
    fclose(stream);
    
    free(filename);
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
    
    char* task_path; //todo
    get_path(pid, "task", &task_path);
        
    int tcnt = scandir(task_path, &folders, NULL, alphasort) - 2;

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
    
    free(task_path);
    
    return tcnt;
}

char* ProcessMonitor::get_path(const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    
    snprintf(*path, length, "%s/%s", _procfs_path, name);
    
    return *path;
}

char* ProcessMonitor::get_path(int pid, const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + 5 + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    
    snprintf(*path, length, "%s/%d/%s", _procfs_path, pid, name);
    
    return *path;
}

char* ProcessMonitor::get_path(int pid, int tid, const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + 5 + 1 + 4 + 1 + 5 + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    
    snprintf(*path, length, "%s/%d/task/%d/%s", _procfs_path, pid, tid, name);
    
    return *path;
}

int ProcessMonitor::cpu_usage()
{
    if (_system_data.cpus.total - _last_system_data.cpus.total == 0)
    {
        return 0;
    }
    
    return 100 - 100 * (_system_data.cpus.idle - _last_system_data.cpus.idle) / (_system_data.cpus.total - _last_system_data.cpus.total);
}

int ProcessMonitor::cpu_usage(int cid)
{
    if (_system_data.cpu[cid].total - _last_system_data.cpu[cid].total == 0)
    {
        return 0;
    }

    return 100 - 100 * (_system_data.cpu[cid].idle - _last_system_data.cpu[cid].idle) / (_system_data.cpu[cid].total - _last_system_data.cpu[cid].total);
}

int ProcessMonitor::process_cpu_usage()
{
    if (_system_data.cpus.total - _last_system_data.cpus.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data.total - _last_process_data.total) / (_system_data.cpus.total - _last_system_data.cpus.total);
}

int ProcessMonitor::global_thread_cpu_usage(int cid)
{
    if (_system_data.cpus.total - _last_system_data.cpus.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data._thread_data[cid].total - _last_process_data._thread_data[cid].total) / (_system_data.cpus.total - _last_system_data.cpus.total);
}

int ProcessMonitor::thread_cpu_usage(int cid)
{
    if (_process_data.total - _last_process_data.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data._thread_data[cid].total - _last_process_data._thread_data[cid].total) / (_process_data.total - _last_process_data.total);
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
    _procfs_path = (char*) procfs_path;
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

unsigned long ProcessMonitor::mem_total()
{
    return _memory_data.total;
}

unsigned long ProcessMonitor::mem_free()
{
    return _memory_data.free;
}

// free up cpu of destination
void ProcessMonitor::copy_system_data(system_data_t* dest_data, system_data_t* src_data)
{
    *dest_data = *src_data;
    
    int num_cpus = src_data->cpu_count;
    
    if (num_cpus > 0)
    {
        dest_data->cpu = (cpu_data_t*) malloc(num_cpus * sizeof(cpu_data_t));
        memcpy(dest_data->cpu, src_data->cpu, num_cpus * sizeof(cpu_data_t));
    }
}

// free up cpu of destination
void ProcessMonitor::copy_process_data(process_data_t* dest_data, process_data_t* src_data)
{
    *dest_data = *src_data;
    
    int num_threads = src_data->_threads;
    
    if (num_threads > 0)
    {
        dest_data->_thread_data = (thread_data_t*) malloc(num_threads * sizeof(thread_data_t));
        memcpy(dest_data->_thread_data, src_data->_thread_data, num_threads * sizeof(thread_data_t));
    }
}