#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>

#include "process_monitor.h"

ProcessMonitor::ProcessMonitor(int pid)
{
   initialize(pid, PM_DEFAULT_INTERVAL, PM_DEFAULT_PROCFS_PATH);
}

ProcessMonitor::ProcessMonitor(int pid, int interval)
{
    initialize(pid, interval, PM_DEFAULT_PROCFS_PATH);
}

ProcessMonitor::ProcessMonitor(int pid, const char* procfs_path)
{
   initialize(pid, PM_DEFAULT_INTERVAL, procfs_path);
}

ProcessMonitor::ProcessMonitor(int pid, int interval, const char* procfs_path)
{
    initialize(pid, interval, procfs_path);
}

void ProcessMonitor::initialize(int pid, int interval, const char* procfs_path)
{
   _pid      = pid;
   _interval = interval;

   _procfs_path = (char*)malloc(strlen(procfs_path) + 1);
   strcpy(_procfs_path, procfs_path);

   initialize_system_data(&_last_system_data);
   initialize_system_data(&_system_data);

   initialize_process_data(&_last_process_data);
   initialize_process_data(&_process_data);
}

ProcessMonitor::~ProcessMonitor()
{
   free(_procfs_path);
   
   // todo
   free(_last_system_data.cpu);   
   free(_system_data.cpu);   
   
   free(_last_process_data._thread_data);
   free(_process_data._thread_data);
}

void* ProcessMonitor::run(void* instance)
{
   /* enable canceling of thread */
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

   /* enable immediate cancelation */
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   ProcessMonitor* pm = (ProcessMonitor*)instance;

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

   parse_system_data(&_system_data);
   parse_process_data(_pid, &_process_data);
}

void ProcessMonitor::parse_system_data(system_data_t* system_data)
{
   parse_system_stat_file(system_data);
   parse_system_meminfo_file(&system_data->_memory_data);
}

void ProcessMonitor::parse_process_data(int pid, process_data_t* process_data)
{
   parse_process_stat_file(pid, process_data);
   parse_process_status_file(pid, &process_data->_memory_data);

   parse_process_threads(pid, &process_data->_thread_data, &process_data->_thread_count);
}

