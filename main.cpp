#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "process_monitor.h"

int main(int argc, const char* argv[])
{
   int pid;

   if (argc == 2)
   {
      pid = atoi(argv[1]);
   }
   else
   {
      printf("USAGE: %s PID\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   ProcessMonitor pm(pid);

   if (!pm.valid_procfs_path())
   {
      printf("ERROR: invalid procfs path or process id\n");
      exit(EXIT_FAILURE);
   }

   pm.start();

   while (1)
   {
      printf("\033[2J");            // clear screen
      printf("\033[%d;%dH", 0, 0);  // cursor to 0, 0

      printf("Process: %s\n\n", pm.executable_name());

      printf("Overall CPU Usage: %3.2f%%\n", pm.cpus_usage());
      printf("  %d Cores: ", pm.num_cpus());

      for (int i = 0; i < pm.num_cpus(); ++i)
      {
         printf("%3.2f%% ", pm.cpu_usage(i));
      }

      printf("\n\n");

      printf("Overall CPU Usage by Process %d: %3.2f%%\n", pid, pm.process_cpus_usage());
      printf("  %d Threads: ", pm.num_threads());

      for (int i = 0; i < pm.num_threads(); ++i)
      {
         printf("%3.2f%% ", pm.process_thread_cpus_usage(i));
      }

      printf("\n\n");

      printf("Total Memory: %lu kB\n", pm.system_mem_total());
      printf("Free  Memory: %lu kB\n", pm.system_mem_free());
      printf("Overall Memory Usage: %3.2f%%\n", pm.system_mem_usage());

      printf("\n");

      printf("Total Memory of Process %d: %lu kB\n", pid, pm.process_mem_total());
      printf("Memory Used by Process %d: %lu kB\n", pid, pm.process_mem_used());
      printf("Process Memory Usage: %3.2f%%\n", pm.process_mem_usage());

      fflush(stdout);

      sleep(2);
   }

   pm.stop();

   return EXIT_SUCCESS;
}
