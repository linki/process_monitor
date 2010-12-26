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
		return -1;
	}

	ProcessMonitor* pm = new ProcessMonitor(pid);
    // pm->procfs_path("test/proc");

    pm->start();

    while (1)
    {
	    printf("p: %lu", pm->utime());
	    
        for (int i = 0; i < pm->threads(); ++i)
        {
            printf(" t%d: %lu", i, pm->utime(i));
        }
	    
        printf("\n");
        
        sleep(1);
    }

    pm->stop();

    return 0;
}








// src/lib/net handler
