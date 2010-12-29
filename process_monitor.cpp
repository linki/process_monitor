#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>

#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	initialize(pid, 2, DEFAULT_PROCFS_PATH);
}

ProcessMonitor::ProcessMonitor(int pid, const char* procfs_path)
{
    initialize(pid, 2, procfs_path);
}

ProcessMonitor::~ProcessMonitor()
{
    free(_procfs_path);
}

void ProcessMonitor::initialize(int pid, int interval, const char* procfs_path)
{
	_pid = pid;
    _interval = interval;
    
    _procfs_path = (char*) malloc(strlen(procfs_path) + 1);
    strcpy(_procfs_path, procfs_path);

    _init_system_data(&_last_system_data);
    _init_system_data(&_system_data);

    _init_process_data(&_last_process_data);
    _init_process_data(&_process_data);
}

void* ProcessMonitor::run(void* instance)
{
	/* enable canceling of thread */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	/* enable immediate cancelation */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    ProcessMonitor* pm = (ProcessMonitor*) instance;

    while (true)
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
    copy_process_data(&_last_process_data, &_process_data);
    
    parse_system(&_system_data);
    parse_process(_pid, &_process_data);
}

void ProcessMonitor::parse_system(system_data_t* system_data)
{
    parse_system_stat_file(system_data);
    parse_meminfo_file(&system_data->_memory_data);
}

void ProcessMonitor::parse_process(int pid, process_data_t* process_data)
{
    parse_process_stat_file(pid, process_data);
    process_data->_thread_count = parse_threads(pid, &process_data->_thread_data);
    parse_process_statm_file(pid, &process_data->_memory_data);
    parse_process_status_file(pid, &process_data->_memory_data2);
}

int ProcessMonitor::parse_threads(int pid, thread_data_t** thread_data)
{
    int* tids;
    int thread_count = _parse_thread_ids(pid, &tids);
    
    *thread_data = (thread_data_t*) realloc(*thread_data, thread_count * sizeof(thread_data_t));
    
    for (int i = 0; i < thread_count; ++i)
    {
        parse_thread(pid, tids[i], *thread_data + i); /* pointer wulst ^^ */
    }
    
    free(tids);
    
    return thread_count;
}

void ProcessMonitor::parse_thread(int pid, int tid, thread_data_t* thread_data)
{    
    parse_thread_stat_file(pid, tid, thread_data);
}

