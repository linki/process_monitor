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
   _running  = 0;

   _procfs_path = (char*)malloc(strlen(procfs_path) + 1);
   strcpy(_procfs_path, procfs_path);

   _system_stat_path    = path_to(_procfs_path, "stat");
   _system_meminfo_path = path_to(_procfs_path, "meminfo");
   _process_stat_path   = path_to(_procfs_path, _pid, "stat");
   _process_status_path = path_to(_procfs_path, _pid, "status");
   _process_task_path   = path_to(_procfs_path, _pid, "task");

   initialize_system_data(&_last_system_data);
   initialize_system_data(&_system_data);

   initialize_process_data(&_last_process_data);
   initialize_process_data(&_process_data);

   fetch();
}

ProcessMonitor::~ProcessMonitor()
{
   free(_procfs_path);

   free(_system_stat_path);
   free(_system_meminfo_path);
   free(_process_stat_path);
   free(_process_status_path);
   free(_process_task_path);

   // todo
   free(_last_system_data._cpu_data);
   free(_system_data._cpu_data);

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

   while (1)
   {
      if (pm->fetch())
      {
         sleep(pm->interval());
      }
      else
      {
         pm->_running = 0;
         break;
      }
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

   _running = 1;
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

   _running = 0;
}

int ProcessMonitor::fetch()
{
   if (!has_valid_procfs_path())
   {
      return 0;
   }

   copy_system_data(&_last_system_data, &_system_data);
   copy_process_data(&_last_process_data, &_process_data);

   parse_system_data(&_system_data);
   parse_process_data(_pid, &_process_data);

   return 1;
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
      parse_thread(pid, tids[i], &(*thread_data)[i]);
   }

   free(tids);
}

void ProcessMonitor::parse_thread(int pid, int tid, thread_data_t* thread_data)
{
   parse_thread_stat_file(pid, tid, thread_data);
}

void ProcessMonitor::parse_process_stat_file(int pid, process_data_t* stat)
{
   FILE* file = fopen(_process_stat_path, "r");

   parse_process_stat_stream(file, stat);
   fclose(file);
}

void ProcessMonitor::parse_thread_stat_file(int pid, int tid, thread_data_t* stat)
{
   char* filename = path_to(_procfs_path, pid, tid, "stat");

   FILE* file = fopen(filename, "r");

   parse_process_stat_stream(file, stat);
   fclose(file);

   free(filename);
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
   FILE* file = fopen(_process_status_path, "r");

   parse_process_status_stream(file, data);
   fclose(file);
}

void ProcessMonitor::parse_process_status_stream(FILE* stream, memory_data_t* data)
{
   char line[64];

   while (fgets(line, 64, stream))
   {
      sscanf(line, "VmPeak: %lu kB\n", &data->peak);
      sscanf(line, "VmSize: %lu kB\n", &data->total);
      sscanf(line, "VmRSS: %lu kB\n", &data->rss);
   }
   ;
}

void ProcessMonitor::parse_system_stat_file(system_data_t* stat)
{
   if (stat->_cpu_count == 0)
   {
      FILE* file = fopen(_system_stat_path, "r");
      stat->_cpu_count = parse_system_stat_stream_for_cpu_count(file);
      stat->_cpu_data  = (cpu_data_t*)realloc(stat->_cpu_data, stat->_cpu_count * sizeof(cpu_data_t));
      fclose(file);
   }

   FILE* file = fopen(_system_stat_path, "r");
   parse_system_stat_stream(file, stat);
   fclose(file);
}

int ProcessMonitor::parse_system_stat_stream_for_cpu_count(FILE* stream)
{
   char line[64];

   fgets(line, 64, stream);
   sscanf(line, "cpu");

   int i;

   while (fgets(line, 64, stream))
   {
      sscanf(line, "cpu%d", &i);
   }

   return i + 1;
}