void ProcessMonitor::parse_process_threads(int pid, thread_data_t** thread_data, int* thread_count)
{
   int * tids;
   int new_thread_count;

   new_thread_count = parse_process_thread_ids(pid, &tids);

   if (*thread_count != new_thread_count)
   {
      *thread_count = new_thread_count;
      *thread_data  = (thread_data_t*)realloc(*thread_data, *thread_count * sizeof(thread_data_t));
   }

   for (int i = 0; i < *thread_count; ++i)
   {
      parse_thread(pid, tids[i], *thread_data + i);   /* pointer wulst ^^ */
   }

   free(tids);
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

void ProcessMonitor::parse_process_status_file(int pid, memory_data_t* data)
{
   FILE* stream;

   open_file(pid, "status", &stream);
   parse_process_status_stream(stream, data);
   fclose(stream);
}

void ProcessMonitor::parse_process_status_stream(FILE* stream, memory_data_t* data)
{
   // todo
   char line[64];

   while (!feof(stream))
   {
      fgets(line, 64, stream);
      sscanf(line, "VmPeak: %lu kB\n", &data->vm_peak);
      sscanf(line, "VmSize: %lu kB\n", &data->vm_size);
      sscanf(line, "VmRSS: %lu kB\n", &data->vm_rss);
   }
}

void ProcessMonitor::parse_system_stat_file(system_data_t* stat)
{
   if (stat->cpu_count == 0)
   {
      FILE* stream;
      open_file("stat", &stream);
      stat->cpu_count = parse_system_stat_stream_for_cpu_count(stream);
      stat->cpu       = (cpu_data_t*)realloc(stat->cpu, stat->cpu_count * sizeof(cpu_data_t));
      fclose(stream);
   }

   FILE* stream;
   open_file("stat", &stream);
   parse_system_stat_stream(stream, stat);
   fclose(stream);
}

int ProcessMonitor::parse_system_stat_stream_for_cpu_count(FILE* stream)
{
   char line[64];

   fgets(line, 64, stream);
   sscanf(line, "cpu");

   int i;

   while (!feof(stream))
   {
      fgets(line, 64, stream);
      sscanf(line, "cpu%d", &i);
   }

   return i + 1;
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

void ProcessMonitor::parse_system_meminfo_file(meminfo_t* data)
{
   FILE* stream;

   open_file("meminfo", &stream);
   parse_meminfo_stream(stream, data);
   fclose(stream);
}

void ProcessMonitor::parse_meminfo_stream(FILE* stream, meminfo_t* data)
{
   char line[64];

   while (!feof(stream))
   {
      fgets(line, 64, stream);
      sscanf(line, "MemTotal: %lu kB\n", &data->total);
      sscanf(line, "MemFree: %lu kB\n", &data->free);
   }

   data->used = data->total - data->free;
}

int ProcessMonitor::parse_process_thread_ids(int pid, int** ptids)
{
   struct dirent **folders;
   char          * task_path;

   get_path(pid, "task", &task_path);

   int tcnt = scandir(task_path, &folders, NULL, alphasort) - 2;

   *ptids = (int*)malloc(tcnt * sizeof(int));

   for (int i = 0; i < tcnt; ++i)
   {
      (*ptids)[i] = atoi(folders[i + 2]->d_name);
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
   int length = strlen(_procfs_path) + 1 + strlen(name) + 1;  // todo

   *path = (char*)malloc(length);
   snprintf(*path, length, "%s/%s", _procfs_path, name);
}

void ProcessMonitor::get_path(int pid, const char* name, char** path)
{
   int length = strlen(_procfs_path) + 1 + 5 + 1 + strlen(name) + 1;  // todo

   *path = (char*)malloc(length);
   snprintf(*path, length, "%s/%d/%s", _procfs_path, pid, name);
}

void ProcessMonitor::get_path(int pid, int tid, const char* name, char** path)
{
   int length = strlen(_procfs_path) + 1 + 5 + 1 + 4 + 1 + 5 + 1 + strlen(name) + 1;  // todo

   *path = (char*)malloc(length);
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

void ProcessMonitor::copy_system_data(system_data_t* dest_data, system_data_t* src_data)
{
   free(dest_data->cpu);

   *dest_data = *src_data;

   if (src_data->cpu_count > 0)
   {
      int bytes = src_data->cpu_count * sizeof(cpu_data_t);
      dest_data->cpu = (cpu_data_t*)malloc(bytes);
      memcpy(dest_data->cpu, src_data->cpu, bytes);
   }
}

void ProcessMonitor::copy_process_data(process_data_t* dest_data, process_data_t* src_data)
{
   free(dest_data->_thread_data);

   *dest_data = *src_data;

   if (src_data->_thread_count > 0)
   {
      int bytes = src_data->_thread_count * sizeof(thread_data_t);
      dest_data->_thread_data = (thread_data_t*)malloc(bytes);
      memcpy(dest_data->_thread_data, src_data->_thread_data, bytes);
   }
}

void ProcessMonitor::initialize_system_data(system_data_t* system_data)
{
   system_data->cpu_count = 0;
   system_data->cpu       = NULL;
}

void ProcessMonitor::initialize_process_data(process_data_t* process_data)
{
   process_data->_thread_count = 0;
   process_data->_thread_data  = NULL;
}

void ProcessMonitor::initialize_thread_data(thread_data_t** thread_data, int* thread_count)
{
   *thread_count = 0;
   *thread_data  = NULL;
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

int ProcessMonitor::num_cpus()
{
   return _system_data.cpu_count;
}

unsigned long ProcessMonitor::cpus_jiffies_total()
{
  return _system_data.cpus.total;
}

unsigned long ProcessMonitor::cpu_jiffies_total(int cid)
{
  return _system_data.cpu[cid].total;
}

unsigned long ProcessMonitor::cpus_jiffies_used()
{
  return _system_data.cpus.utime + _system_data.cpus.stime;
}

unsigned long ProcessMonitor::cpu_jiffies_used(int cid)
{
  return _system_data.cpu[cid].utime + _system_data.cpu[cid].stime;
}

unsigned long ProcessMonitor::system_mem_total()
{
   return _system_data._memory_data.total;
}

unsigned long ProcessMonitor::system_mem_free()
{
   return _system_data._memory_data.free;
}

unsigned long ProcessMonitor::system_mem_used()
{
   return _system_data._memory_data.used;
}

char ProcessMonitor::state()
{
   return _process_data.state;
}

unsigned long ProcessMonitor::process_mem_total()
{
   return _process_data._memory_data.vm_size;
}

unsigned long ProcessMonitor::process_mem_used()
{
   return _process_data._memory_data.vm_rss;
}

unsigned long ProcessMonitor::process_mem_peak()
{
   return _process_data._memory_data.vm_peak;
}

int ProcessMonitor::num_threads()
{
  return _process_data.num_threads;
}

double ProcessMonitor::cpus_usage()
{
   if (_last_system_data.cpus.total == _system_data.cpus.total) return 0;

   return (double)(10000 - 10000 * (_system_data.cpus.idle - _last_system_data.cpus.idle) / (_system_data.cpus.total - _last_system_data.cpus.total)) / 100;
}

double ProcessMonitor::cpu_usage(int cid)
{
   if (_last_system_data.cpu[cid].total == _system_data.cpu[cid].total) return 0;

   return (double)(10000 - 10000 * (_system_data.cpu[cid].idle - _last_system_data.cpu[cid].idle) / (_system_data.cpu[cid].total - _last_system_data.cpu[cid].total)) / 100;
}

double ProcessMonitor::system_mem_usage()
{
   if (_system_data._memory_data.total == 0) return 0;

  return (double)(10000 - 10000 * _system_data._memory_data.free / _system_data._memory_data.total) / 100;
}

double ProcessMonitor::process_mem_usage()
{
    if (_system_data._memory_data.total == 0) return 0;
    
   return (double)(10000 * _process_data._memory_data.vm_rss / _system_data._memory_data.total) / 100;
}

// todo, need correct formula here. check with top
double ProcessMonitor::process_cpus_usage()
{
    if (_last_system_data.cpus.total == _system_data.cpus.total) return 0;

   return (double)(10000 * (_process_data.total - _last_process_data.total) / (_system_data.cpus.total - _last_system_data.cpus.total)) / 100;
}

double ProcessMonitor::thread_cpus_usage(int tid)
{
  if (_last_system_data.cpus.total == _system_data.cpus.total) return 0;

  return (double)(10000 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_system_data.cpus.total - _last_system_data.cpus.total)) / 100;
}

double ProcessMonitor::process_thread_cpus_usage(int tid)
{
   if (_last_process_data.total == _process_data.total) return 0;

   return (double)(10000 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_process_data.total - _last_process_data.total)) / 100;
}