void ProcessMonitor::parse_process_stat_file(int pid, process_data_t* stat)
{
    FILE* stream;
    open_file(pid, "stat", &stream);
    parse_process_stat_stream(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_thread_stat_file(int pid, int tid, thread_data_t* stat)
{
    FILE* stream;
    open_file(pid, tid, "stat", &stream);
    parse_process_stat_stream(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_process_stat_stream(FILE* stream, process_data_t* stat)
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
    FILE* stream;
    open_file(pid, "statm", &stream);
    parse_process_statm_stream(stream, data);
    fclose(stream);
}

void ProcessMonitor::parse_process_statm_stream(FILE* stream, process_datam_t* data)
{
    fscanf(stream, "%llu %llu %llu %llu %llu %llu %llu\n",
        &data->size, &data->resident, &data->share, &data->text,
        &data->lib, &data->data, &data->dt);    
}

void ProcessMonitor::parse_process_status_file(int pid, process_status_t* data)
{
    FILE* stream;
    open_file(pid, "status", &stream);
    parse_process_status_stream(stream, data);
    fclose(stream);
}

void ProcessMonitor::parse_process_status_stream(FILE* stream, process_status_t* data)
{
    // todo
    char line[64];
    while (!feof(stream))
    {
        fgets(line, 64, stream);
        sscanf(line, "VmSize: %llu kB\n", &data->vm_size);
        sscanf(line, "VmRSS: %llu kB\n", &data->vm_rss);
    }
}

void ProcessMonitor::parse_system_stat_file(system_data_t* stat)
{
    FILE* stream;
    
    // todo?
    if (stat->cpu_count == 0)
    {
        open_file("stat", &stream);
        stat->cpu_count = parse_system_stat_stream_for_cpu_count(stream);
        fclose(stream);
        
        stat->cpu = (cpu_data_t*) realloc(stat->cpu, stat->cpu_count * sizeof(cpu_data_t));        
    }

    open_file("stat", &stream);
    parse_system_stat_stream(stream, stat);
    fclose(stream);
}

void ProcessMonitor::parse_system_stat_stream(FILE* stream, system_data_t* stat_data)
{
    char line[64];
    
    fgets(line, 64, stream);
    sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        &stat_data->cpus.utime, &stat_data->cpus.nice, &stat_data->cpus.stime, &stat_data->cpus.idle, &stat_data->cpus.iowait,
        &stat_data->cpus.irq, &stat_data->cpus.softirq, &stat_data->cpus.steal, &stat_data->cpus.guest);
        
    stat_data->cpus.total = stat_data->cpus.utime + stat_data->cpus.stime + stat_data->cpus.idle;

    int i;

    while (!feof(stream))
    {
        fgets(line, 64, stream);

        if (sscanf(line, "cpu%d", &i))
        {
            sscanf(line, "cpu%*d %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                &stat_data->cpu[i].utime, &stat_data->cpu[i].nice, &stat_data->cpu[i].stime, &stat_data->cpu[i].idle, &stat_data->cpu[i].iowait,
                &stat_data->cpu[i].irq, &stat_data->cpu[i].softirq, &stat_data->cpu[i].steal, &stat_data->cpu[i].guest);
            
        	stat_data->cpu[i].total = stat_data->cpu[i].utime + stat_data->cpu[i].stime + stat_data->cpu[i].idle;
        }
        else
        {
            break;
        }
    }
}

int ProcessMonitor::parse_system_stat_stream_for_cpu_count(FILE* stream)
{
    
    
    

    int i;
    
    char line[64];
    while (!feof(stream))
    {
        fgets(line, 64, stream);
        sscanf(line, "cpu");
        sscanf(line, "cpu%d", &i);
    }    
    
    //while (fscanf(stream, "cpu%d %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", &i));
    return i + 1;
}
    
void ProcessMonitor::parse_meminfo_file(meminfo_t* data)
{
    FILE* stream;
    open_file("meminfo", &stream);
    parse_meminfo_stream(stream, data);
    fclose(stream);
}

void ProcessMonitor::parse_meminfo_stream(FILE* stream, meminfo_t* data)
{
    fscanf(stream, "MemTotal: %llu kB\n   \
                    MemFree: %llu kB\n",
                    &data->total,
                    &data->free);
                    
    data->used = data->total - data->free;
    
   // char line[64];
   // while (!feof(stream))
   // {
   //     fgets(line, 64, stream);
   //     sscanf(line, "MemTotal: %llu kB\n", &data->total);
   //     sscanf(line, "MemFree: %llu kB\n", &data->free);
   // }
}

int ProcessMonitor::_parse_thread_ids(int pid, int** ptids)
{
    struct dirent **folders;
    
    char* task_path;
    get_path(pid, "task", &task_path);
        
    int tcnt = scandir(task_path, &folders, NULL, alphasort) - 2;

    *ptids = (int*) malloc(tcnt * sizeof(int));
        
    for (int i = 0; i < tcnt; ++i)
    {
        (*ptids)[i] = atoi(folders[i+2]->d_name);
    }

    for (int i = 0; i < tcnt + 2; ++i)
    {
        free(folders[i]);
    }
            
    free(folders);
    
    free(task_path);
    
    return tcnt;
}

void ProcessMonitor::get_path(const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    snprintf(*path, length, "%s/%s", _procfs_path, name);
}

void ProcessMonitor::get_path(int pid, const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + 5 + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    snprintf(*path, length, "%s/%d/%s", _procfs_path, pid, name);
}

void ProcessMonitor::get_path(int pid, int tid, const char* name, char** path)
{
    int length = strlen(_procfs_path) + 1 + 5 + 1 + 4 + 1 + 5 + 1 + strlen(name) + 1; // todo
    *path = (char*) malloc(length);
    snprintf(*path, length, "%s/%d/task/%d/%s", _procfs_path, pid, tid, name);
}

void ProcessMonitor::open_file(const char* name, FILE** file)
{
    char* filename;
    get_path(name, &filename);
    *file = fopen(filename, "r");
    free(filename);
}

void ProcessMonitor::open_file(int pid, const char* name, FILE** file)
{
    char* filename;
    get_path(pid, name, &filename);
    *file = fopen(filename, "r");
    free(filename);
}

void ProcessMonitor::open_file(int pid, int tid, const char* name, FILE** file)
{
    char* filename;
    get_path(pid, tid, name, &filename);
    *file = fopen(filename, "r");
    free(filename);
}

double ProcessMonitor::cpu_usage()
{
    if (_system_data.cpus.total == _last_system_data.cpus.total)
    {
        return 0;
    }
    
    return (double) (10000 - 10000 * (_system_data.cpus.idle - _last_system_data.cpus.idle) / (_system_data.cpus.total - _last_system_data.cpus.total)) / 100;
}

double ProcessMonitor::cpu_usage(int cid)
{
    if (_system_data.cpu[cid].total == _last_system_data.cpu[cid].total)
    {
        return 0;
    }

    return (double) (10000 - 10000 * (_system_data.cpu[cid].idle - _last_system_data.cpu[cid].idle) / (_system_data.cpu[cid].total - _last_system_data.cpu[cid].total)) / 100;
}

// todo, need correct formula here
int ProcessMonitor::process_cpu_usage()
{
    if (_system_data.cpus.total - _last_system_data.cpus.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data.total - _last_process_data.total) / (_system_data.cpus.total - _last_system_data.cpus.total);
}

int ProcessMonitor::thread_cpu_usage(int tid)
{
    if (_process_data.total - _last_process_data.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_process_data.total - _last_process_data.total);
}

int ProcessMonitor::global_thread_cpu_usage(int tid)
{
    if (_system_data.cpus.total - _last_system_data.cpus.total == 0)
    {
        return 0;
    }

    return 100 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_system_data.cpus.total - _last_system_data.cpus.total);
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
    return _process_data._thread_count;
}
    
int ProcessMonitor::pid()
{
    return _pid;
}

int ProcessMonitor::interval()
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
    return _system_data._memory_data.total;
}

