#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "process_monitor.h"

int main (int argc, const char* argv[])
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

	ProcessMonitor* pm = new ProcessMonitor(pid);
    // pm->procfs_path("test/proc");

    pm->start();

    while (1)
    {
	    printf("s: %c p: %lu", pm->state(), pm->utime());
	    
        for (int i = 0; i < pm->threads(); ++i)
        {
            printf(" t%d: %lu", i, pm->utime(i));
        }
	    
        printf("\n");
        
        sleep(0.1);
    }

    pm->stop();

    exit(EXIT_SUCCESS);
}








// src/lib/net handler