void ProcessMonitor::parse_system_stat_stream(FILE* stream, system_data_t* stat_data)
{
   char line[64];

   fgets(line, 64, stream);
   sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
          &stat_data->_cpus_data.utime, &stat_data->_cpus_data.nice, &stat_data->_cpus_data.stime, &stat_data->_cpus_data.idle, &stat_data->_cpus_data.iowait,
          &stat_data->_cpus_data.irq, &stat_data->_cpus_data.softirq, &stat_data->_cpus_data.steal, &stat_data->_cpus_data.guest);

   stat_data->_cpus_data.total = stat_data->_cpus_data.utime + stat_data->_cpus_data.stime + stat_data->_cpus_data.idle;

   int i;

   while (fgets(line, 64, stream))
   {
      if (sscanf(line, "cpu%d", &i))
      {
         sscanf(line, "cpu%*d %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                &stat_data->_cpu_data[i].utime, &stat_data->_cpu_data[i].nice, &stat_data->_cpu_data[i].stime, &stat_data->_cpu_data[i].idle, &stat_data->_cpu_data[i].iowait,
                &stat_data->_cpu_data[i].irq, &stat_data->_cpu_data[i].softirq, &stat_data->_cpu_data[i].steal, &stat_data->_cpu_data[i].guest);

         stat_data->_cpu_data[i].total = stat_data->_cpu_data[i].utime + stat_data->_cpu_data[i].stime + stat_data->_cpu_data[i].idle;
      }
      else
         break;
   }
}

void ProcessMonitor::parse_system_meminfo_file(meminfo_t* data)
{
   FILE* file = fopen(_system_meminfo_path, "r");

   parse_meminfo_stream(file, data);
   fclose(file);
}

void ProcessMonitor::parse_meminfo_stream(FILE* stream, meminfo_t* data)
{
   char line[64];

   while (fgets(line, 64, stream))
   {
      sscanf(line, "MemTotal: %lu kB\n", &data->total);
      sscanf(line, "MemFree: %lu kB\n", &data->free);
   }

   data->used = data->total - data->free;
}

int ProcessMonitor::parse_process_thread_ids(int pid, int** ptids)
{
   struct dirent **folders;
   int           tcnt;

   tcnt = scandir(_process_task_path, &folders, NULL, alphasort) - 2;

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

   return tcnt;
}