unsigned long ProcessMonitor::mem_free()
{
    return _system_data._memory_data.free;
}

int ProcessMonitor::process_mem_usage()
{
    return 100 * _process_data._memory_data2.vm_rss / _system_data._memory_data.total;
}

int ProcessMonitor::mem_usage()
{
    return 100 - 100 * _system_data._memory_data.free / _system_data._memory_data.total;
}

void ProcessMonitor::copy_system_data(system_data_t* dest_data, system_data_t* src_data)
{
    free(dest_data->cpu);
    
    *dest_data = *src_data;
    
    int num_cpus = src_data->cpu_count;
    
    if (num_cpus > 0)
    {
        int bytes = num_cpus * sizeof(cpu_data_t);
        dest_data->cpu = (cpu_data_t*) malloc(bytes);
        memcpy(dest_data->cpu, src_data->cpu, bytes);
    }
}

void ProcessMonitor::copy_process_data(process_data_t* dest_data, process_data_t* src_data)
{
    free(dest_data->_thread_data);
    
    *dest_data = *src_data;
    
    int num_threads = src_data->_thread_count;
    
    if (num_threads > 0)
    {
        int bytes = num_threads * sizeof(thread_data_t);
        dest_data->_thread_data = (thread_data_t*) malloc(bytes);
        memcpy(dest_data->_thread_data, src_data->_thread_data, bytes);
    }
}

void ProcessMonitor::_init_process_data(process_data_t* process_data)
{
    process_data->_thread_count = 0;
    process_data->_thread_data = NULL;
}

void ProcessMonitor::_init_system_data(system_data_t* system_data)
{
    system_data->cpu_count = 0;
    system_data->cpu = NULL;
}