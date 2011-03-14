#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "process_monitor.h"

/* Sample Output:
 *
 * Process: (channels)
 *
 * Overall CPU Usage: 33.26%
 *   2 Cores: 36.50% 30.19% 
 * 
 * Overall CPU Usage by Process 2814: 41.50%
 *   5 Threads: 0.00% 26.90% 22.80% 26.90% 23.97% 
 * 
 * Total Memory: 1538980 kB
 * Free  Memory: 593572 kB
 * Overall Memory Usage: 61.44%
 * 
 * Total Memory of Process 2814: 47180 kB
 * Memory Used by Process 2814: 8524 kB
 * Process Memory Usage: 0.55%
 *
 */
int main(int argc, const char **argv)
{
   if (argc != 2)
   {
      fprintf(stderr, "USAGE: %s PID\n", argv[0]);
      return EXIT_FAILURE;
   }

   int pid = atoi(argv[1]);   

   ProcessMonitor pm(pid);

   if (!pm.has_valid_procfs_path())
   {
      fprintf(stderr, "ERROR: invalid procfs path or process id\n");
      return EXIT_FAILURE;
   }

   pm.start();

   while (pm.is_running())
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

   printf("\nProcess exited.\n\n");

   return EXIT_SUCCESS;
}
