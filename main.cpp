#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "process_monitor.h"

int main (int argc, const char* argv[])
{
    pid_t pid;

	if (argc == 2)
	{
	    pid = atoi(argv[1]);
	}
	else
	{
		printf("USAGE: %s PID\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ProcessMonitor* pm = new ProcessMonitor(pid);
    // pm->procfs_path("test/proc");
    pm->fetch();
    pm->fetch();    

    pm->start();

    while (1)
    {
        printf("s: %c p: %d ", pm->state(), pm->process_cpu_usage());
        
        for (int i = 0; i < pm->threads(); ++i)
        {
            printf("s%d: %c g%d: %d l%d: %d ", i, pm->state(), i, pm->global_thread_cpu_usage(i), i, pm->thread_cpu_usage(i));
        }
        
        // printf("a: %d", pm->cpu_usage());
        // 
        // for (int i = 0; i < pm->cpu_count(); ++i)
        // {
        //     printf(" c%d: %d", i, pm->cpu_usage(i));
        // }
	    
	    
        printf("\n");
        
        sleep(1);
    }

    pm->stop();

    return EXIT_SUCCESS;
}








// src/lib/net handler