void ProcessMonitor::copy_system_data(system_data_t* dest_data, system_data_t* src_data)
{
   free(dest_data->_cpu_data);

   *dest_data = *src_data;

   if (src_data->_cpu_count > 0)
   {
      int bytes = src_data->_cpu_count * sizeof(cpu_data_t);
      dest_data->_cpu_data = (cpu_data_t*)malloc(bytes);
      memcpy(dest_data->_cpu_data, src_data->_cpu_data, bytes);
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

char* ProcessMonitor::path_to(const char* procfs_path, const char* name)
{
   int length = strlen(procfs_path) + 1 + strlen(name) + 1;  // todo

   char* path = (char*)malloc(length);

   snprintf(path, length, "%s/%s", procfs_path, name);

   return path;
}

char* ProcessMonitor::path_to(const char* procfs_path, int pid, const char* name)
{
   int length = strlen(procfs_path) + 1 + 5 + 1 + strlen(name) + 1;  // todo

   char* path = (char*)malloc(length);

   snprintf(path, length, "%s/%d/%s", procfs_path, pid, name);

   return path;
}

char* ProcessMonitor::path_to(const char* procfs_path, int pid, int tid, const char* name)
{
   int length = strlen(procfs_path) + 1 + 5 + 1 + 4 + 1 + 5 + 1 + strlen(name) + 1;  // todo

   char* path = (char*)malloc(length);

   snprintf(path, length, "%s/%d/task/%d/%s", procfs_path, pid, tid, name);

   return path;
}

void ProcessMonitor::initialize_system_data(system_data_t* system_data)
{
   memset(system_data, 0, sizeof(system_data_t));
}

void ProcessMonitor::initialize_process_data(process_data_t* process_data)
{
   memset(process_data, 0, sizeof(process_data_t));
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

int ProcessMonitor::has_valid_procfs_path()
{
   FILE* file = fopen(_process_stat_path, "r");

   if (file == NULL)
   {
      return 0;
   }
   else
   {
      fclose(file);
      return 1;
   }
}

int ProcessMonitor::is_running()
{
   return _running;
}

int ProcessMonitor::num_cpus()
{
   return _system_data._cpu_count;
}

unsigned long ProcessMonitor::cpus_clock_ticks_total()
{
   return _system_data._cpus_data.total;
}

unsigned long ProcessMonitor::cpu_clock_ticks_total(int cid)
{
   return _system_data._cpu_data[cid].total;
}

unsigned long ProcessMonitor::cpus_clock_ticks_used()
{
   return _system_data._cpus_data.utime + _system_data._cpus_data.stime;
}

unsigned long ProcessMonitor::cpu_clock_ticks_used(int cid)
{
   return _system_data._cpu_data[cid].utime + _system_data._cpu_data[cid].stime;
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

char* ProcessMonitor::executable_name()
{
   return _process_data.comm;
}

unsigned long ProcessMonitor::process_mem_total()
{
   return _process_data._memory_data.total;
}

unsigned long ProcessMonitor::process_mem_used()
{
   return _process_data._memory_data.rss;
}

unsigned long ProcessMonitor::process_mem_peak()
{
   return _process_data._memory_data.peak;
}

int ProcessMonitor::num_threads()
{
   return _process_data.num_threads;
}

double ProcessMonitor::cpus_usage()
{
   if (_last_system_data._cpus_data.total == _system_data._cpus_data.total)
      return 0;

   return (double)(10000 - 10000 * (_system_data._cpus_data.idle - _last_system_data._cpus_data.idle) / (_system_data._cpus_data.total - _last_system_data._cpus_data.total)) / 100;
}

double ProcessMonitor::cpu_usage(int cid)
{
   if (cid >= _last_system_data._cpu_count || cid >= _system_data._cpu_count)
      return 0;

   if (_last_system_data._cpu_data[cid].total == _system_data._cpu_data[cid].total)
      return 0;

   return (double)(10000 - 10000 * (_system_data._cpu_data[cid].idle - _last_system_data._cpu_data[cid].idle) / (_system_data._cpu_data[cid].total - _last_system_data._cpu_data[cid].total)) / 100;
}

double ProcessMonitor::system_mem_usage()
{
   if (_system_data._memory_data.total == 0)
      return 0;

   return (double)(10000 - 10000 * _system_data._memory_data.free / _system_data._memory_data.total) / 100;
}

double ProcessMonitor::process_mem_usage()
{
   if (_system_data._memory_data.total == 0)
      return 0;

   return (double)(10000 * _process_data._memory_data.rss / _system_data._memory_data.total) / 100;
}

// todo, need correct formula here
double ProcessMonitor::process_cpus_usage()
{
   if (_last_system_data._cpus_data.total == _system_data._cpus_data.total)
      return 0;

   return (double)(10000 * (_process_data.total - _last_process_data.total) / (_system_data._cpus_data.total - _last_system_data._cpus_data.total)) / 100;
}

double ProcessMonitor::thread_cpus_usage(int tid)
{
   if (tid >= _last_process_data._thread_count || tid >= _process_data._thread_count)
      return 0;

   if (_last_system_data._cpus_data.total == _system_data._cpus_data.total)
      return 0;

   return (double)(10000 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_system_data._cpus_data.total - _last_system_data._cpus_data.total)) / 100;
}

double ProcessMonitor::process_thread_cpus_usage(int tid)
{
   if (tid >= _last_process_data._thread_count || tid >= _process_data._thread_count)
      return 0;

   if (_last_process_data.total == _process_data.total)
      return 0;

   return (double)(10000 * (_process_data._thread_data[tid].total - _last_process_data._thread_data[tid].total) / (_process_data.total - _last_process_data.total)) / 100;
}
